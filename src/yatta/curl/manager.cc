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

namespace Yatta
{
    namespace Curl
    {
        Manager::Manager () :
            m_multihandle (NULL),
            m_sharehandle (NULL),
            m_running_handles (0)
        {
            curl_global_init (CURL_GLOBAL_ALL);
            m_multihandle = curl_multi_init ();
            m_sharehandle = curl_share_init ();
        }

        void
        Manager::add_handle (CURL *handle)
        {
            curl_easy_setopt (handle, CURLOPT_SHARE, m_sharehandle);
            curl_multi_add_handle (m_multihandle, handle);
            m_running_handles++;
        }

        void
        Manager::remove_handle (CURL *handle)
        {
            curl_multi_remove_handle (m_multihandle, handle);
            m_running_handles--;
        }

        void
        Manager::perform ()
        {
            int prev_running_handles = m_running_handles;
            while (curl_multi_perform (m_multihandle,
                                       &m_running_handles) ==
                   CURLM_CALL_MULTI_PERFORM)
            {
                if (prev_running_handles != m_running_handles)
                {
                    // TODO: handle finished handle(s)
                }
            }
        }

        Manager::~Manager ()
        {
            curl_global_cleanup ();
        }
    };
};
