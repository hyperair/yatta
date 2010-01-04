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

    Yatta::Curl::Download dl ("http://tw.releases.ubuntu.com/9.10/ubuntu-9.10-desktop-amd64.iso", "/tmp", "ubuntu.iso");
    // Yatta::Curl::Download dl ("http://localhost/test.file", "/tmp", "test.file");
    dl.start ();
    Glib::MainLoop::create ()->run ();
}
