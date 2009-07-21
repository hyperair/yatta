/*      manager.cc -- part of the Yatta! Download Manager
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

#include "manager.h"
#include "chunk.h"

#include <glibmm/timer.h>

namespace Yatta
{
    namespace Curl
    {
        Manager::Manager () :
            m_multihandle (NULL),
            m_sharehandle (NULL),
            m_running_handles (0),
            m_exiting (false),
            m_select_thread (NULL)

        {
            // must initialize curl globally first!
            curl_global_init (CURL_GLOBAL_ALL);

            // then initialize the curl handles
            m_multihandle = curl_multi_init ();
            m_sharehandle = curl_share_init ();

            // connect perform function to dispatcher
            m_curl_ready.connect (sigc::mem_fun (*this, &Manager::perform));

            // start thread
            m_select_thread = Glib::Thread::create 
                (sigc::mem_fun (*this, &Manager::select_thread), true);
        }

        Manager::~Manager ()
        {
            m_exiting = true;
            // wake up the sleeping thread (if it is) before joining
            m_multihandle_notempty.broadcast ();
            m_select_thread->join ();
            curl_global_cleanup ();
        }

        void
        Manager::add_handle (Chunk::Ptr chunk)
        {
            CURL *handle = chunk->get_handle ();

            curl_easy_setopt (handle,
                    CURLOPT_SHARE,
                    m_sharehandle);
            {
                Glib::Mutex::Lock lock (m_multihandle_mutex);
                curl_multi_add_handle (m_multihandle, handle);
            }

            m_chunkmap.insert (std::make_pair (handle, chunk));
            m_running_handles++;

            m_multihandle_notempty.broadcast ();
        }

        void
        Manager::remove_handle (Chunk::Ptr chunk)
        {
            CURL *handle = chunk->get_handle ();

            {
                Glib::Mutex::Lock lock (m_multihandle_mutex);
                curl_multi_remove_handle (m_multihandle, handle);
            }
            m_chunkmap.erase (handle);
            m_running_handles--;

            m_multihandle_notempty.broadcast ();
        }

        void
        Manager::perform ()
        {
            Glib::Mutex::Lock lock (m_multihandle_mutex); // lock multihandle
            int prev_running_handles = m_running_handles;
            while (curl_multi_perform (m_multihandle,
                                       &m_running_handles) ==
                   CURLM_CALL_MULTI_PERFORM)
            {
                if (prev_running_handles != m_running_handles)
                {
                    // TODO: handle finished handle(s)
                }
                prev_running_handles = m_running_handles;
            }
        }

        void
        Manager::select_thread ()
        {
            fd_set read_fds, write_fds, error_fds;
            int nfds; // number of FDs
            long timeout; // timeout in ms
            struct timeval tv; // timeout to be passed into select

            // loop until we want to exit
            while (!m_exiting)
            {
                FD_ZERO (&read_fds);
                FD_ZERO (&write_fds);
                FD_ZERO (&error_fds);

                // grab information from curl for select()
                {
                    Glib::Mutex::Lock lock (m_multihandle_mutex);

                    // wait here until there are handles, or time to exit
                    while (curl_multi_fdset (m_multihandle,
                                &read_fds, &write_fds, &error_fds, &nfds),
                            nfds == -1 &&
                            !m_exiting)
                        m_multihandle_notempty.wait (m_multihandle_mutex);

                    // if it's time to exit, just do so already
                    if (m_exiting)
                        throw Glib::Thread::Exit();

                    // then get the timeout
                    curl_multi_timeout (m_multihandle, &timeout);
                }

                if (nfds == -1)
                    continue;

                // convert milliseconds into timeval
                tv.tv_sec = timeout * 1000;
                tv.tv_usec = timeout / 1000;

                // call curl if timeout or fd ready
                select (nfds,
                        &read_fds,
                        &write_fds,
                        &error_fds,
                        (timeout<0)?NULL:&tv);

                m_curl_ready.emit ();
            }
        }
    };
};
