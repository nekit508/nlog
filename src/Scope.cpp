#include "../include/Scope.h"
#include "../include/ast.h"

ast::Var *ast::Scope::findVar(const std::string& name) {
    for (const auto item: vars)
        if (item->name == name)
            return item;

    return parent == nullptr ? parent->findVar(name) : nullptr;
}

ast::Scope *ast::Scope::findScope(const std::string& name) {
    for (const auto &item: children) {
        if (item->name == name)
            return item;
    }

    if (parent == nullptr) {
        ast::Error err(nullptr, "Scope with name " + name + " does not exists.");
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
