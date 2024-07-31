#pragma once
#include "utils.h"

namespace ast {
    struct Scope;
    /** Processor's unit command (like `add a b c`). */
    struct Command {
        Scope *scope = nullptr;
        Command *previous = nullptr, *next = nullptr;
        std::string string;

        ~Command();

        virtual void get(std::string &output);
        virtual void replaceVars();
        virtual Command* copy();
    };

    struct StartCommand : Command {
        void get(std::string &output) override;
        void replaceVars() override;
        Command * copy() override;
    };

    struct AnchorCommand : Command {
        std::string name;

        void get(std::string &output) override;
        void replaceVars() override;
        Command * copy() override;
    };
}