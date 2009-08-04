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
                // typedefs
                typedef std::tr1::shared_ptr<Chunk> Ptr;
                typedef sigc::signal<void,
                        void* /*data*/, size_t /*size*/,
                        size_t /*nmemb*/> signal_header_t;
                typedef sigc::signal<void,
                    double /*dltotal*/, double /*dlnow*/,
                    double /*ultotal*/, double /*ulnow*/> signal_progress_t;
                typedef signal_header_t signal_write_t;
                typedef sigc::signal<void> signal_started_t;
                typedef signal_started_t signal_stopped_t;

                // constructors and destructorp
                explicit Chunk (Download &parent, size_t offset=0);
                static   Chunk::Ptr create (Download &parent, size_t offset=0);
                virtual  ~Chunk ();

                // accessor functions
                signal_header_t   signal_header ();
                signal_progress_t signal_progress ();
                signal_write_t    signal_write ();
                signal_started_t  signal_started ();
                signal_stopped_t  signal_stopped ();

                // offset accessor (beginning of this chunk)
                size_t get_offset () const;
                void   set_offset (const size_t & arg);
                // how many bytes in this chunk downloaded
                size_t get_downloaded () const;
                // current position accessor (offset + downloaded)
                size_t tell () const;

                CURL * get_handle ();

            protected:
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

            private:
                struct Private;
                std::tr1::shared_ptr<Private> _priv;
        };
    };
};

#endif // YATTA_CURL_CHUNK_H
