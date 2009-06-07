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
#include <gtkmm/uimanager.h>
#include <gtkmm/stock.h>

#include "mainwindow.h"
#include "main.h"
#include "../options.h"

namespace Yatta
{
    namespace UI
    {
        MainWindow::MainWindow (Main &ui_main) :
            Gtk::Window (),
            m_ref_uimgr (Gtk::UIManager::create ()),
            m_ui_main (ui_main)
        {
            // set defaults for the window
            set_title (_("Yatta Download Manager"));
            set_default_size (640, 480);

            // prepare uimgr
            try {
                m_ref_uimgr->add_ui_from_file 
                    (m_ui_main.get_options ().get_datadir () +
                     "/main_menu.ui");
                m_ref_uimgr->add_ui_from_file
                    (m_ui_main.get_options ().get_datadir () +
                     "/main_tb.ui");
            } catch (Glib::FileError &e) {
                // TODO: log error
                exit (1);
            }
            prepare_actions ();

            // prepare widgets
            construct_widgets ();
        }

        void
        MainWindow::construct_widgets ()
        {
            // add a vbox to the window first
            Gtk::VBox *main_vbox = manage (new Gtk::VBox);
            add (*main_vbox);

            // get menu and toolbar
            Gtk::Widget *menu, *toolbar;
            menu = m_ref_uimgr->get_widget ("/mm_bar");
            toolbar = m_ref_uimgr->get_widget ("/mt_bar");

            if ( !menu || !toolbar ) {
                // TODO: log error
                exit (1);
            }

            // add menu and toolbar into the main vbox
            main_vbox->pack_start (*menu);
            main_vbox->pack_start (*toolbar);

            // show the main vbox!
            main_vbox->show_all ();

            // bind them shortcuts!
            add_accel_group (m_ref_uimgr->get_accel_group ());
        }

        void
        MainWindow::prepare_actions ()
        {
            Glib::RefPtr<Gtk::ActionGroup> actions =
                Gtk::ActionGroup::create ();

            // add the menu bar items
            actions->add (Gtk::Action::create ("mm_file", _("_File")));
            actions->add (Gtk::Action::create ("mm_edit", _("_Edit")));
            actions->add (Gtk::Action::create ("mm_download", _("_Download")));
            actions->add (Gtk::Action::create ("mm_help", _("_Help")));

            // add common actions
            // in file menu
            actions->add (Gtk::Action::create
                    ("ShowAddURIDlg",
                     Gtk::Stock::NEW,
                     _("Add URI"),
                     _("Add downloads manually")));
            actions->add (Gtk::Action::create 
                    ("Quit",
                     Gtk::Stock::QUIT),
                     sigc::mem_fun (*this, &MainWindow::hide));

            // in edit menu
            actions->add (Gtk::Action::create 
                    ("ShowPrefsDlg",
                     Gtk::Stock::PREFERENCES));

            // in download menu
            actions->add (Gtk::Action::create 
                    ("OpenFolder",
                     Gtk::Stock::OPEN));
            actions->add (Gtk::Action::create 
                    ("ResumeDownload",
                     Gtk::Stock::MEDIA_PLAY));
            actions->add (Gtk::Action::create 
                    ("PauseDownload",
                     Gtk::Stock::MEDIA_PAUSE));
            actions->add (Gtk::Action::create 
                    ("CancelDownload",
                     Gtk::Stock::STOP));
            actions->add (Gtk::Action::create
                    ("AddChunk",
                     Gtk::Stock::ADD,
                     _("Add Chunk"),
                     _("Add chunk to currently running download")));
            actions->add (Gtk::Action::create
                    ("RMChunk",
                     Gtk::Stock::REMOVE,
                     _("Remove Chunk"),
                     _("Reduce number of currently running chunks")));
            actions->add (Gtk::Action::create
                    ("MoveTop",
                     Gtk::Stock::GOTO_TOP,
                     _("Move to top"),
                     _("Move this download to the top of the queue")));
            actions->add (Gtk::Action::create
                    ("MoveUp",
                     Gtk::Stock::GO_UP,
                     _("Move up"),
                     _("Move this download up in the queue")));
            actions->add (Gtk::Action::create
                    ("MoveDown",
                     Gtk::Stock::GO_DOWN,
                     _("Move down"),
                     _("Move this download down in the queue")));
            actions->add (Gtk::Action::create
                    ("MoveBottom",
                     Gtk::Stock::GOTO_BOTTOM,
                     _("Move to bottom"),
                     _("Move this download to the bottom of the queue")));

            // in help menu
            actions->add (Gtk::Action::create ("ShowAbtDlg", 
                                               Gtk::Stock::ABOUT));

            // add the actions into the ui mgr
            m_ref_uimgr->insert_action_group (actions);
        }

        void
        MainWindow::on_hide ()
        {
            Gtk::Main::quit ();
        }
    };
};
