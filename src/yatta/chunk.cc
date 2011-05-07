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

#include <limits>
#include "chunk.hh"

using namespace Yatta;
using namespace std;

struct Chunk::Private
{
    sigc::signal<void, Ptr,
                 void * /* buffer */,
                 size_t /* nbytes */> signal_write;
    sigc::signal<void, Ptr> signal_started;
    sigc::signal<void, Ptr> signal_stopped;
    sigc::signal<void, Ptr> signal_reset;
    sigc::signal<void, Ptr> signal_finished;

    // some states
    std::string url;
    bool running;
    size_t offset;
    size_t target_pos;
    size_t current_pos;

    Private (const std::string &url,
             size_t offset,
             size_t size) :
        url (url),
        running (false),
        offset (offset),
        target_pos (offset + size),
        current_pos (offset)
    {
        if (target_pos <= current_pos)
            target_pos = numeric_limits<size_t>::max();
    }
};

Chunk::Chunk (const std::string &url,
              size_t offset,
              size_t size) :
    _priv (new Private (url, offset, size))
{}

void
Chunk::merge (Ptr previous_chunk)
{
    if (previous_chunk->current_pos () < offset ())
        throw Unmergeable ();

    bool need_start = previous_chunk->running () && !running ();

    previous_chunk->stop ();
    _priv->offset = previous_chunk->offset ();

    if (need_start)
        start ();
}

bool
Chunk::running () const
{
    return _priv->running;
}

size_t
Chunk::offset () const
{
    return _priv->offset;
}

size_t
Chunk::target_pos () const
{
    return _priv->target_pos;
}

size_t
Chunk::current_pos () const
{
    return _priv->current_pos;
}

std::string
Chunk::url () const
{
    return _priv->url;
}

sigc::connection
Chunk::connect_signal_write (WriteSlot slot)
{
    return _priv->signal_write.connect (slot);
}

sigc::connection
Chunk::connect_signal_started (StartedSlot slot)
{
    return _priv->signal_started.connect (slot);
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
Chunk::signal_write (void *buffer, size_t nbytes)
{
    _priv->signal_write (shared_from_this (), buffer, nbytes);
    _priv->current_pos += nbytes;
    if (_priv->current_pos > _priv->target_pos)
        stop ();
}

void
Chunk::signal_started ()
{
    _priv->running = true;
    _priv->signal_started (shared_from_this ());
}

void
Chunk::signal_stopped ()
{
    _priv->running = false;
    _priv->signal_stopped (shared_from_this ());
}

void
Chunk::signal_reset ()
{
    _priv->current_pos = _priv->offset;
    _priv->signal_reset (shared_from_this ());
}

void
Chunk::signal_finished ()
{
    _priv->signal_finished (shared_from_this ());
}
