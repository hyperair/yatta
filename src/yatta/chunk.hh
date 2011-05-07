/* chunk.hh -- Abstract class representing a download chunk
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

#ifndef YATTA_CHUNK_H
#define YATTA_CHUNK_H

#include <tr1/memory>
#include <sigc++/sigc++.h>
#include <string>
#include <exception>

namespace Yatta
{
    class ChunkFactory;
    typedef std::tr1::shared_ptr<ChunkFactory> ChunkFactoryPtr;
    typedef std::tr1::shared_ptr<ChunkFactory> ChunkFactoryWPtr;

    class Chunk : protected std::tr1::enable_shared_from_this <Chunk>
    {
    public:
        typedef std::tr1::shared_ptr<Chunk> Ptr;
        typedef std::tr1::weak_ptr<Chunk> WPtr;

        // factory paradigm
        static Ptr create (const std::string &url,
                           size_t offset,
                           size_t size = 0);
        static void register_factory (ChunkFactoryPtr factory);

        virtual void start () = 0;
        virtual void stop () = 0;
        void reset ();

        class Unmergeable : public std::exception
        {
        public:
            Unmergeable () {}
            virtual ~Unmergeable () throw () {}

            virtual const char* what() const throw()
            { return "Could not merge chunks"; }

        private:
            std::string error_msg;
        };

        void merge (Ptr previous_chunk);

        // accessors
        bool running () const;
        virtual bool resumable () const = 0;

        size_t offset () const;
        size_t target_pos () const;
        size_t current_pos () const;
        size_t size () const { return target_pos () - offset (); }
        virtual size_t content_length() const = 0;

        std::string url () const;

        // setters
        void target_pos (size_t) const;

        // signals
        typedef sigc::slot<void, Ptr,
                           void * /* buffer */,
                           size_t /* nbytes */> WriteSlot;
        typedef sigc::slot<void, Ptr> StartedSlot;
        typedef sigc::slot<void, Ptr> StoppedSlot;
        typedef sigc::slot<void, Ptr> ResetSlot;
        typedef sigc::slot<void, Ptr> FinishedSlot;

        sigc::connection connect_signal_write (WriteSlot slot);
        sigc::connection connect_signal_started (StartedSlot slot);
        sigc::connection connect_signal_stopped (StoppedSlot slot);
        sigc::connection connect_signal_finished (FinishedSlot slot);

        virtual ~Chunk () {}

    protected:
        Chunk (const std::string &url,
               size_t offset,
               size_t size);

        // functions called by derivatives to fire signals
        void signal_write (void *buffer, size_t nbytes);
        void signal_started ();
        void signal_stopped ();
        void signal_reset ();
        void signal_finished ();

    private:
        Chunk (const Chunk &);  // no copying

        struct Private;
        friend class Private;

        std::tr1::shared_ptr<Private> _priv;
    };

    typedef Chunk::Ptr ChunkPtr;


    class ChunkFactory
    {
    public:
        typedef ChunkFactoryPtr Ptr;
        typedef ChunkFactoryWPtr WPtr;

        virtual ~ChunkFactory () throw () {}
        virtual ChunkPtr create_chunk () = 0;
    };
}

#endif  // YATTA_CHUNK_H
