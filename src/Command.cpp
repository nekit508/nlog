#include "../include/Command.h"
#include "../include/Scope.h"
#include <iostream>

std::string ast::Command::get() {
    std::string string = this->string;
    std::cout << "Base: " << string;
    size_t begin;
    while ((begin = string.find_first_of("%v")) != std::string::npos) {
        size_t end = string.find_first_of('%', begin + 2);
        if (end != std::string::npos) {
            std::string varName = string.substr(begin + 2, end - begin - 2);
            nlogCompilerUtils::replaceAll(string, "%v" + varName + "%", scope->findVar(varName)->value);
        }
    }

    std::cout << " Get: " << string << "\n";

    return string + "\n";
}

void ast::Command::replaceVars() {
    std::cout << "Base: " << string;
    size_t begin;
    while ((begin = string.find_first_of("%v")) != std::string::npos) {
        size_t end = string.find_first_of('%', begin + 2);
        if (end != std::string::npos) {
            std::string varName = string.substr(begin + 2, end - begin - 2);
            nlogCompilerUtils::replaceAll(string, "%v" + varName + "%", scope->findVar(varName)->value);
        }
    }

    std::cout << " ReplaceVars: " << string << "\n";
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
