/*      options.cc -- part of the Yatta! Download Manager
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "options.h"

namespace Yatta
{
    Options::Options () : 
        Glib::OptionContext (),
        m_datadir (DATADIR),
        m_maingroup ("main", "Main options")
    {
        {
            // prepare datadir OptionEntry
            Glib::OptionEntry datadir_entry;
            datadir_entry.set_long_name ("datadir");
            datadir_entry.set_description 
                (Glib::ustring (_("Override data directory") )
                 + " [" + DATADIR + "]");
            m_maingroup.add_entry (datadir_entry, m_datadir);
        }

        set_main_group (m_maingroup);
    }

    const Glib::ustring &
    Options::get_datadir ()
    {
        return m_datadir;
    }

    Options::~Options ()
    {
    }
}
