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
#include <limits>

#include <sigc++/bind.h>
#include <sigc++/hide.h>
#include <sigc++/signal.h>
#include <sigc++/connection.h>

#include "download.hh"
#include "../ioqueue.hh"
#include "manager.hh"

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
                max_chunks (30),
                resumable (false),
                size (0),
                running (false),
                fileio (dirname, filename),
                check_resumable_connection (),
                get_size_connection ()
            {}

            Glib::ustring      url;
            chunk_list_t       chunks;
            unsigned short     max_chunks;
            bool               resumable;
            size_t             size;
            bool               running;
            IOQueue            fileio;
            sigc::signal<void> signal_started;
            sigc::signal<void> signal_finished;
            sigc::signal<void> signal_stopped;
            sigc::connection   check_resumable_connection;
            sigc::connection   get_size_connection;
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
                connect_chunk_signals (chunk);
                chunk->start ();
                return;
            } else if (!resumable () || size () == 0)
                // if !resumable, no point adding
                return;

            // ! _priv->chunks.empty ()
            // look for largest undownloaded gap
            for (chunk_list_t::iterator i = _priv->chunks.begin ();
                 i != _priv->chunks.end ();
                 ++i)
            {
                chunk_list_t::iterator next = i;
                next++;
                size_t next_offset = next == _priv->chunks.end () ?
                    size () : (*next)->offset ();
                size_t current_gap = next_offset - (*i)->tell ();

                // keep track of all the biggest gaps
                if (biggest_gaps.empty () ||
                    current_gap >= biggest_gaps.back ().first)
                    biggest_gaps.push (std::make_pair (current_gap, next));

                // trim size of queue
                if (biggest_gaps.size () > num_chunks) biggest_gaps.pop ();
            }
            // biggest_gaps.size () >= 1

            // calculate how many new chunks per gap we want
            unsigned short chunks_per_gap = num_chunks / biggest_gaps.size ();
            unsigned short remainder =
                num_chunks - chunks_per_gap * biggest_gaps.size ();

            // add new chunks
            // add some chunks at the beginning first with the remainder
            add_chunks (chunks_per_gap + remainder, biggest_gaps.front ().first,
                        biggest_gaps.front ().second);
            biggest_gaps.pop ();
            for (; !biggest_gaps.empty (); biggest_gaps.pop ())
                add_chunks (chunks_per_gap, biggest_gaps.front ().first,
                            biggest_gaps.front ().second);
        }

        // add num_chunks for given gap size at iter location
        void Download::add_chunks (unsigned short num_chunks,
                                   size_t gap_size,
                                   Download::chunk_list_t::iterator iter)
        {
            size_t new_chunk_offset = iter == _priv->chunks.end () ?
                size () : (*iter)->offset ();
            size_t size_per_chunk = gap_size / num_chunks;

            for (unsigned short i = 0;
                 i < num_chunks;
                 ++i)
            {
                new_chunk_offset -= size_per_chunk;
                chunk_ptr_t chunk (new Chunk (*this, new_chunk_offset,
                                              size_per_chunk));
                iter = _priv->chunks.insert (iter, chunk);
                connect_chunk_signals (chunk);
                chunk->start ();
            }

            // set the previous chunk's new total to be downloaded
            if (iter != _priv->chunks.begin ()) {
                iter--;
                (*iter)->total (new_chunk_offset - (*iter)->offset ());
            }
        }

        // decrease number of running chunks
        void Download::stop_chunks (unsigned short num_chunks)
        {
            for (chunk_list_t::reverse_iterator i = _priv->chunks.rbegin ();
                 num_chunks && i != _priv->chunks.rend ();
                 ++i, --num_chunks)
                (*i)->stop ();
        }

        void Download::start ()
        {
            // already started, don't do anything
            if (_priv->running) return;

            // set status and wake up all the chunks
            _priv->running = true;
            normalize_chunks ();

            _priv->signal_started.emit ();
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

            _priv->signal_stopped.emit ();
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

        sigc::connection
        Download::connect_signal_started (const sigc::slot<void> &slot)
        {
            return _priv->signal_started.connect (slot);
        }

        sigc::connection
        Download::connect_signal_stopped (const sigc::slot<void> &slot)
        {
            return _priv->signal_stopped.connect (slot);
        }

        sigc::connection
        Download::connect_signal_finished (const sigc::slot<void> &slot)
        {
            return _priv->signal_finished.connect (slot);
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
                if (max_chunks >= total_chunks) {
                    if (running_chunks < total_chunks)
                        for (chunk_list_t::iterator i = _priv->chunks.begin ();
                             i != _priv->chunks.end ();
                             i++)
                            (*i)->start ();

                    // all existing chunks are now running
                    // running_chunks = total_chunks;

                    // add remaining chunks to reach maximum
                    add_chunks (max_chunks - total_chunks);
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

        void Download::connect_chunk_signals (chunk_ptr_t chunk)
        {
            // connect callbacks to signals, with chunk bound to them
            chunk->connect_signal_header (
                sigc::bind<0> (
                    sigc::mem_fun (*this, &Download::on_chunk_header),
                    chunk_wptr_t (chunk)));
            chunk->connect_signal_progress (
                sigc::bind<0> (
                    sigc::hide (
                        sigc::hide (
                            sigc::mem_fun (
                                *this,
                                &Download::on_chunk_progress))),
                    chunk_wptr_t (chunk)));
            chunk->connect_signal_write (
                sigc::bind<0> (
                    sigc::mem_fun (*this,
                                   &Download::on_chunk_write),
                    chunk_wptr_t (chunk)));

            // hook up to the chunk's finish signal
            chunk->connect_signal_finished (
                sigc::bind<0> (
                    sigc::hide (
                        sigc::mem_fun (*this,
                                       &Download::on_chunk_finished)),
                    chunk_wptr_t (chunk)));

            // if this is the first chunk, check if it's resumable
            if (chunk->offset () == 0) {
                _priv->check_resumable_connection =
                    chunk->connect_signal_header (
                        sigc::hide (
                            sigc::hide (
                                sigc::bind (
                                    sigc::mem_fun (
                                        *this,
                                        &Download::chunk_check_resumable),
                                    chunk_wptr_t (chunk)))));
                _priv->get_size_connection = chunk->connect_signal_header (
                    sigc::hide (
                        sigc::hide (
                            sigc::bind (
                                sigc::mem_fun (
                                    *this,
                                    &Download::chunk_get_size),
                                chunk_wptr_t (chunk)))));
            }
        }

        // slots for interfacing with chunks
        void Download::on_chunk_header (chunk_wptr_t weak_chunk, void *data,
                                        size_t bytes)
        {
            // TODO: if filename is empty, here's where we figure it out
        }

        void Download::chunk_check_resumable (chunk_wptr_t weak_chunk)
        {
            chunk_ptr_t chunk = weak_chunk.lock ();
            // TODO: check ftp properly as well
            long status;

            if (curl_easy_getinfo (chunk->handle (),
                                   CURLINFO_RESPONSE_CODE,
                                   &status) != CURLE_OK ||
                status == 0)
                return;

            _priv->resumable = (status == 206);
            normalize_chunks ();

            // we have our data, it's not going to change, so disconnect
            _priv->check_resumable_connection.disconnect ();
        }

        void Download::chunk_get_size (chunk_wptr_t weak_chunk)
        {
            chunk_ptr_t chunk = weak_chunk.lock ();

            double size;
            // if size isn't provided, return.
            if (curl_easy_getinfo (chunk->handle (),
                                   CURLINFO_CONTENT_LENGTH_DOWNLOAD,
                                   &size) != CURLE_OK ||
                size == 0 || size == -1)
                return;

            _priv->size = static_cast<size_t> (size);
            normalize_chunks ();

            // we have our data, so disconnect
            _priv->get_size_connection.disconnect ();
        }

        void Download::on_chunk_progress (chunk_wptr_t weak_chunk,
                                          double dltotal,
                                          double dlnow)
        {
            // TODO: Add something here or get rid of it for good
        }

        void Download::on_chunk_write (chunk_wptr_t weak_chunk,
                                       void *data,
                                       size_t bytes)
        {
            chunk_ptr_t chunk = weak_chunk.lock ();
            _priv->fileio.write (chunk->tell (),
                                 data,
                                 bytes);
        }

        void Download::on_chunk_finished (Download::chunk_wptr_t weak_chunk)
        {
            chunk_ptr_t chunk = weak_chunk.lock ();

            // check if download has completed
            if (chunk->offset () == 0 && size () == chunk->tell ()) {
                stop ();
                _priv->signal_finished.emit ();
            } else if (chunk->downloaded () < chunk->total ()) {
                // if ended prematurely, restart chunk
                chunk->start ();
            } else { // not done. search for next chunk and merge
                chunk_list_t::iterator i;
                for (i = _priv->chunks.begin ();
                     i != _priv->chunks.end () && *i != chunk;
                     ++i);

                // we should only be called with a chunk that exists
                g_assert (i != _priv->chunks.end ());

                // merge with next chunk if need be
                chunk_list_t::iterator next = i;
                next++;
                if (next != _priv->chunks.end ()) {
                    (*next)->merge (*chunk);
                    _priv->chunks.erase (i);
                }
            }
        }
    }
}
