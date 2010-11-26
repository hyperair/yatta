#include <libxml++/parsers/textreader.h>
#include <fstream>
#include <iostream>
#include <locale>

Glib::ustring replace_newline (Glib::ustring str)
{
    size_t pos;
    while ((pos = str.find ('\n')) != Glib::ustring::npos) {
        str.erase (pos, 1);
        str.insert (pos, "\\n");
    }

    return str;
}

int
main (int argc, char **argv)
{
    std::locale::global (std::locale ("C"));

    if (argc != 4) {
        std::cerr << "Wrong number of arguments. Expected 3, got " << (argc - 1)
                  << std::endl;
        return 1;
    }

    xmlpp::TextReader reader (argv[1]);
    std::ofstream outfile (argv[2]);

    // preamble
    outfile << "#include \"../mainwindow.hh\"" << std::endl
            << "namespace Yatta { namespace UI {" << std::endl
            << "const char *" << argv[3] << " = \"";

    while (reader.read ()) {
        if (reader.get_name () == "ui") {
            outfile << replace_newline (reader.read_outer_xml ()) << std::endl;
        }
    }

    outfile << '"' << std::endl;

    return 0;
}
