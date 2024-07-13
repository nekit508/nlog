#pragma once

#include <vector>
#include <string>
#include "Command.h"
#include "Var.h"

namespace ast {
    struct Scope {
        std::string name;
        uint64_t type;
        Scope *parent;
        std::vector<Scope*> children;
        Command *lastCommand;
        std::vector<Var *> vars;

        Var *findVar(const std::string name);
        Scope *findScope(const std::string name);
        Command *createCommand();
    };
}