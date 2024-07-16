#pragma once

#include "Scope.h"

namespace ast {
    struct Output {
        ast::Scope *owner;
        std::string value;
    };
}