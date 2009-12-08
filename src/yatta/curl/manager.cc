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
        typedef std::map<curl_socket_t, Glib::PollFD *> pollmap_t;

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
            chunkmap_t chunkmap; // map of CURL* to Chunk ptrs
            pollmap_t pollmap; // map of curl sockets to PollFD structs
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
                               &Manager::socket_cb);

            set_can_recurse (true);
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
            curl_multi_add_handle (_priv->multihandle, handle);

            _priv->chunkmap.insert (std::make_pair (handle, chunk));
            _priv->running_handles++;

            chunk->signal_started ().emit ();
        }

        void
        Manager::remove_handle (Chunk::Ptr chunk)
        {
            CURL *handle = chunk->get_handle ();
            remove_handle (handle);
        }

        void
        Manager::remove_handle (CURL *handle)
        {
            chunkmap_t::iterator result =
                _priv->chunkmap.find (handle);
            if (result == _priv->chunkmap.end ()) return;
            remove_handle (result);
        }

        void
        Manager::remove_handle (chunkmap_t::iterator iter)
        {
            Chunk::Ptr chunk = iter->second;
            _priv->chunkmap.erase (iter);
            _priv->running_handles = _priv->chunkmap.size ();
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
            }
        }

        int
        Manager::socket_cb (CURL *easy,
                   curl_socket_t s,
                   int action,
                   void *userp,
                   void *socketp)
        {
            Manager *self = static_cast<Manager *> (userp);

            pollmap_t::iterator result = self->_priv->pollmap.find (s);
            Glib::PollFD *pollfd = (result == self->_priv->pollmap.end ()) ?
                result->second : 0;

            switch (action)
            {
            case CURL_POLL_IN:
            case CURL_POLL_OUT:
            case CURL_POLL_INOUT:
                // adding a poll
            {
                Glib::IOCondition flags = Glib::IO_ERR | Glib::IO_HUP;
                if (action & CURL_POLL_IN)
                    flags |= Glib::IO_IN;
                if (action & CURL_POLL_OUT)
                    flags |= Glib::IO_OUT;

                if (! pollfd) {
                }
                else
                    pollfd->set_events (flags);

                self->_priv->pollmap[s] = pollfd;
                self->add_poll (*pollfd);
                break;
            }

            case CURL_POLL_NONE:
            case CURL_POLL_REMOVE:
                // removing a poll
                if (pollfd)
                    self->remove_poll (*pollfd);
                delete pollfd;
                self->_priv->pollmap.erase (result);
            }

            return 0;
        }

        bool
        Manager::prepare (int &timeout)
        {
            long timeout2;
            timeout = curl_multi_timeout (_priv->multihandle, &timeout2);
            timeout = static_cast<int> (timeout2);
            return (timeout == 0);
        }

        bool
        Manager::check ()
        {
            long timeout;
            curl_multi_timeout (_priv->multihandle, &timeout);
            if (timeout == 0) return true;

            for (pollmap_t::iterator i = _priv->pollmap.begin ();
                 i != _priv->pollmap.end ();
                 ++i)
            {
                if (i->second->get_revents ())
                    return true;
            }
        }

        bool
        Manager::dispatch (sigc::slot_base *slot)
        {
            int running_handles = _priv->running_handles;

            for (pollmap_t::iterator i = _priv->pollmap.begin ();
                 i != _priv->pollmap.end ();
                 ++i)
            {
                Glib::IOCondition flags = i->second->get_revents ();

                int evmask = 0;
                if (flags & (Glib::IO_IN | Glib::IO_PRI))
                    evmask |= CURL_CSELECT_IN;
                if (flags & Glib::IO_OUT)
                    evmask |= CURL_CSELECT_OUT;
                if (flags & (Glib::IO_ERR | Glib::IO_HUP))
                    evmask |= CURL_CSELECT_ERR;

                curl_multi_socket_action (_priv->multihandle, i->first,
                                          evmask, &running_handles);
            }

            // tell curl it's time
            long timeout;
            if (curl_multi_timeout (_priv->multihandle, &timeout), timeout == 0)
                curl_multi_socket_action (_priv->multihandle,
                                          CURL_SOCKET_TIMEOUT,
                                          0, &running_handles);

            if (running_handles != _priv->running_handles) {
                // an easy handle (chunk) has finished!
                int msgs;
                CURLMsg *msg = 0;
                while (msg = curl_multi_info_read (_priv->multihandle, &msgs))
                {
                    CURL *handle = msg->easy_handle;
                    CURLcode result = msg->data.result;
                    chunkmap_t::iterator iter =
                        _priv->chunkmap.find (handle);
                    if (iter == _priv->chunkmap.end ())
                        continue; // TODO: report error as a BUG

                    // handle removal of chunk
                    iter->second->signal_finished ().emit (result);
                    remove_handle (iter);
                }
            }
        }
    };
};
