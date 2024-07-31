#include "../include/ast.h"
#include "../include/Scope.h"
#include "iostream"

ast::Error::Error(std::shared_ptr<peg_parser::SyntaxTree> tree, std::string message) {
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


void ast::replaceVars(std::string &source, Scope *context) {
    while (true) {
        std::string::size_type begin = source.find_first_of("%v");
        if (begin == std::string::npos)
            break;

        std::string::size_type end = source.find_first_of('%', begin+1);
        if (end == std::string::npos)
            break;

        Var* var = context->findVar(source.substr(begin+2, end - begin - 2));

        source.replace(begin, end - begin + 1, var->value);
    }
}
