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

#include "main.hh"
#include "mainwindow.hh"
#include "aboutdialog.hh"
#include "../options.hh"

namespace Yatta
{
    namespace UI
    {
        struct Main::Priv
        {
            Priv (Main &self, Options &options) :
                mainwin (self),
                aboutdlg (),
                options (options) {}
            MainWindow mainwin; // main window
            About      aboutdlg; // about dialog
            Options   &options;

        };

        Main::Main (int &argc, char **&argv, Options &options) :
            Gtk::Main (argc, argv, options),
            _priv (new Priv (*this, options))
        {
        }

        void Main::run ()
        {
            _priv->mainwin.show ();
            Gtk::Main::run ();
        }

        void Main::show_aboutdlg ()
        {
            _priv->aboutdlg.show ();
        }

        Options &Main::options ()
        {
            return _priv->options;
        }

        Main::~Main ()
        {
        }
    }
}
