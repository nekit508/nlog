#include "utils.hpp"
#include "Command.h"

namespace ast {
    struct Error : std::exception {
        peg_parser::SyntaxTree* tree;
        std::string message;

        Error(peg_parser::SyntaxTree* tree, std::string message) {
            this->tree = tree;
            this->message = message;
        }

        void print() const {
            std::cout << "Error at " << tree->begin << "-" << tree->end << ": " << message << "\n";
        }
    };

    /** Cuts segment of code sequence [`start`;`end`] and pastes it after `after` */
    void cutInto(Command* start, Command* end, Command* after) {
        Command* prevStart = start->previous;
        Command* nextEnd = end->next;
        Command* nextAfter = after->next;

        if (prevStart != nullptr)
            prevStart->next = nextEnd;
        if (nextEnd != nullptr)
            nextEnd->previous = prevStart;

        after->next = start;
        nextAfter->previous = end;
    }

    /** Pastes segment of code sequence [`start`;`end`] after `after` */
    void copyInto(Command* start, Command* end, Command* after) {
        Command* nextAfter = after->next;

        after->next = start;
        nextAfter->previous = end;
    }

    std::string translate(Command* start, Command* end = nullptr) {
        std::string string;
        Command* current = start;

        do {
            string = current->get();
            current = current->next;
        } while (current == nullptr || current == end);

        return string;
    }
}