/*      aboutwindow.h -- part of the Yatta! Download Manager
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

#ifndef YATTA_UI_ABOUTWINDOW_H
#define YATTA_UI_ABOUTWINDOW_H

#include <gtkmm/aboutdialog.h>

namespace Yatta
{
    namespace UI
    {
        class About :
            public Gtk::AboutDialog
        {
            public:
                About ();
                virtual void on_response (int response_id);
                virtual ~About ();
        };
    }
}

#endif // YATTA_UI_ABOUTWINDOW_H
