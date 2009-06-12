/*      manager.h -- part of the Yatta! Download Manager
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

#ifndef YATTA_CURL_MANAGER_H
#define YATTA_CURL_MANAGER_H

#include <curl/multi.h>
#include <curl/curl.h>

namespace Yatta
{
    namespace Curl
    {
        class Manager
        {
            public:
                Manager ();
                void add_handle (CURL *handle);
                void remove_handle (CURL *handle);
                virtual ~Manager ();
            private:
                CURLM *m_multihandle; // only multi handle which will be used
                CURLSH *m_sharehandle; // to share data between easy handles
                int m_running_handles; // number of running handles
        };
    };
};
#endif // YATTA_CURL_MANAGER_H
