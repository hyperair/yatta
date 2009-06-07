/*      mainwindow.h -- part of the Yatta! Download Manager
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

#ifndef YATTA_UI_MAINWINDOW_H
#define YATTA_UI_MAINWINDOW_H

#include <gtkmm/window.h>
#include <gtkmm/uimanager.h>

namespace Yatta
{
    namespace UI
    {
        // forward declaration
        class Main;

        class MainWindow :
            public Gtk::Window
        {
            public:
                MainWindow (Main &ui_main);

            protected:
                /**
                 * @description: Constructs widgets in the window
                 */
                virtual void construct_widgets ();

                /**
                 * @description: Prepares Actions for the menu and toolbar
                 *               and adds them into m_ref_uimgr
                 */
                virtual void prepare_actions ();

                /**
                 * @see Gtk::Widget::hide()
                 */
                virtual void on_hide ();

            private:
                Glib::RefPtr<Gtk::UIManager> m_ref_uimgr;
                Main &m_ui_main; // main UI object
        };
    };
};

#endif // YATTA_UI_MAINWINDOW_H
