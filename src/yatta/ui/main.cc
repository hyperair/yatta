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

#include <gtkmm/main.h>

#include "main.h"
#include "../options.h"

namespace Yatta
{
    namespace UI
    {
        Main::Main (int &argc, char **&argv, Options &options) :
            Gtk::Main (argc, argv, options),
            m_options (options),
            m_mainwin (*this)
        {
        }

        void
        Main::run ()
        {
            m_mainwin.show ();
            Gtk::Main::run ();
        }

        Options &
        Main::get_options ()
        {
            return m_options;
        }

        Main::~Main ()
        {
        }
    };
};
