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
    struct Options::Priv
    {
        Priv () :
            datadir (DATADIR),
            maingroup ("main", "Main options") {}
        Glib::ustring     datadir;
        Glib::OptionGroup maingroup;
    };

    Options::Options () :
        Glib::OptionContext (),
        _priv (new Priv ())
    {
        {
            // prepare datadir OptionEntry
            Glib::OptionEntry datadir_entry;
            datadir_entry.set_long_name ("datadir");
            datadir_entry.set_description
                (Glib::ustring (_("Override data directory") )
                 + " [" DATADIR "]");
            _priv->maingroup.add_entry (datadir_entry, _priv->datadir);
        }

        set_main_group (_priv->maingroup);
    }

    const Glib::ustring &Options::datadir ()
    {
        return _priv->datadir;
    }

    Options::~Options ()
    {
    }
}
