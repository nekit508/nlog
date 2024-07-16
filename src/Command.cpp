#include "../include/Command.h"
#include "../include/Scope.h"
#include <iostream>
#include "../include/ast.h"

std::string ast::Command::get() {
    std::string string = this->string;

    ast::replaceVars(string, scope);

    return string + "\n";
}

void ast::Command::replaceVars() {
    ast::replaceVars(string, scope);
}

ast::Command *ast::Command::copy() {
    return new Command;
}

std::string ast::StartCommand::get()  {
    return "";
}

void ast::StartCommand::replaceVars() {
}

ast::Command *ast::StartCommand::copy() {
    return new StartCommand;
}
