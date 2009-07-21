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
        Chunk::Chunk (Download &parent, size_t offset=0) :
            m_handle (curl_easy_init ()),
            m_parent (parent),
            m_offset (offset)
        {
            // set some curl options...
            curl_easy_setopt (m_handle, CURLOPT_URL, parent.get_uri ());
            curl_easy_setopt (m_handle, CURLOPT_RESUME_FROM, offset);

            // make curl pass this into the callbacks
            curl_easy_setopt (m_handle, CURLOPT_WRITEDATA, this);
            curl_easy_setopt (m_handle, CURLOPT_PROGRESSDATA, this);
            curl_easy_setopt (m_handle, CURLOPT_HEADERDATA, this);

            // bind the callbacks
            curl_easy_setopt (m_handle, CURLOPT_WRITEFUNCTION, &write_cb);
            curl_easy_setopt (m_handle, CURLOPT_PROGRESSFUNCTION, &progress_cb);
            curl_easy_setopt (m_handle, CURLOPT_HEADERFUNCTION, &header_cb);
        }

        Chunk::Ptr Chunk::create (Download &parent, size_t offset=0)
        {
            return Chunk::Ptr (new Chunk(parent, offset));
        }

        Chunk::~Chunk ()
        {
        }

        CURL *Chunk::get_handle ()
        {
            return m_handle;
        }

        size_t
        Chunk::header_cb (void *data, size_t size, size_t nmemb, void *obj)
        {
            reinterpret_cast<Chunk*> (obj)->m_signal_header (data, size, nmemb);
        }

        size_t
        Chunk::progress_cb (void *obj,
                double dltotal, double dlnow,
                double ultotal, double ulnow)
        {
            reinterpret_cast<Chunk*> (obj)->m_signal_progress (dltotal,
                    dlnow, ultotal, ulnow);
        }

        size_t
        Chunk::write_cb (void *data, size_t size, size_t nmemb, void *obj)
        {
            reinterpret_cast<Chunk*> (obj)->m_signal_write (data, size, nmemb);
        }

        // accessor methods
        // signals
        signal_header_t Chunk::signal_header ()
        {
            return m_signal_header;
        }
        signal_progress_t Chunk::signal_progress ()
        {
            return m_signal_progress;
        }
        signal_write_t Chunk::signal_write ()
        {
            return m_signal_write;
        }

        // others
        size_t Chunk::get_offset()
        {
            return m_offset;
        }

        void Chunk::set_offset (const size_t &arg)
        {
            m_offset = arg;
        }
    };
};
