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
            Chunk::Ptr chunk (Chunk::create (*this, get_new_offset()));
            m_chunks.push_back (chunk);
        }

        void Download::remove_chunk ()
        {
        }

        Glib::ustring Download::get_url () const
        {
            return m_url;
        }

        void Download::set_url (const Glib::ustring &url)
        {
            m_url = url;
        }

        size_t Download::get_new_offset ()
        {
            // if no chunks already exist, start the first one
            if (m_chunks.empty () == 0)
                return 0;

            // find biggest undownloaded gap...
            chunk_list_t::iterator biggest_gap_front;
            size_t biggest_gap_size;

            for (chunk_list_t::iterator j = m_chunks.begin (), i = j++;
                 j != m_chunks.end ();
                 i = j++)
            {
                size_t current_gap_size = (*j)->tell () - (*i)->get_offset ();

                // if current isn't bigger than previous biggest, move on
                if (current_gap_size <= biggest_gap_size)
                    continue;

                biggest_gap_size = current_gap_size;
                biggest_gap_front = i;
            }

            // ...and return the centre point
            return (*biggest_gap_front)->get_offset () + biggest_gap_size / 2;
        }
    };
};
