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
        struct Manager::Private
        {
            Private () :
                multihandle (NULL),
                sharehandle (NULL),
                running_handles (0),
                exiting (false),
                select_thread (NULL)
            {}

            CURLM *multihandle; // only multi handle which will be used
            CURLSH *sharehandle; // to share data between easy handles

            int running_handles; // number of running handles
            std::map<CURL*, Chunk::Ptr> chunkmap;

            Glib::Dispatcher curl_ready; // call curl_multi_perform
            Glib::Mutex multihandle_mutex; // lock for multihandle
            Glib::Cond multihandle_notempty; // not empty || exit time
            volatile bool exiting; // tell thread to exit
            Glib::Thread *select_thread; // select thread
        };

        Manager::Manager () :
            _priv (new Private())
        {
            // must initialize curl globally first!
            curl_global_init (CURL_GLOBAL_ALL);

            // then initialize the curl handles
           _priv->multihandle = curl_multi_init ();
           _priv->sharehandle = curl_share_init ();

            // connect perform function to dispatcher
           _priv->curl_ready.connect (sigc::mem_fun (*this, &Manager::perform));

            // start thread
           _priv->select_thread = Glib::Thread::create 
                (sigc::mem_fun (*this, &Manager::select_thread), true);
        }

        Manager::~Manager ()
        {
            // tell the thread we're exiting
            _priv->exiting = true;

            // wake up the sleeping thread (if it is) before joining
            _priv->multihandle_notempty.broadcast ();
            _priv->select_thread->join ();

            // now that the thread's done, clean up libcurl
            curl_multi_cleanup (_priv->multihandle);
            curl_share_cleanup (_priv->sharehandle);
            curl_global_cleanup ();
        }

        void
        Manager::add_handle (Chunk::Ptr chunk)
        {
            CURL *handle = chunk->get_handle ();

            curl_easy_setopt (handle,
                    CURLOPT_SHARE,
                    _priv->sharehandle);
            {
                Glib::Mutex::Lock lock (_priv->multihandle_mutex);
                curl_multi_add_handle (_priv->multihandle, handle);
            }

            _priv->chunkmap.insert (std::make_pair (handle, chunk));
            _priv->running_handles++;

            _priv->multihandle_notempty.broadcast ();
        }

        void
        Manager::remove_handle (Chunk::Ptr chunk)
        {
            CURL *handle = chunk->get_handle ();

            {
                Glib::Mutex::Lock lock (_priv->multihandle_mutex);
                curl_multi_remove_handle (_priv->multihandle, handle);
            }
            _priv->chunkmap.erase (handle);
            _priv->running_handles--;
        }

        void
        Manager::perform ()
        {
            Glib::Mutex::Lock lock (_priv->multihandle_mutex);
            int prev_running_handles = _priv->running_handles;
            while (curl_multi_perform (_priv->multihandle,
                                       &_priv->running_handles) ==
                   CURLM_CALL_MULTI_PERFORM);
            if (prev_running_handles != _priv->running_handles)
            {
                // TODO: handle finished handle(s)
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
            while (!_priv->exiting)
            {
                FD_ZERO (&read_fds);
                FD_ZERO (&write_fds);
                FD_ZERO (&error_fds);

                // grab information from curl for select()
                {
                    Glib::Mutex::Lock lock (_priv->multihandle_mutex);

                    // wait here until there are handles, or time to exit
                    while (curl_multi_fdset (_priv->multihandle,
                                &read_fds, &write_fds, &error_fds, &nfds),
                            nfds == -1 &&
                            !_priv->exiting)
                        _priv->multihandle_notempty.wait 
                            (_priv->multihandle_mutex);

                    // if it's time to exit, just do so already
                    if (_priv->exiting)
                        throw Glib::Thread::Exit();

                    // then get the timeout
                    curl_multi_timeout (_priv->multihandle, &timeout);
                }

                // convert milliseconds into timeval
                tv.tv_sec = timeout * 1000;
                tv.tv_usec = timeout / 1000;

                // call curl if timeout or fd ready
                select (nfds+1,
                        &read_fds,
                        &write_fds,
                        &error_fds,
                        (timeout<0)?NULL:&tv);

                _priv->curl_ready.emit ();
            }
        }
    };
};
