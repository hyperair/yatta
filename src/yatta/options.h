/*      options.h -- part of the Yatta! Download Manager
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

#ifndef YATTA_OPTIONS_H
#define YATTA_OPTIONS_H

#include <glibmm/optioncontext.h>

namespace Yatta
{
    class Options : 
        public Glib::OptionContext
    {
        public:
            Options ();

            /**
             * @description: Get the datadir, containing data files
             * @return: Glib::ustring containing data dir
             */
            const Glib::ustring &get_datadir ();
            virtual ~Options ();
        private:
            Glib::ustring m_datadir; // datadir (defaults to DATADIR)
            Glib::OptionGroup m_maingroup; // main options group
    };
};

#endif // YATTA_OPTIONS_H
