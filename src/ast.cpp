#include "../include/ast.h"
#include "iostream"

ast::Error::Error(peg_parser::SyntaxTree *tree, std::string message) {
    this->tree = tree;
    this->message = message;
}

void ast::Error::print() const {
    if (tree == nullptr)
        std::cout << "Error: " << message << "\n";
    else
        std::cout << "Error at " << tree->begin << "-" << tree->end << ": " << message << "\n";
}

void ast::copyInto(ast::Command *start, ast::Command *end, ast::Command *after) {
    Command *nextAfter = after->next;
    Command *prev = after, *cur = start;
    Command *n;
    do {
        n = cur->copy();

        n->scope = cur->scope;
        n->string = cur->string;
        n->replaceVars();

        prev->next = n;
        n->previous = prev;

        prev = n;
        cur = cur->next;
    } while (cur != end->next);

    if (nextAfter != nullptr) {
        nextAfter->previous = n;
        n->next = nextAfter;
    }
}

std::string ast::translate(ast::Command *start, ast::Command *end) {
    std::string string;
    Command* current = start;

    do {
        string += current->get();
        current = current->next;
    } while (current != nullptr || current != end);

    return string;
}
