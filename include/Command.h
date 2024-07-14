#pragma once
#include "utils.h"

namespace ast {
    struct Scope;
    /** Processor's unit command (like `add a b c`). */
    struct Command {
        Scope *scope = nullptr;
        Command *previous = nullptr, *next = nullptr;
        std::string string;

        virtual std::string get();
        virtual void replaceVars();
        virtual Command* copy();
    };

    struct StartCommand : Command {
        std::string get() override;
        void replaceVars() override;
        Command * copy() override;
    };
}