/*      download.cc -- part of the Yatta! Download Manager
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

#include "download.h"

namespace Yatta
{
    namespace Curl
    {
        Download::Download (const Glib::ustring &url) :
            sigc::trackable (),
            m_url (url)
        {
        }

        Download::~Download ()
        {
        }

        void Download::add_chunk ()
        {
            Chunk::Ptr chunk = Chunk::create (*this);
            m_chunks.push_back (chunk);
        }

        void Download::remove_chunk ()
        {
        }
    };
};
