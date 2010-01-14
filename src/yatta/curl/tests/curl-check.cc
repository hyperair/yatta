#include <glibmm.h>
#include <giomm.h>
#include "../manager.hh"
#include "../download.hh"

struct bla
{
    Glib::RefPtr<Glib::MainLoop> main_loop;
    bla (Glib::RefPtr<Glib::MainLoop> main_loop) :
        main_loop (main_loop) {}
    void operator ()()
    {
        g_debug ("Download finished, shutting down loop");
        main_loop->quit ();
    }
};
int main ()
{
    Glib::init ();
    Gio::init ();
    Glib::RefPtr<Yatta::Curl::Manager> mgr = Yatta::Curl::Manager::get ();
    mgr->attach ();

    Yatta::Curl::Download dl ("http://sg.releases.ubuntu.com/9.10/ubuntu-9.10-desktop-amd64.iso", "/tmp", "ubuntu.iso");
    // Yatta::Curl::Download dl ("http://localhost/test.file", "/tmp", "test.file");
    // Yatta::Curl::Download dl ("http://www.ubuntu.com", "/tmp", "testing");
    dl.start ();
    bla func (Glib::MainLoop::create ());
    dl.connect_signal_finished (sigc::slot<void> (func));
    func.main_loop->run ();
}
