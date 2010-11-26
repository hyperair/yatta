#include <libxml++/parsers/textreader.h>
#include <fstream>
#include <iostream>
#include <locale>

Glib::ustring escape_string (Glib::ustring str)
{
    size_t pos = 0;
    while ((pos = str.find ('\n', pos)) != Glib::ustring::npos) {
        str.erase (pos, 1);
        str.insert (pos, "\\n");
        pos += 2;
    }

    pos = 0;
    while ((pos = str.find ('"', pos+2)) != Glib::ustring::npos) {
        str.erase (pos, 1);
        str.insert (pos, "\\\"");
        pos += 2;
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
            << "const char *" << argv[3] << " = \"";

    while (reader.read ()) {
        if (reader.get_name () == "ui") {
            outfile << escape_string (reader.read_outer_xml ());
            break;
        }
    }

    outfile << "\";" << std::endl;

    return 0;
}
