/*      main.cc -- part of the Yatta! Download Manager
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

#include <libintl.h>
#include <iostream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "yatta/options.h"
#include "yatta/ui/main.h"

int
main (int argc, char **argv)
{
    // initialize gettext
    bindtextdomain (GETTEXT_PACKAGE, PROGRAMNAME_LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    // get options
    Yatta::Options options;

    // initialize ui kit
    Yatta::UI::Main ui_kit (argc, argv, options);

    // run main loop
    ui_kit.run ();
    return 0;
}
