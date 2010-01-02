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
        typedef std::list<Chunk::Ptr> chunk_list_t;

        struct Download::Private
        {
            Private (const Glib::ustring &url,
                     const std::string &dirname,
                     const std::string &filename = "") :
                url (url),
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

            // if no chunks already exist, start from the beginning. no need to
            // jump directly to adding new chunks because the next two blocks
            // will no-op
            if (_priv->chunks.empty ())
                biggest_gaps.push (std::make_pair (0, _priv->chunks.end ()));

            // if not resumable, adding chunks won't make a difference
            else if (!resumable ())
                return;

            // find biggest undownloaded gap. use two iterators at once, since
            // we're looking at the gap between i and j
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
            if (!_priv->chunks.empty ()) {
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
                Chunk::Ptr chunk (Chunk::create (*this, new_offset));

                connect_chunk_signals (chunk);
                _priv->chunks.insert (biggest_gaps.front ().second, chunk);
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

        void Download::connect_chunk_signals (Chunk::Ptr chunk)
        {
            // connect callbacks to signals, with chunk bound to them
            chunk->signal_header ()
                .connect (sigc::bind<0>
                          (sigc::mem_fun (*this, &Download::on_chunk_header),
                           chunk));
            chunk->signal_progress ()
                .connect (sigc::bind<0>
                          (sigc::hide
                           (sigc::hide
                            (sigc::mem_fun (*this,
                                            &Download::on_chunk_progress))),
                           chunk));
            chunk->signal_write ()
                .connect (sigc::bind<0>
                          (sigc::mem_fun (*this, &Download::on_chunk_write),
                           chunk));

            // if this is the first chunk, check if it's resumable
            if (chunk->offset () == 0)
                _priv->check_resumable_connection =
                    chunk->signal_header ()
                    .connect (sigc::hide (sigc::hide
                                          (sigc::hide
                                           (sigc::bind
                                            (sigc::mem_fun
                                             (*this,
                                              &Download::chunk_check_resumable),
                                             chunk)))));
        }

        // slots for interfacing with chunks
        void Download::on_chunk_header (Chunk::Ptr chunk,
                                    void *data,
                                    size_t size,
                                    size_t nmemb)
        {
        }

        void Download::chunk_check_resumable (Chunk::Ptr chunk)
        {
            // TODO: check ftp properly as well
            long status;

            if (curl_easy_getinfo (chunk->handle (),
                                   CURLINFO_RESPONSE_CODE,
                                   &status)
                != CURLE_OK)
                return;

            _priv->resumable = (status == 206);
            normalize_chunks ();

            // we only want to be called once, so disconnect the slot
            _priv->check_resumable_connection.disconnect ();
        }

        void Download::on_chunk_progress (Chunk::Ptr chunk,
                                          double dltotal,
                                          double dlnow)
        {
        }

        void Download::on_chunk_write (Chunk::Ptr chunk,
                                       void *data,
                                       size_t size,
                                       size_t nmemb)
        {
            // TODO: check and merge with the next chunk if necessary
            _priv->fileio.write (chunk->tell (),
                                 data,
                                 size * nmemb);
        }
    };
};
