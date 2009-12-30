/*      download.h -- part of the Yatta! Download Manager
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

#ifndef YATTA_CURL_DOWNLOAD_H
#define YATTA_CURL_DOWNLOAD_H

#include <tr1/memory>
#include <list>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

#include "chunk.h"

namespace Yatta
{
    namespace Curl
    {
        // forward declaration
        class Manager;

        class Download : public sigc::trackable
        {
            public:
            Download (const Glib::ustring &url,
                      const std::string &dirname,
                      const std::string &filename = "");
                virtual ~Download ();

                void add_chunk (); // increases number of running chunks
                void remove_chunk (); // decreases number of running chunks

                void start ();
                void stop ();

                // accessors
                unsigned short running_chunks () const;
                unsigned short max_chunks () const;
                void max_chunks (unsigned short max_chunks);

                Glib::ustring url () const;
                void url (const Glib::ustring &url);

                bool resumable() const;
                bool running () const;

                size_t size () const;

            protected:
                void normalize_chunks ();

                virtual void on_chunk_header (Chunk::Ptr chunk,
                                              void *data,
                                              size_t size,
                                              size_t nmemb);
                virtual void on_chunk_progress (Chunk::Ptr chunk,
                                                double dltotal,
                                                double dlnow);
                virtual void on_chunk_write (Chunk::Ptr chunk,
                                             void *data,
                                             size_t size,
                                             size_t nmemb);
                virtual void chunk_check_resumable (Chunk::Ptr chunk);
            private:
                struct Private;
                std::tr1::shared_ptr<Private> _priv;
        };
    };
};

#endif // YATTA_CURL_DOWNLOAD_H
