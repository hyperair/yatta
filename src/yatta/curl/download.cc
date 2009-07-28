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

#include <sigc++/bind.h>
#include <sigc++/hide.h>

#include "download.h"
#include "manager.h"

namespace Yatta
{
    namespace Curl
    {
        Download::Download (const Glib::ustring &url,
                Manager &mgr) :
            sigc::trackable (),
            m_mgr (mgr),
            m_url (url)
        {
        }

        Download::~Download ()
        {
        }

        void Download::add_chunk ()
        {
            // first we have to find the new chunk's offset.
            size_t new_offset;
            chunk_list_t::iterator iter_chunk_before;

            // if no chunks already exist, we start from the beginning
            if (m_chunks.empty ())
                new_offset = 0;
            else if (resumable ())
            {
                // find biggest undownloaded gap...
                size_t biggest_gap_size;

                // use two iterators at once, since we're looking at each
                // undownloaded gap
                for (chunk_list_t::iterator j = m_chunks.begin (), i = j++;
                     j != m_chunks.end ();
                     i = j++)
                {
                    size_t current_gap_size = (*j)->tell ()
                                              - (*i)->get_offset ();

                    // if current isn't bigger than previous biggest, move on
                    if (current_gap_size <= biggest_gap_size)
                        continue;

                    // otherwise keep track of it
                    biggest_gap_size = current_gap_size;
                    iter_chunk_before = i;
                }

                // check the size between the last chunk and EOF
                if (get_size ()-1 >= biggest_gap_size)
                {
                    iter_chunk_before = --m_chunks.end ();
                    biggest_gap_size = get_size () - 1;
                }

                // new offset is centrepoint of the largest undownloaded gap
                new_offset = (*iter_chunk_before)->tell ()
                             + biggest_gap_size / 2;
            }

            Chunk::Ptr chunk (Chunk::create (*this, new_offset));

            // connect callbacks to signals, with chunk bound to them
            chunk->signal_header ()
                .connect (sigc::bind<0>
                            (sigc::mem_fun (*this, &Download::signal_header_cb),
                             chunk));
            chunk->signal_progress ()
                .connect (sigc::bind<0> (sigc::hide (sigc::hide
                            (sigc::mem_fun (*this,
                                 &Download::signal_progress_cb))),
                             chunk));
            chunk->signal_write ()
                .connect (sigc::bind<0>
                            (sigc::mem_fun (*this, &Download::signal_write_cb),
                             chunk));

            // insert the chunk into the list, and start the chunk downloading
            m_chunks.insert (iter_chunk_before, chunk);
            m_mgr.add_handle (chunk);
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

        bool Download::resumable () const
        {
            return m_resumable;
        }

        size_t Download::get_size () const
        {
            return m_size;
        }

        void Download::signal_header_cb (Chunk::Ptr chunk,
                void *data,
                size_t size,
                size_t nmemb)
        {
        }
        void Download::signal_progress_cb (Chunk::Ptr chunk,
                double dltotal,
                double dlnow)
        {
        }
        void Download::signal_write_cb (Chunk::Ptr chunk,
                void *data,
                size_t size,
                size_t nmemb)
        {
            // first check if resumable. for now, only http supported.
            // TODO: make ftp work, and then other protocols
            long status;
            curl_easy_getinfo (chunk->get_handle (),
                    CURLINFO_RESPONSE_CODE,
                    &status);
            m_resumable = (status == 206);
            // TODO: add more chunks here if the limit hasn't been reached
            // TODO: actually write to file
        }
    };
};
