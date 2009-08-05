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

#include "chunk.h"
#include "download.h"

namespace Yatta
{
    namespace Curl
    {
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
                signal_header (),
                signal_progress (),
                signal_write (),
                signal_finished ()
            {}

            // data
            CURL *handle;
            Download &parent;
            size_t offset;
            size_t downloaded;
            size_t total;

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
            curl_easy_setopt (get_handle (), CURLOPT_URL,
                              parent.get_url ().c_str ());
            curl_easy_setopt (get_handle (), CURLOPT_RESUME_FROM, offset);

            // make curl pass this into the callbacks
            curl_easy_setopt (get_handle (), CURLOPT_WRITEDATA, this);
            curl_easy_setopt (get_handle (), CURLOPT_PROGRESSDATA, this);
            curl_easy_setopt (get_handle (), CURLOPT_HEADERDATA, this);

            // bind the callbacks
            curl_easy_setopt (get_handle (), CURLOPT_WRITEFUNCTION, &write_cb);
            curl_easy_setopt (get_handle (), CURLOPT_PROGRESSFUNCTION,
                              &progress_cb);
            curl_easy_setopt (get_handle (), CURLOPT_HEADERFUNCTION,
                              &header_cb);
        }

        // static convenience wrapper to constructor
        Chunk::Ptr Chunk::create (Download &parent, size_t offset)
        {
            return Chunk::Ptr (new Chunk(parent, offset));
        }

        // destructor
        Chunk::~Chunk ()
        {
        }

        // signal accessors
        Chunk::signal_header_t
        Chunk::signal_header ()
        {
            return _priv->signal_header;
        }

        Chunk::signal_progress_t
        Chunk::signal_progress ()
        {
            return _priv->signal_progress;
        }

        Chunk::signal_write_t
        Chunk::signal_write ()
        {
            return _priv->signal_write;
        }

        Chunk::signal_started_t
        Chunk::signal_started ()
        {
            return _priv->signal_started;
        }

        Chunk::signal_stopped_t
        Chunk::signal_stopped ()
        {
            return _priv->signal_stopped;
        }

        Chunk::signal_finished_t
        Chunk::signal_finished ()
        {
            return _priv->signal_finished;
        }

        // I/O status accessors
        size_t
        Chunk::get_offset() const
        {
            return _priv->offset;
        }
        void
        Chunk::set_offset (const size_t &arg)
        {
            _priv->offset = arg;
        }

        size_t
        Chunk::get_downloaded () const
        {
            return _priv->downloaded;
        }

        size_t
        Chunk::tell () const
        {
            return _priv->offset + _priv->downloaded;
        }

        // other accessors
        CURL *
        Chunk::get_handle ()
        {
            return _priv->handle;
        }

        // static CURL callbacks
        size_t
        Chunk::header_cb (void *data, size_t size, size_t nmemb, void *obj)
        {
            reinterpret_cast<Chunk*> (obj)->signal_header ()
                .emit (data, size, nmemb);
        }

        size_t
        Chunk::progress_cb (void *obj,
                double dltotal, double dlnow,
                double ultotal, double ulnow)
        {
            Chunk *self = reinterpret_cast<Chunk*> (obj);
            // keep track of where we are and how much to download in order for
            // tell () to work.
            self->_priv->downloaded = dlnow;
            self->_priv->total = dltotal;
            self->signal_progress ()
                .emit (dltotal, dlnow, ultotal, ulnow);
        }

        size_t
        Chunk::write_cb (void *data, size_t size, size_t nmemb, void *obj)
        {
            reinterpret_cast<Chunk*> (obj)->signal_write ()
                .emit (data, size, nmemb);
        }
    };
};
