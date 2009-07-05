/*      chunk.h -- part of the Yatta! Download Manager
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

#ifndef YATTA_CURL_CHUNK_H
#define YATTA_CURL_CHUNK_H

#include <tr1/memory>
#include <curl/curl.h>
#include <sigc++/signal.h>

namespace Yatta
{
    namespace Curl
    {
        // forward declaration
        class Download;

        class Chunk
        {
            public:
                typedef std::tr1::shared_ptr<Chunk> Ptr;

                // typedefs for signals
                typedef sigc::signal<void, 
                        void* /*data*/, size_t /*size*/,
                        size_t /*nmemb*/> header_arrived_t;
                typedef sigc::signal<void, 
                    double /*dltotal*/, double /*dlnow*/,
                    double /*ultotal*/, double /*ulnow*/> progress_t;
                typedef header_arrived_t data_arrived_t;

                // constructor and destructor
                explicit Chunk (Download &parent);
                static Chunk::Ptr create (Download &parent);
                virtual ~Chunk ();

                // accessor functions
                header_arrived_t signal_header ();
                progress_t       signal_progress ();
                data_arrived_t   signal_write ();

                CURL *get_handle ();
            private:
                CURL *m_handle;
                Download &m_parent;

                // signals
                header_arrived_t m_signal_header;
                progress_t       m_signal_progress;
                data_arrived_t   m_signal_write;

                // static curl functions
                // header function
                static size_t
                header_cb (void *data, size_t size, size_t nmemb, void *obj);

                // progress function
                static size_t
                progress_cb (void *obj,
                        double dltotal,
                        double dlnow,
                        double ultotal,
                        double ulnow);

                // write function
                static size_t
                write_cb (void *data, size_t size, size_t nmemb, void *obj);
        };
    };
};

#endif // YATTA_CURL_CHUNK_H
