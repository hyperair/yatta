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
#include <sigc++/connection.h>

#include "download.h"
#include "ioqueue.h"
#include "manager.h"

namespace Yatta
{
    namespace Curl
    {
        // first the private class implementation
        struct Download::Private
        {
            Private (Glib::RefPtr<Manager> mgr,
                     const Glib::ustring &url,
                     const std::string &dirname,
                     const std::string &filename = "") :
                mgr (mgr),
                url (url),
                resumable (false),
                size (0),
                fileio (dirname, filename),
                check_resumable_connection ()
            {}

            typedef std::list<Chunk::Ptr> chunk_list_t;

            Glib::RefPtr<Manager> &        mgr;
            Glib::ustring    url;
            chunk_list_t     chunks;
            bool             resumable;
            size_t           size;
            IOQueue          fileio;
            sigc::connection check_resumable_connection;
        };

        // constructor
        Download::Download (Glib::RefPtr<Manager> mgr,
                            const Glib::ustring &url,
                            const std::string &dirname,
                            const std::string &filename) :
            sigc::trackable (),
            _priv (new Private (mgr, url, dirname, filename))
        {
        }

        // destructor
        Download::~Download ()
        {
        }

        // increase number of running chunks
        void Download::add_chunk ()
        {
            // first we have to find the new chunk's offset.
            size_t new_offset;
            Private::chunk_list_t::iterator iter_chunk_before;

            // if no chunks already exist, we start from the beginning
            if (_priv->chunks.empty ())
                new_offset = 0;
            else if (resumable ())
            {
                // find biggest undownloaded gap...
                size_t biggest_gap_size;

                // use two iterators at once, since we're looking at each
                // undownloaded gap
                for (Private::chunk_list_t::iterator j = _priv->chunks.begin (),
                         i = j++;
                     j != _priv->chunks.end ();
                     i = j++)
                {
                    size_t current_gap_size = \
                        (*j)->tell () - (*i)->offset ();

                    // if current isn't bigger than previous biggest, move on
                    if (current_gap_size <= biggest_gap_size)
                        continue;

                    // otherwise keep track of it
                    biggest_gap_size = current_gap_size;
                    iter_chunk_before = i;
                }

                // check the size between the last chunk and EOF
                if (size ()-1 >= biggest_gap_size)
                {
                    iter_chunk_before = --_priv->chunks.end ();
                    biggest_gap_size = size () - 1;
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
                .connect (sigc::bind<0>
                          (sigc::hide
                           (sigc::hide
                            (sigc::mem_fun (*this,
                                            &Download::signal_progress_cb))),
                           chunk));
            chunk->signal_write ()
                .connect (sigc::bind<0>
                          (sigc::mem_fun (*this, &Download::signal_write_cb),
                           chunk));

            // if this is the first chunk, check if it's resumable
            if (new_offset == 0)
                _priv->check_resumable_connection =
                    chunk->signal_header ()
                    .connect (sigc::hide (sigc::hide
                                          (sigc::hide
                                           (sigc::bind
                                            (sigc::mem_fun
                                             (*this,
                                              &Download::chunk_check_resumable),
                                             chunk)))));

            // insert the chunk into the list, and start the chunk downloading
            _priv->chunks.insert (iter_chunk_before, chunk);
            _priv->mgr->add_handle (chunk);
        }

        // decrease number of running chunks
        void Download::remove_chunk ()
        {
        }

        // accessor methods
        Glib::ustring Download::url () const
        {
            return _priv->url;
        }

        void Download::url (const Glib::ustring &url)
        {
            _priv->url = url;
        }

        bool Download::resumable () const
        {
            return _priv->resumable;
        }

        size_t Download::size () const
        {
            return _priv->size;
        }

        // slots for interfacing with chunks
        void Download::signal_header_cb (Chunk::Ptr chunk,
                                    void *data,
                                    size_t size,
                                    size_t nmemb)
        {
        }

        void Download::chunk_check_resumable (Chunk::Ptr chunk)
        {
            // we only want to be called once, so disconnect the slot
            _priv->check_resumable_connection.disconnect ();

            // TODO: check ftp properly as well
            long status;

            if (curl_easy_getinfo (chunk->handle (),
                                   CURLINFO_RESPONSE_CODE,
                                   &status)
                != CURLE_OK)
                return;

            _priv->resumable = (status == 206);
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
            _priv->fileio.write (chunk->tell (),
                                 data,
                                 size * nmemb);

            if (_priv->resumable)
                add_chunk ();
        }
    };
};
