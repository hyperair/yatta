/*      ioqueue.h -- part of the Yatta! Download Manager
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

#ifndef YATTA_CURL_IOQUEUE_H
#define YATTA_CURL_IOQUEUE_H

#include <tr1/memory>
#include <string>
#include <glibmm/refptr.h>

// some forward decls
namespace Gio
{
    class File;
    class AyncResult;
};

namespace Yatta
{
    namespace Curl
    {
        class IOQueue
        {
            public:
                IOQueue (const std::string &dirname,
                         const std::string &filename = "");
                void write (size_t offset, void *data, size_t size);
                void perform ();
                void set_filename (const std::string &filename);
                virtual ~IOQueue ();

            protected:
                void create_file_finish
                    (Glib::RefPtr<Gio::File> gfile,
                     Glib::RefPtr<Gio::AsyncResult> &result);
                void perform_finish (Glib::RefPtr<Gio::AsyncResult> &result);

            private:
                struct Private;
                std::tr1::shared_ptr<Private> _priv;
        };
    };
};

#endif // YATTA_CURL_IOQUEUE_H
