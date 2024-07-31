#include "../include/Scope.h"
#include "../include/ast.h"

ast::Var *ast::Scope::findVar(const std::string& name) {
    for (const auto item: vars)
        if (item->name == name)
            return item;

    if (parent == nullptr) {
        ast::Error err(nullptr, "Var with name " + name + " does not exists.");
        err.print();
        return nullptr;
    }

    return parent->findVar(name);
}

ast::Scope *ast::Scope::findScope(const std::string& name) {
    for (const auto &item: children) {
        if (item->name == name)
            return item;
    }

    if (parent == nullptr) {
        ast::Error err(nullptr, "Scope with name " + name + " does not exists.");
        err.print();
        return nullptr;
    }

    return parent->findScope(name);
}

ast::Command *ast::Scope::createCommand() {
    auto command = new ast::Command;

    command->scope = this;
    lastCommand = getLastCommand();

    lastCommand->next = command;
    command->previous = lastCommand;
    lastCommand = command;

    return command;
}

ast::Scope *ast::Scope::createScope() {
    auto *scope = new Scope();

    scope->parent = this;
    children.push_back(scope);

    return scope;
}

ast::Command *ast::Scope::getFirstCommand() const {
    ast::Command *output = lastCommand;

    while (output->previous != nullptr)
        output = output->previous;

    return output;
}

ast::Command *ast::Scope::getLastCommand() const {
    ast::Command *output = lastCommand;

    while (output->next != nullptr)
        output = output->next;

    return output;
}

ast::Scope::Scope() {
    auto *start = new ast::StartCommand;
    start->replaceVars();

    start->scope = this;
    lastCommand = start;
}

ast::Scope::~Scope() {
    for (const auto &item: vars)
        delete item;
    for (const auto &item: children)
        delete item;

    delete lastCommand;
}


void ast::Scope::writeToString(std::string &output) {
    ast::Command* current = getFirstCommand();

    while (current != nullptr) {
        current->get(output);
        current = current->next;
    }
}

void ast::Scope::replaceVars(std::string &code) {
    size_t begin;
    while ((begin = code.find_first_of("%v")) != std::string::npos) {
        size_t end = code.find_first_of('%', begin + 2);
        if (end != std::string::npos) {
            std::string varName = code.substr(begin + 2, end - begin - 2);
            nlogCompilerUtils::replaceAll(code, "%v" + varName + "%", this->findVar(varName)->value);
        } else {
            break;
        }
    }
}