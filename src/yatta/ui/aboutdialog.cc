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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "aboutdialog.h"

namespace Yatta
{
    namespace UI
    {
        About::About () :
            Gtk::AboutDialog ()
        {
            const char *authors[] = {
                "Chow Loong Jin <hyperair@gmail.com>",
                NULL
            };

            set_program_name ("Yatta!");
            set_version (VERSION);
            set_copyright ("Copyright © 2009, "
                           "Chow Loong Jin <hyperair@gmail.com>");
            set_authors (authors);
        }

        void
        About::on_response (int response_id)
        {
            hide ();
        }

        About::~About ()
        {
        }
    };
};
