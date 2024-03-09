#include <iostream>
#include <sstream>
#include "OptParserExtended.h"

using std::cout;
using std::endl;

namespace mlib {

    OptParserExtended::OptParserExtended (std::initializer_list<const char*> list)
    : OptParser{list}
    {}

    std::string OptParserExtended::getoptStringRequired(char option) const {
        std::string optArg;
        if (!getopt(option, optArg)) {
            cout << "Option -" << option << " is missing\n\n";
            cout << "Usage:" << endl;
            cout << synopsis() << endl;
            cout << "Where:" << endl
                 << description() << endl;
            exit(1);
        }
        return optArg;
    }

    int OptParserExtended::getoptIntRequired(char option) const {
        std::string optArg;
        if (!getopt(option, optArg)) {
            cout << "Option -" << option << " is missing\n\n";
            cout << "Usage:" << endl;
            cout << synopsis() << endl;
            cout << "Where:" << endl
                 << description() << endl;
            exit(1);
        }
        std::istringstream iss(optArg);
        int val;
        iss >> val;
        if (!iss) {
            cout << "Option -" << option << " has incorrect parameter (it is not an integer)\n\n";
            cout << "Usage:" << endl;
            cout << synopsis() << endl;
            cout << "Where:" << endl
                 << description() << endl;
            exit(1);
        }
        return val;
    }
}
