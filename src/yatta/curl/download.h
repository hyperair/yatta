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

#include <list>
#include <glibmm/ustring.h>

#include "chunk.h"

namespace Yatta
{
    namespace Curl
    {
        class Download : public sigc::trackable
        {
            public:
                Download (const Glib::ustring &url);
                void add_chunk (); // increases number of running chunks
                void remove_chunk (); // decreases number of running chunks
                virtual ~Download ();

            private:
                const Glib::ustring m_url;
                std::list<Chunk::Ptr> m_chunks;
        };
    };
};

#endif // YATTA_CURL_DOWNLOAD_H
