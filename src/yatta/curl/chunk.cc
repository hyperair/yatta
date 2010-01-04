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
#include <sigc++/signal.h>

#include "chunk.h"
#include "download.h"
#include "manager.h"

namespace Yatta
{
    namespace Curl
    {
        // typedefs
        typedef sigc::signal<void,
                           void* /*data*/, size_t /*size*/,
                           size_t /*nmemb*/>
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
            Private (Download &parent, size_t offset) :
                handle (curl_easy_init ()),
                parent (parent),
                offset (offset),
                downloaded (0),
                total (0),
                running (false),
                signal_header (),
                signal_progress (),
                signal_write (),
                signal_finished ()
            {}

            // data
            CURL     *handle;
            Download &parent;
            size_t    offset;
            size_t    downloaded;
            size_t    total;
            bool      running;

            // signals
            signal_header_t   signal_header;
            signal_progress_t signal_progress;
            signal_write_t    signal_write;
            signal_started_t  signal_started;
            signal_stopped_t  signal_stopped;
            signal_finished_t signal_finished;
        };

        // constructor
        Chunk::Chunk (Download &parent, size_t offset) :
            _priv (new Private (parent, offset))
        {
            // set some curl options...
            curl_easy_setopt (handle (), CURLOPT_URL,
                              parent.url ().c_str ());
            curl_easy_setopt (handle (), CURLOPT_RESUME_FROM, offset);

            // hack to make libcurl pass the range anyway..
            if (offset == 0) {
                struct curl_slist *headers = NULL;
                std::ostringstream ss;
                ss << "Range: bytes=" << offset << "-";
                headers = curl_slist_append (
                    headers,
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
        }

        // destructor
        Chunk::~Chunk ()
        {
            // if we're still running, stop and disconnect from Manager
            if (_priv->running) stop();
        }

        //member functions
        void Chunk::start ()
        {
            Manager::get ()->add_handle (this);
            _priv->running = true;
            _priv->signal_started.emit ();
        }

        void Chunk::stop ()
        {
            Manager::get ()->remove_handle (this);
            _priv->running = false;
            _priv->signal_stopped.emit ();
        }

        void Chunk::stop_finished (CURLcode result)
        {
            _priv->signal_finished.emit (result);
            stop ();
        }

        void Chunk::merge (Chunk &previous_chunk)
        {
            g_assert (previous_chunk.tell () >= this->offset ());

            size_t new_offset = previous_chunk.offset ();
            size_t downloaded = MAX (previous_chunk.tell (), tell());

            // set new values for this chunk
            offset (new_offset);
            _priv->downloaded = downloaded;

            if (running ()) {
                start ();
                stop ();
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
            return _priv->running;
        }

        size_t Chunk::offset() const
        {
            return _priv->offset;
        }

        void Chunk::offset (const size_t &arg)
        {
            _priv->offset = arg;
        }

        size_t Chunk::downloaded () const
        {
            return _priv->downloaded;
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

        // static CURL callbacks
        size_t Chunk::on_curl_header (void *data, size_t size,
                                      size_t nmemb, void *obj)
        {
            Chunk *self = reinterpret_cast<Chunk*> (obj);

            self->_priv->signal_header.emit (data, size, nmemb);

            return size * nmemb;
        }

        size_t Chunk::on_curl_progress (void *obj,
                double dltotal, double dlnow,
                double ultotal, double ulnow)
        {
            Chunk *self = reinterpret_cast<Chunk*> (obj);

            // we only update the total to download here, but not the downloaded
            // bytes because it can skew what tell() says causing bytes to be
            // written to the wrong location
            self->_priv->total = dltotal;
            self->_priv->signal_progress.emit (dltotal, dlnow, ultotal, ulnow);

            return 0;
        }

        size_t Chunk::on_curl_write (void *data, size_t size,
                                     size_t nmemb, void *obj)
        {
            Chunk *self = reinterpret_cast<Chunk*> (obj);
            self->_priv->signal_write.emit (data, size, nmemb);
            self->_priv->downloaded += size * nmemb;
            return size * nmemb;
        }
    };
};
