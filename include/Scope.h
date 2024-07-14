#pragma once

#include <vector>
#include <string>
#include "Command.h"
#include "Var.h"
#include <cstdint>

namespace ast {
    struct Scope {
        std::string name;
        uint64_t type;
        Scope *parent = nullptr;
        std::vector<Scope*> children;
        Command *lastCommand = nullptr;
        std::vector<Var *> vars;

        Var *findVar(const std::string& name);
        Scope *findScope(const std::string& name);
        Command *createCommand();
        Scope *createScope();

        Command *getFirstCommand() const;
        Command *getLastCommand() const;

        Scope();
    };
}