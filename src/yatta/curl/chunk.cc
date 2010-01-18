/*      chunk.cc -- part of the Yatta! Download Manager
 *      Copyright (C) 2009, Chow Loong Jin <hyperair@gmail.com>
 *
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sstream>
#include <limits>
#include <algorithm>

#include <sigc++/signal.h>

#include "chunk.hh"
#include "download.hh"
#include "manager.hh"

namespace Yatta
{
    namespace Curl
    {
        // typedefs
        typedef sigc::signal<void,
                             void* /*data*/, size_t /*bytes*/>
        signal_header_t;
        typedef sigc::signal<void,
                           double /*dltotal*/, double /*dlnow*/,
                           double /*ultotal*/, double /*ulnow*/>
        signal_progress_t;
        typedef signal_header_t signal_write_t;
        typedef sigc::signal<void> signal_started_t;
        typedef signal_started_t signal_stopped_t;
        typedef sigc::signal<void, CURLcode> signal_finished_t;


        // first the private class implementation
        struct Chunk::Private
        {
            // constructor
            Private (Download &parent, size_t offset, size_t total) :
                handle (NULL),
                parent (parent),
                offset (offset),
                downloaded (0),
                total (total),
                running (false),
                curl_callback_running (false),
                cancelled (false),
                signal_header (),
                signal_progress (),
                signal_write (),
                signal_finished ()
            {
                if (total == 0)
                    this->total = std::numeric_limits<size_t>::max ();
            }

            // data
            CURL     *handle;
            Download &parent;
            size_t    offset;
            size_t    downloaded;
            size_t    total;
            bool      running;
            bool      curl_callback_running;
            bool      cancelled;

            // signals
            signal_header_t   signal_header;
            signal_progress_t signal_progress;
            signal_write_t    signal_write;
            signal_started_t  signal_started;
            signal_stopped_t  signal_stopped;
            signal_finished_t signal_finished;
        };

        // constructor
        Chunk::Chunk (Download &parent, size_t offset, size_t total) :
            _priv (new Private (parent, offset, total))
        {
        }

        // destructor
        Chunk::~Chunk ()
        {
            // we're screwed if this happens
            g_assert (!_priv->curl_callback_running);

            stop ();
        }

        //member functions
        void Chunk::start ()
        {
            if (running ()) return;

            // create chunk
            _priv->handle = curl_easy_init ();
            curl_easy_setopt (handle (), CURLOPT_URL,
                              _priv->parent.url ().c_str ());
            curl_easy_setopt (handle (), CURLOPT_RESUME_FROM, tell ());

            // hack to tel libcurl to pass the range anyway to induce a 206
            if (tell () == 0) {
                struct curl_slist *headers = NULL;
                std::ostringstream ss;
                ss << "Range: bytes=" << tell () << "-";
                headers = curl_slist_append (headers,
                                             ss.str ().c_str ());
                curl_easy_setopt (handle (), CURLOPT_HTTPHEADER, headers);
            }

            // make curl pass this into the callbacks
            curl_easy_setopt (handle (), CURLOPT_WRITEDATA, this);
            curl_easy_setopt (handle (), CURLOPT_PROGRESSDATA, this);
            curl_easy_setopt (handle (), CURLOPT_HEADERDATA, this);

            // bind the callbacks
            curl_easy_setopt (handle (), CURLOPT_WRITEFUNCTION,
                              &on_curl_write);
            curl_easy_setopt (handle (), CURLOPT_PROGRESSFUNCTION,
                              &on_curl_progress);
            curl_easy_setopt (handle (), CURLOPT_HEADERFUNCTION,
                              &on_curl_header);


            Manager::get ()->add_handle (this);
            _priv->signal_started.emit ();
        }

        void Chunk::stop ()
        {
            if (!running ()) return;

            // we're in the middle of a callback, so set cancelled, and wait for
            // curl to get rid of us. then stop_finished will be called which
            // will call us again
            if (_priv->curl_callback_running) {
                _priv->cancelled = true;
                return;
            }

            Manager::get ()->remove_handle (this);
            _priv->signal_stopped.emit ();
            curl_easy_cleanup (_priv->handle);
            _priv->handle = NULL;
            _priv->cancelled = false;
        }

        void Chunk::stop_finished (CURLcode result)
        {
            long code;
            curl_easy_getinfo (handle (), CURLINFO_RESPONSE_CODE, &code);
            stop ();
            _priv->signal_finished.emit (result);
        }

        void Chunk::merge (Chunk &previous_chunk)
        {
            // stop the previous chunk
            previous_chunk.stop ();

            g_assert (previous_chunk.tell () >= this->offset ());

            size_t new_offset = previous_chunk.offset ();
            size_t cur_pos = std::max (previous_chunk.tell (), tell());
            size_t end_pos = offset () + total ();

            // set new values for this chunk
            _priv->offset = new_offset;
            _priv->downloaded = cur_pos - new_offset;
            _priv->total = end_pos - new_offset;

            if (previous_chunk.running () || running ()) {
                // TODO: Make sure CURL* is restarted properly here
                stop ();
                start ();
            }
        }

        // signal accessors
        sigc::connection Chunk::connect_signal_header (
            Chunk::slot_header_t slot)
        {
            return _priv->signal_header.connect (slot);
        }

        sigc::connection Chunk::connect_signal_progress (
            Chunk::slot_progress_t slot)
        {
            return _priv->signal_progress.connect (slot);
        }

        sigc::connection Chunk::connect_signal_write (
            Chunk::slot_write_t slot)
        {
            return _priv->signal_write.connect (slot);
        }

        sigc::connection Chunk::connect_signal_started (
            Chunk::slot_started_t slot)
        {
            return _priv->signal_started.connect (slot);
        }

        sigc::connection Chunk::connect_signal_stopped (
            Chunk::slot_stopped_t slot)
        {
            return _priv->signal_stopped.connect (slot);
        }

        sigc::connection Chunk::connect_signal_finished (
            Chunk::slot_finished_t slot)
        {
            return _priv->signal_finished.connect (slot);
        }

        // I/O status accessors
        bool Chunk::running () const
        {
            return !_priv->cancelled && handle ();
        }

        size_t Chunk::offset() const
        {
            return _priv->offset;
        }

        size_t Chunk::downloaded () const
        {
            return _priv->downloaded;
        }

        size_t Chunk::total () const
        {
            return _priv->total;
        }

        void Chunk::total (size_t new_total)
        {
            _priv->total = new_total;
        }

        size_t Chunk::tell () const
        {
            return offset () + downloaded ();
        }

        // other accessors
        CURL *Chunk::handle ()
        {
            return _priv->handle;
        }

        const CURL *Chunk::handle () const
        {
            return _priv->handle;
        }

        // static CURL callbacks
        size_t Chunk::on_curl_header (void *data, size_t size,
                                      size_t nmemb, void *obj)
        {
            Chunk *self = reinterpret_cast<Chunk*> (obj);

            // guard against invalid curl behaviour
            self->_priv->curl_callback_running = true;
            self->_priv->signal_header.emit (data, size * nmemb);
            self->_priv->curl_callback_running = false;

            return size * nmemb;
        }

        size_t Chunk::on_curl_progress (void *obj,
                double dltotal, double dlnow,
                double ultotal, double ulnow)
        {
            Chunk *self = reinterpret_cast<Chunk*> (obj);

            if (self->_priv->cancelled)
                return 1; // error

            self->_priv->curl_callback_running = true;
            self->_priv->signal_progress.emit (dltotal, dlnow, ultotal, ulnow);
            self->_priv->curl_callback_running = false;

            return 0;
        }

        size_t Chunk::on_curl_write (void *data, size_t size,
                                     size_t nmemb, void *obj)
        {
            Chunk *self = reinterpret_cast<Chunk*> (obj);

            // we're stopped, so don't do anything
            if (self->_priv->cancelled || self->downloaded () >= self->total ())
                return 0;

            size_t bytes_handled = std::min (self->total () -
                                             self->downloaded (),
                                             size * nmemb);

            self->_priv->curl_callback_running = true;
            self->_priv->signal_write.emit (data, bytes_handled);
            self->_priv->downloaded += bytes_handled;
            self->_priv->curl_callback_running = false;

            return bytes_handled;
        }
    };
};
