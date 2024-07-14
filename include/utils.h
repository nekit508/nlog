#include <fstream>
#include <sstream>
#include "peg_parser/parser.h"

#pragma once

namespace nlogCompilerUtils {
    std::string readFile(std::string &path);

    void writeFile(std::string &path, std::string &str);

    void writeFile(std::string &path, std::string str);

    void replaceAll(std::string &str, const std::string &a, const std::string &ar);

    peg_parser::Parser::Result parse(peg_parser::Parser &parser, std::string &data);
}