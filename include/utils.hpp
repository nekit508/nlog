#pragma once

#include "peg_parser/parser.h"

namespace nlogCompilerUtils {
    std::string readFile(std::string &path) {
        std::fstream stream(path, std::ios::in);
        std::stringstream out;
        out << stream.rdbuf();
        return out.str();
    }

    void writeFile(std::string &path, std::string &str) {
        std::fstream stream(path, std::ios::out | std::ios::binary);
        stream.write(str.c_str(), str.size());
        stream.close();
    }

    void writeFile(std::string &path, std::string str) {
        std::fstream stream(path, std::ios::out | std::ios::binary);
        stream.write(str.c_str(), str.size());
        stream.close();
    }

    void replaceAll(std::string &str, const std::string &a, const std::string &ar) {
        uint64_t ind;
        while ((ind = str.find(a)) != std::string::npos)
            str.replace(ind, a.size(), ar);
    }

    peg_parser::Parser::Result parse(peg_parser::Parser &parser, std::string &data) {
        return parser.parseAndGetError(data);
    }
}