#include "../include/ast.h"
#include "../include/Scope.h"
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

void ast::replaceVars(std::string &source, Scope *context) {
    size_t begin;
    while ((begin = source.find_first_of("%v")) != std::string::npos) {
        size_t end = source.find_first_of('%', begin + 2);
        if (end != std::string::npos) {
            std::string varName = source.substr(begin + 2, end - begin - 2);
            nlogCompilerUtils::replaceAll(source, "%v" + varName + "%", context->findVar(varName)->value);
        } else {
            break;
        }
    }
}
