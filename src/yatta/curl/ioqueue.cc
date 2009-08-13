/*      ioqueue.cc -- part of the Yatta! Download Manager
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

#include <giomm.h>
#include <list>
#include <cstring>
#include "ioqueue.h"

namespace Yatta
{
    namespace Curl
    {
        struct IOQueue::Private
        {
            Private (const std::string &dirname,
                     const std::string &filename) :
                dirname (dirname),
                filename (filename),
                queue (),
                handle ()
            {}

            struct Item
            {
                Item (size_t offset, void *data, size_t size) :
                    offset (offset),
                    data (NULL),
                    size (size)
                {
                    this->data = operator new (size);
                    memcpy (this->data, data, size);
                }

                size_t offset;
                void * data;
                size_t size;
            };

            std::string dirname;
            std::string filename;
            std::list<Item> queue;
            Glib::RefPtr<Gio::FileOutputStream> handle;
        };

        IOQueue::IOQueue (const std::string &dirname,
                          const std::string &filename) :
            _priv (new Private (dirname, filename))
        {
            // delay creation of file if filename is empty
            if (filename.empty ())
                return;

            Glib::RefPtr<Gio::File> gfile =
                Gio::File::create_for_path (Glib::build_filename (dirname,
                                                                  filename));

            // create file asynchronously and pass this gfile over for
            // gfile->create_file_finish
            gfile->create_file_async
                (sigc::bind<0> (sigc::mem_fun (*this,
                                               &IOQueue::create_file_finish),
                                gfile));
        }

        IOQueue::~IOQueue ()
        {
        }

        void
        IOQueue::write (size_t offset, void *data, size_t size)
        {
            // perform isn't in action, so we need to start the chain
            bool need_perform = _priv->queue.empty () && !_priv->handle;
            _priv->queue.push_back (Private::Item (offset, data, size));
            if (need_perform)
                perform ();
        }

        void
        IOQueue::perform ()
        {
            // if it's empty, no point doing anything
            if (_priv->queue.empty ())
                return;

            // grab the first item and perform async work
            Private::Item &item = _priv->queue.front ();
            _priv->handle->seek (item.offset, Glib::SEEK_TYPE_SET);
            _priv->handle
                ->write_async (item.data, item.size,
                               sigc::mem_fun (*this, &IOQueue::perform_finish));

            /* when it's done, perform_finish will be called, which will call
             * perform again.
             * don't delete item (queue must be non-empty to show perform going
             * on)
             */
        }

        void
        IOQueue::set_filename (const std::string &filename)
        {
            _priv->filename = filename;
        }

        void
        IOQueue::create_file_finish (Glib::RefPtr<Gio::File> file,
                                     Glib::RefPtr<Gio::AsyncResult> &result)
        {
            _priv->handle = file->create_file_finish (result);

            // if perform was waiting, then start the chain
            if (!_priv->queue.empty ())
                perform ();
        }

        void
        IOQueue::perform_finish (Glib::RefPtr<Gio::AsyncResult> &result)
        {
            // we're handling this item now. delete it from queue
            delete _priv->queue.front ().data;
            _priv->queue.pop_front ();

            // grab result (and check for error), then start the next operation
            try {
                _priv->handle->write_finish (result);
                perform ();
            } catch (Gio::Error &e)
            {
                // TODO: Abort the download somehow
            }
        }
    };
};
