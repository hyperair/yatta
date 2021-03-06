/*      main.h -- part of the Yatta! Download Manager
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

#ifndef YATTA_UI_MAIN_H
#define YATTA_UI_MAIN_H

#include <tr1/memory>

#include <gtkmm/main.h>

#include "mainwindow.hh"
#include "aboutdialog.hh"

namespace Yatta
{
    // forward decl
    class Options;

    namespace UI
    {
        /**
         * @brief: Main UI class, where everything begins
         */
        class Main :
            public Gtk::Main
        {
            public:
                /**
                 * @brief: Constructor
                 * @param argc Number of arguments
                 * @param argv Array of arguments
                 */
                Main (int &argc, char **&argv, Options &options);

                /**
                 * @description: Run the main loop of the UI
                 */
                void run ();

                /**
                 * @description: Show the about dialog
                 */
                void show_aboutdlg ();

                /**
                 * @description: Get Yatta::Options object
                 * @return: reference to the Options object
                 */
                Options &options ();

                /**
                 * @brief: Desctructor
                 */
                virtual ~Main ();

            private:
                struct Priv;
                std::tr1::shared_ptr<Priv> _priv;
        };
    }
}

#endif // YATTA_UI_MAIN_H
