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
        Chunk::Chunk (Download &parent) :
            m_handle (curl_easy_init ()),
            m_parent (parent)
        {
            // make curl pass this into the callbacks
            curl_easy_setopt (m_handle, CURLOPT_WRITEDATA, this);
            curl_easy_setopt (m_handle, CURLOPT_PROGRESSDATA, this);
            curl_easy_setopt (m_handle, CURLOPT_HEADERDATA, this);

            // bind the callbacks
            curl_easy_setopt (m_handle, CURLOPT_WRITEFUNCTION, &write_cb);
            curl_easy_setopt (m_handle, CURLOPT_PROGRESSFUNCTION, &progress_cb);
            curl_easy_setopt (m_handle, CURLOPT_HEADERFUNCTION, &header_cb);
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
        }

        size_t
        Chunk::progress_cb (void *obj,
                double dltotal, double dlnow,
                double ultotal, double ulnow)
        {
        }

        size_t
        Chunk::write_cb (void *data, size_t size, size_t nmemb, void *obj)
        {
        }
    };
};
