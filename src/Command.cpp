#include "../include/Command.h"
#include "../include/Scope.h"
#include <iostream>
#include "../include/ast.h"

void ast::Command::get(std::string &output) {
    std::string string = this->string;

    ast::replaceVars(string, scope);

    output += string + "\n";
}

void ast::Command::replaceVars() {
    ast::replaceVars(string, scope);
}

ast::Command *ast::Command::copy() {
    return new Command;
}

ast::Command::~Command() {
    if (previous != nullptr) {
        previous->next = nullptr;
        delete previous;
    }
    if (next != nullptr) {
        next->previous = nullptr;
        delete next;
    }
}


void ast::StartCommand::get(std::string &output)  {
}

void ast::StartCommand::replaceVars() {
}

ast::Command *ast::StartCommand::copy() {
    return new StartCommand;
}


void ast::AnchorCommand::get(std::string &output) {
    output += "%a" + name + "%";
}

void ast::AnchorCommand::replaceVars() {

}

ast::Command *ast::AnchorCommand::copy() {
    return new AnchorCommand;
}
