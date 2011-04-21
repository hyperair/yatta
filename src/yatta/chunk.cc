/* chunk.cc -- Abstract class representing a download chunk
 * Copyright © 2011, Chow Loong Jin <hyperair@ubuntu.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "chunk.hh"

using Yatta::Chunk;

struct Chunk::Private
{
    sigc::signal<void, Ptr,
               char * /* buffer */,
               size_t /* nbytes */> signal_write;
    sigc::signal<void, Ptr> signal_started;
    sigc::signal<void, Ptr> signal_stopped;
    sigc::signal<void, Ptr> signal_finished;
};

Chunk::Chunk () :
    _priv (new Private)
{}

sigc::connection
Chunk::connect_signal_write (WriteSlot slot)
{
    return _priv->signal_write.connect (slot);
}

sigc::connection
Chunk::connect_signal_started (StartedSlot slot)
{
    return _priv->signal_stopped.connect (slot);
}

sigc::connection
Chunk::connect_signal_stopped (StoppedSlot slot)
{
    return _priv->signal_stopped.connect (slot);
}

sigc::connection
Chunk::connect_signal_finished (FinishedSlot slot)
{
    return _priv->signal_finished.connect (slot);
}


void
Chunk::signal_write (char *buffer, size_t nbytes)
{
    _priv->signal_write (shared_from_this (), buffer, nbytes);
}

void
Chunk::signal_started ()
{
    _priv->signal_started (shared_from_this ());
}

void
Chunk::signal_stopped ()
{
    _priv->signal_stopped (shared_from_this ());
}

void
Chunk::signal_finished ()
{
    _priv->signal_finished (shared_from_this ());
}
