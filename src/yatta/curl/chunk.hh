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

#include "../chunk.hh"

namespace Yatta
{
    namespace Curl
    {
        class Chunk : public ::Yatta::Chunk
        {
        public:
            // constructors and destructors
            Chunk (const std::string &url,
                   size_t offset,
                   size_t size);

            virtual  ~Chunk ();

            virtual void start ();
            virtual void stop ();

            virtual bool resumable () const;
            virtual size_t content_length() const;

            // stop the chunk because it has finished (will emit
            // signal_finished)
            void stop_finished (CURLcode result);

            CURL * handle ();
            const CURL * handle () const;

        private:
            struct Private;
            friend class Private;
            std::tr1::shared_ptr<Private> _priv;
        };
    }
}

#endif // YATTA_CURL_CHUNK_H
