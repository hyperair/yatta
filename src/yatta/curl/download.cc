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

#include <queue>

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
        struct Download::Private
        {
            Private (const Glib::ustring &url,
                     const std::string &dirname,
                     const std::string &filename = "") :
                url (url),
                chunks (0),
                max_chunks (10),
                resumable (false),
                size (0),
                running (false),
                fileio (dirname, filename),
                check_resumable_connection ()
            {}

            Glib::ustring    url;
            chunk_list_t     chunks;
            unsigned short   max_chunks;
            bool             resumable;
            size_t           size;
            bool             running;
            IOQueue          fileio;
            sigc::connection check_resumable_connection;
        };

        // constructor
        Download::Download (const Glib::ustring &url,
                            const std::string &dirname,
                            const std::string &filename) :
            sigc::trackable (),
            _priv (new Private (url, dirname, filename))
        {
        }

        // destructor
        Download::~Download ()
        {
        }

        // increase number of chunks by num_chunks
        void Download::add_chunks (unsigned short num_chunks)
        {
            // sanity check
            g_assert (num_chunks > 0);

            // store biggest gaps here to create chunks later
            std::queue<std::pair<size_t /*gap*/,
                chunk_list_t::iterator /*next_chunk*/> > biggest_gaps;

            // if no chunks already exist, start from the beginning. we don't
            // want to add more than one chunk yet because we don't know whether
            // it's resumable or not
            if (_priv->chunks.empty ()) {
                chunk_ptr_t chunk (new Chunk (*this, 0));
                _priv->chunks.push_back (chunk);
                connect_chunk_signals (chunk, _priv->chunks.begin ());
                chunk->start ();
                return;
            } else if (!resumable ()) // if !resumable, no point adding
                return;

            // find biggest undownloaded gap. use two iterators at once, since
            // we're looking at the gap between i and j
            if (!_priv->chunks.empty ()) {
                for (chunk_list_t::iterator j = _priv->chunks.begin (),
                         i = j++;
                     j != _priv->chunks.end ();
                     i = j++)
                {
                    size_t current_gap_size = (*i)->offset () - (*j)->tell ();

                    // we only want gaps >= current biggest
                    if (!biggest_gaps.empty () &&
                        current_gap_size < biggest_gaps.back ().first)
                        continue;

                    // push to queue of new chunks
                    biggest_gaps.push (std::make_pair (current_gap_size, j));
                    if (biggest_gaps.size () > num_chunks) biggest_gaps.pop ();
                }

                // check the size between the last chunk and EOF
                size_t current_gap_size = size () -
                    (_priv->chunks.back ())->tell ();

                // if the last gap is big enough, add it to the list
                if (current_gap_size >= biggest_gaps.back ().first) {
                    biggest_gaps.push (std::make_pair (current_gap_size,
                                                       _priv->chunks.end ()));
                    if (biggest_gaps.size () > num_chunks) biggest_gaps.pop ();
                }
            }

            // add new chunks
            for (; !biggest_gaps.empty (); biggest_gaps.pop ()) {
                // new offset is centrepoint of gap
                size_t new_offset = (*biggest_gaps.front ().second)->tell () +
                    biggest_gaps.front ().first / 2;

                // create and start chunk
                chunk_ptr_t chunk (new Chunk (*this, new_offset));
                connect_chunk_signals (chunk, _priv->chunks.insert (
                                           biggest_gaps.front ().second,
                                           chunk));
                chunk->start ();
            }
        }

        // decrease number of running chunks
        void Download::stop_chunks (unsigned short num_chunks)
        {
            for (chunk_list_t::reverse_iterator i = _priv->chunks.rbegin ();
                 num_chunks > 0 && i != _priv->chunks.rend ();
                 ++i)
                (*i)->stop ();
        }

        void Download::start ()
        {
            // already started, don't do anything
            if (_priv->running) return;

            // set status and wake up all the chunks
            _priv->running = true;
            normalize_chunks ();
        }

        void Download::stop ()
        {
            // already stopped don't do anything
            if (!_priv->running) return;

            // set status and stop all chunks
            _priv->running = false;
            for (chunk_list_t::iterator i = _priv->chunks.begin ();
                 i != _priv->chunks.end ();
                 i++)
                (*i)->stop ();
        }

        // accessor methods
        unsigned short Download::running_chunks () const
        {
            unsigned short count = 0;

            for (chunk_list_t::iterator i = _priv->chunks.begin ();
                 i != _priv->chunks.end ();
                 i++) {
                if ((*i)->running ())
                    count++;
            }

            return count;
        }

        unsigned short Download::max_chunks () const
        {
            return _priv->max_chunks;
        }

        void Download::max_chunks (unsigned short max_chunks)
        {
            _priv->max_chunks = max_chunks;
            normalize_chunks ();
        }

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

        void Download::normalize_chunks ()
        {
            if (_priv->chunks.empty ()) {
                // no chunks yet. start the first chunk and return. we will be
                // called again when the resumable status is found
                add_chunks (1);
                return;
            }

            // if not resumable, there can only be one chunk so do nothing
            if (!resumable ()) return;

            int running_chunks = static_cast<int> (this->running_chunks ());
            int total_chunks = _priv->chunks.size ();
            int max_chunks = this->max_chunks ();

            if (running_chunks == max_chunks) return;

            // start more chunks if we haven't reached max
            if (running_chunks < max_chunks) {
                // if max > total, then start all existing chunks
                if (max_chunks >= total_chunks &&
                    running_chunks < total_chunks) {
                    for (chunk_list_t::iterator i = _priv->chunks.begin ();
                         i != _priv->chunks.end ();
                         i++)
                        (*i)->start ();

                    // all existing chunks are now running
                    // running_chunks = total_chunks;

                    // add remaining chunks to reach maximum
                    add_chunks (max_chunks - running_chunks);
                } else if (max_chunks < total_chunks &&
                           running_chunks < max_chunks) {
                    // start first (max_chunks - running_chunks) chunks. if they
                    // are too near, they'll finish soon and be merged
                    for (chunk_list_t::iterator i = _priv->chunks.begin ();
                         running_chunks < max_chunks &&
                             i != _priv->chunks.end ();
                         i++, running_chunks++)
                        (*i)->start ();
                }
            } else // running_chunks > max_chunks
                stop_chunks (running_chunks - max_chunks);
        }

        void Download::connect_chunk_signals (chunk_ptr_t chunk,
                                              chunk_list_t::iterator iter)
        {
            // connect callbacks to signals, with chunk bound to them
            chunk->connect_signal_header (
                sigc::bind<0> (
                    sigc::mem_fun (*this, &Download::on_chunk_header),
                    chunk));
            chunk->connect_signal_progress (
                sigc::bind<0> (
                    sigc::hide (
                        sigc::hide (
                            sigc::mem_fun (
                                *this,
                                &Download::on_chunk_progress))),
                    chunk));
            chunk->connect_signal_write (
                sigc::bind<0> (
                    sigc::mem_fun (*this,
                                   &Download::on_chunk_write),
                    iter));

            // if this is the first chunk, check if it's resumable
            if (chunk->offset () == 0)
                _priv->check_resumable_connection =
                    chunk->connect_signal_header (
                        sigc::hide (
                            sigc::hide (
                                sigc::hide (
                                    sigc::bind (
                                        sigc::mem_fun (
                                            *this,
                                            &Download::chunk_check_resumable),
                                        chunk)))));
        }

        // slots for interfacing with chunks
        void Download::on_chunk_header (chunk_wptr_t weak_chunk, void *data,
                                        size_t size, size_t nmemb)
        {
        }

        void Download::chunk_check_resumable (chunk_wptr_t weak_chunk)
        {
            chunk_ptr_t chunk = weak_chunk.lock ();
            // TODO: check ftp properly as well
            long status;

            if (curl_easy_getinfo (chunk->handle (),
                                   CURLINFO_RESPONSE_CODE,
                                   &status) != CURLE_OK)
                return;

            _priv->resumable = (status == 206);
            normalize_chunks ();

            // we only want to be called once, so disconnect the slot
            _priv->check_resumable_connection.disconnect ();
        }

        void Download::on_chunk_progress (chunk_wptr_t weak_chunk,
                                          double dltotal,
                                          double dlnow)
        {
            // TODO: Add something here or get rid of it for good
        }

        void Download::on_chunk_write (chunk_list_t::iterator iter,
                                       void *data,
                                       size_t size,
                                       size_t nmemb)
        {
            chunk_ptr_t chunk = *iter;
            _priv->fileio.write (chunk->tell (),
                                 data,
                                 size * nmemb);

            g_assert (iter != _priv->chunks.end ());

            // look at the next chunk and merge if necessary
            if (++iter != _priv->chunks.end () &&
                (*iter)->offset () <= chunk->tell ())
            {
                (*iter)->merge (*chunk);
                (*iter)->start ();
                _priv->chunks.erase (--iter);
            }
        }
    };
};
