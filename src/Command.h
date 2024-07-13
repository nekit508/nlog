#pragma once
#include "Scope.h"
#include "utils.hpp"

namespace ast {
    /** Processor's unit command (like `add a b c`). */
    struct Command {
        Scope *scope;
        Command *previous, *next;

        std::string string;

        virtual std::string get();
    };
}