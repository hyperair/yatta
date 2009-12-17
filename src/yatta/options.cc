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
            maingroup ("main", "Main options") {}
        Glib::OptionGroup maingroup;
    };

    Options::Options () :
        Glib::OptionContext (),
        _priv (new Priv ())
    {
        set_main_group (_priv->maingroup);
    }

    Options::~Options ()
    {
    }
}
