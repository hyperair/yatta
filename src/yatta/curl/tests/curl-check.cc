#include <glibmm.h>
#include <giomm.h>
#include "../manager.h"
#include "../download.h"

int main ()
{
    Glib::init ();
    Gio::init ();
    Glib::RefPtr<Yatta::Curl::Manager> mgr = Yatta::Curl::Manager::get ();
    mgr->attach ();

    Yatta::Curl::Download dl ("http://www.ubuntu.com/", "/tmp", "testing!!!");
    dl.start ();
    Glib::MainLoop::create ()->run ();
}
