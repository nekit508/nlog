#include "../include/Command.h"

std::string ast::Command::get() {
    std::string string(this->string);

    size_t begin;
    while ((begin = string.find_first_of("%v")) != std::string::npos) {
        size_t end = string.find_first_of('v', begin);
        if (end != std::string::npos) {
            std::string varName = string.substr(begin, end - begin);
            nlogCompilerUtils::replaceAll(string, varName, scope->findVar(varName)->value);
        }
    }

    return string;
}
