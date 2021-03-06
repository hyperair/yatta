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

#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <gtkmm/accelmap.h>

#include "mainwindow.hh"
#include "main.hh"
#include "../options.hh"

namespace Yatta
{
    namespace UI
    {
        struct MainWindow::Priv
        {
            Priv (Main &ui_main) :
                uimgr (Gtk::UIManager::create ()),
                statusbar (),
                notebook (),
                ui_main (ui_main) {}
            Glib::RefPtr<Gtk::UIManager> uimgr;
            Gtk::Statusbar statusbar;
            Gtk::Notebook  notebook;
            Main &ui_main; // main UI object
        };

        MainWindow::MainWindow (Main &ui_main) :
            Gtk::Window (),
            _priv (new Priv (ui_main))
        {
            // set defaults for the window
            set_title (_("Yatta! Download Manager"));
            set_default_size (640, 480);

            // prepare widgets and show when idle
            construct_widgets ();

            // TODO: use build_filename and don't repeat code
            Gtk::AccelMap::load (Glib::get_user_config_dir () +
                                 "/yatta/accels.map");
        }

        MainWindow::~MainWindow ()
        {
            // TODO: see note in constructor
            Gtk::AccelMap::save (Glib::get_user_config_dir () +
                                 "/yatta/accels.map");
        }

        void MainWindow::construct_widgets ()
        {
            // add a vbox to the window first
            Gtk::VBox *main_vbox = manage (new Gtk::VBox);
            add (*main_vbox);

            // get menu and toolbar
            Gtk::Widget *menu, *toolbar;
            prepare_uimgr ();
            menu = _priv->uimgr->get_widget ("/mm_bar");
            toolbar = _priv->uimgr->get_widget ("/mt_bar");

            if (!menu || !toolbar) {
                // TODO: log error
                exit (1);
            }

            // prepare VPaned
            Gtk::VPaned *vpaned = manage (new Gtk::VPaned);
            Gtk::HPaned *hpaned = manage (new Gtk::HPaned);
            vpaned->pack1 (*hpaned, Gtk::EXPAND | Gtk::FILL);
            vpaned->pack2 (_priv->notebook, Gtk::EXPAND | Gtk::FILL);

            // add widgets into vbox
            main_vbox->pack_start (*menu, Gtk::PACK_SHRINK);
            main_vbox->pack_start (*toolbar, Gtk::PACK_SHRINK);
            main_vbox->pack_start (*vpaned, Gtk::PACK_EXPAND_WIDGET);
            main_vbox->pack_start (_priv->statusbar, Gtk::PACK_SHRINK);

            // show the main vbox!
            main_vbox->show_all ();

            // bind them shortcuts!
            add_accel_group (_priv->uimgr->get_accel_group ());
        }

        void MainWindow::prepare_uimgr ()
        {
            // load .ui data
            _priv->uimgr->add_ui_from_string (main_menu_uidata);
            _priv->uimgr->add_ui_from_string (main_tb_uidata);

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
                                               Gtk::Stock::ABOUT),
                          sigc::mem_fun (_priv->ui_main, &Main::show_aboutdlg));

            // add the actions into the ui mgr
            _priv->uimgr->insert_action_group (actions);
        }

        void MainWindow::on_hide ()
        {
            Gtk::Main::quit ();
        }
    }
}
