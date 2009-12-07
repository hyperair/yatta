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

#include <glibmm/dispatcher.h>

#include "manager.h"
#include "chunk.h"

namespace Yatta
{
    namespace Curl
    {
        typedef std::map<CURL*, Chunk::Ptr> chunkmap_t;
        struct Manager::Private
        {
            Private () :
                multihandle (NULL),
                sharehandle (NULL),
                running_handles (0)
                {}

            CURLM *multihandle; // only multi handle which will be used
            CURLSH *sharehandle; // to share data between easy handles

            int running_handles; // number of running handles
            chunkmap_t chunkmap;
        };

        Manager::Manager () :
            _priv (new Private())
        {
            // must initialize curl globally first!
            curl_global_init (CURL_GLOBAL_ALL);

            // then initialize the curl handles
            _priv->multihandle = curl_multi_init ();
            _priv->sharehandle = curl_share_init ();

            curl_multi_setopt (_priv->multihandle, CURLMOPT_PIPELINING, 0);
            curl_multi_setopt (_priv->multihandle, CURLMOPT_SOCKETDATA, this);
            curl_multi_setopt (_priv->multihandle, CURLMOPT_SOCKETFUNCTION,
                               &Manager::timeout);
        }

        Manager::~Manager ()
        {
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
            chunk->signal_started ().emit ();
        }

        void
        Manager::remove_handle (Chunk::Ptr chunk)
        {
            CURL *handle = chunk->get_handle ();

            _priv->chunkmap.erase (handle);
            _priv->running_handles--;
            chunk->signal_stopped ().emit ();
        }

        void
        Manager::perform ()
        {
            int prev_running_handles = _priv->running_handles;
            while (curl_multi_perform (_priv->multihandle,
                                       &_priv->running_handles) ==
                   CURLM_CALL_MULTI_PERFORM);

            if (prev_running_handles != _priv->running_handles)
            {
                int msgs;
                CURLMsg *msg = 0;
                while (msg = curl_multi_info_read (_priv->multihandle, &msgs))
                {
                    CURL *handle = msg->easy_handle;
                    CURLcode result = msg->data.result;
                    chunkmap_t::iterator iter = _priv->chunkmap.find (handle);
                    if (iter == _priv->chunkmap.end ())
                        continue; // TODO: report error

                    // handle removal of chunk (TODO: don't repeat code)
                    iter->second->signal_finished ().emit (result);
                    _priv->chunkmap.erase (iter);
                    iter->second->signal_stopped ().emit ();
                }
            }
        }

        static int
        socket_cb (CURL *easy,
                   curl_socket_t, s,
                   int action,
                   void *userp,
                   void *socketp)
        {
        }
    };
};
