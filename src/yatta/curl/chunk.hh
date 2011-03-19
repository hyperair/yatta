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
#include <sigc++/slot.h>
#include <sigc++/connection.h>

#include "../download.hh"

namespace Yatta
{
    namespace Curl
    {
        class Chunk
        {
        public:
            // typedefs
            typedef sigc::slot<void, void* /*data*/,
                               size_t /*bytes*/> slot_header_t;
            typedef sigc::slot<void, double /*dltotal*/, double /*dlnow*/,
                               double /*ultotal*/,
                               double /*ulnow*/> slot_progress_t;
            typedef slot_header_t slot_write_t;
            typedef sigc::slot<void> slot_started_t;
            typedef slot_started_t slot_stopped_t;
            typedef sigc::slot<void, CURLcode> slot_finished_t;

            // constructors and destructors
            Chunk (Download &parent, size_t offset, size_t total=0);
            virtual  ~Chunk ();

            // member functions
            // start the chunk running
            void start ();
            // stop the chunk
            void stop ();

            // stop the chunk because it has finished (will emit
            // signal_finished)
            void stop_finished (CURLcode result);

            // merge previous_chunk into this.
            // precondition: previous_chunk.tell () >= this->offset ()
            void merge (Chunk &previous_chunk);

            // accessor functions
            // signal when header is received
            sigc::connection connect_signal_header (slot_header_t slot);

            // signal when libcurl calls progress function
            sigc::connection connect_signal_progress (slot_progress_t slot);

            // signal when there is stuff to write
            sigc::connection connect_signal_write (slot_write_t slot);

            // signal when chunk is added to the Manager
            sigc::connection connect_signal_started (slot_started_t slot);

            // signal when chunk is stopped
            sigc::connection connect_signal_stopped (slot_stopped_t slot);

            // signal when chunk has finished (might have errors)
            sigc::connection connect_signal_finished (slot_finished_t slot);

            // check if the chunk is running.
            bool   running () const;

            // offset accessor (beginning of this chunk)
            size_t offset () const;

            // how many bytes in this chunk downloaded
            size_t downloaded () const;

            // how many bytes to download before this chunk should stop itself
            void total (size_t total);
            size_t total () const;

            // current position accessor (offset + downloaded)
            size_t tell () const;

            CURL * handle ();
            const CURL * handle () const;

        protected:
            // static callbacks for CURL C library
            // header function
            static size_t on_curl_header (void *data, size_t size,
                                          size_t nmemb, void *obj);

            // progress function
            static size_t on_curl_progress (void *obj,
                                            double dltotal, double dlnow,
                                            double ultotal, double ulnow);

            // write function
            static size_t on_curl_write (void *data, size_t size,
                                         size_t nmemb, void *obj);

        private:
            struct Private;
            std::tr1::shared_ptr<Private> _priv;
        };
    }
}

#endif // YATTA_CURL_CHUNK_H
