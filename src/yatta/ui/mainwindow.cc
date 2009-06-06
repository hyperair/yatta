/*      mainwindow.cc -- part of the Yatta! Download Manager
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

#include <glibmm/i18n.h>
#include <gtkmm/main.h>
#include <gtkmm/box.h>

#include "mainwindow.h"

namespace Yatta
{
    namespace UI
    {
        MainWindow::MainWindow () :
            Gtk::Window ()
        {
            // set defaults for the window
            set_title (_("Yatta Download Manager"));
            set_default_size (640, 480);

            // begin constructing the window
            // add a vbox to the window first
            Gtk::VBox *main_vbox = manage (new Gtk::VBox);
            add (*main_vbox);
        }

        void
        MainWindow::on_hide ()
        {
            Gtk::Main::quit();
        }
    };
};
