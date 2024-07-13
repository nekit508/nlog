#include "../include/Scope.h"

ast::Var *ast::Scope::findVar(const std::string name) {
    for (const auto item: vars)
        if ("%v" + item->name + "%" == name)
            return item;

    return parent == nullptr ? parent->findVar(name) : nullptr;
}

ast::Scope *ast::Scope::findScope(const std::string name) {
    for (const auto &item: children) {
        if (item->name == name)
            return item;
    }

    return parent == nullptr ? parent->findScope(name) : nullptr;
}

ast::Command *ast::Scope::createCommand() {
    auto command = new ast::Command;

    command->scope = this;

    command->previous = lastCommand;
    lastCommand->next = command;
    lastCommand = command;

    return command;
}
