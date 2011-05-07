/*      chunk.cc -- part of the Yatta! Download Manager
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

#include <sstream>
#include <limits>
#include <algorithm>

#include <sigc++/signal.h>

#include "chunk.hh"
#include "../download.hh"
#include "manager.hh"


using Yatta::Curl::Chunk;
typedef ::Yatta::Chunk IChunk;

// first the private class implementation
struct Chunk::Private
{
    // constructor
    Private () :
        handle (NULL),
        headers (NULL),
        in_curl_callback (false),
        stop_queued (false)
    {}

    // data
    CURL       *handle;
    curl_slist *headers;
    bool        in_curl_callback;
    bool        stop_queued;

    // write function
    static size_t on_curl_write (void *data, size_t size,
                                 size_t nmemb, void *obj);
};

// Exception safe method of marking in_curl_callback as true
namespace {
    class BoolLock
    {
    public:
        BoolLock (bool &flag) :
            flag (flag),
            owns_flag (!flag)
        {}

        ~BoolLock ()
        {
            if (owns_flag)
                flag = false;
        }

    private:
        bool &flag;
        bool owns_flag;
    };
}

// constructor
Chunk::Chunk (const std::string &url, size_t offset, size_t total) :
    IChunk (url, offset, total),
    _priv (new Private)
{}

// destructor
Chunk::~Chunk ()
{
    // we're screwed if this happens
    g_assert (!_priv->in_curl_callback);

    stop ();
}

// member functions
void Chunk::start ()
{
    if (running ())
        return;

    // create chunk
    _priv->handle = curl_easy_init ();
    curl_easy_setopt (handle (), CURLOPT_URL,
                      url ().c_str ());
    curl_easy_setopt (handle (), CURLOPT_RESUME_FROM, current_pos ());

    // hack to tell libcurl to pass the range anyway to induce a 206
    if (current_pos () == 0) {
        std::ostringstream ss;
        ss << "Range: bytes=" << current_pos () << "-";
        _priv->headers = curl_slist_append (_priv->headers, ss.str ().c_str ());

        curl_easy_setopt (handle (), CURLOPT_HTTPHEADER, _priv->headers);
    }

    // make curl pass this into the callbacks
    curl_easy_setopt (handle (), CURLOPT_WRITEDATA, this);

    // bind the callbacks
    curl_easy_setopt (handle (), CURLOPT_WRITEFUNCTION,
                      &Private::on_curl_write);


    Manager::get ()->add_handle (this);

    signal_started ();
}

void Chunk::stop ()
{
    if (!running ()) return;

    // we're in the middle of a callback, so set cancelled, and wait for
    // curl to get rid of us. then stop_finished will be called which
    // will call us again
    if (_priv->in_curl_callback) {
        _priv->stop_queued = true;
        return;
    }

    Manager::get ()->remove_handle (this);
    curl_easy_cleanup (_priv->handle);
    curl_slist_free_all (_priv->headers);

    _priv->handle = NULL;
    _priv->headers = NULL;
    _priv->stop_queued = false;

    signal_stopped ();
}

bool Chunk::resumable () const
{
    long code;
    curl_easy_getinfo (_priv->handle, CURLINFO_RESPONSE_CODE, &code);
    return (code == 206);
}

size_t Chunk::content_length() const
{
    long code;
    curl_easy_getinfo (_priv->handle, CURLINFO_RESPONSE_CODE, &code);
    return (code == 206);
}

void Chunk::stop_finished (CURLcode)
{
    long code;
    curl_easy_getinfo (handle (), CURLINFO_RESPONSE_CODE, &code);
    stop ();

    signal_finished ();
}

// other accessors
CURL *Chunk::handle ()
{
    return _priv->handle;
}

const CURL *Chunk::handle () const
{
    return _priv->handle;
}

// static CURL callbacks
size_t Chunk::Private::on_curl_write (void *data, size_t size,
                                      size_t nmemb, void *obj)
{
    Chunk *self = reinterpret_cast<Chunk*> (obj);

    // we're stopped, so don't do anything
    if (self->_priv->stop_queued ||
        self->target_pos () >= self->current_pos ())
        return 0;

    size_t bytes_handled = std::min (self->target_pos () - self->current_pos (),
                                     size * nmemb);

    BoolLock callback_lock = BoolLock (self->_priv->in_curl_callback);
    self->signal_write (data, bytes_handled);

    return bytes_handled;
}
