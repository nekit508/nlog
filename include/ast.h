#include "utils.h"
#include "Command.h"
#include <iostream>

namespace ast {
    struct Error : std::exception {
        peg_parser::SyntaxTree* tree;
        std::string message;

        Error(peg_parser::SyntaxTree* tree, std::string message);

        void print() const;
    };

    /** Pastes segment of code sequence [`start`;`end`] after `after` */
    void copyInto(Command* start, Command* end, Command* after);

    std::string translate(Command* start, Command* end = nullptr);
}