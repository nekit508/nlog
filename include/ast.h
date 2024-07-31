#include "utils.h"
#include "Command.h"
#include <iostream>

namespace ast {
    struct Error : std::exception {
        std::shared_ptr<peg_parser::SyntaxTree> tree;
        std::string message;

        Error(std::shared_ptr<peg_parser::SyntaxTree> tree, std::string message);

        void print() const;
    };

    /** Pastes segment of code sequence [`start`;`end`] after `after` */
    void copyInto(Command* start, Command* end, Command* after);

    void replaceVars(std::string &source, Scope *context);

    void translate(Command *start, Command *end, std::string &output);
}