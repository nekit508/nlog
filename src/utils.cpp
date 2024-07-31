#include <iostream>
#include "../include/utils.h"

std::string *nlogCompilerUtils::readFile(std::string &path) {
    std::fstream stream(path, std::ios::in);
    std::stringstream out;
    out << stream.rdbuf();
    std::cout << out.str() << "\n";
    return new std::string(out.str());
}

void nlogCompilerUtils::writeFile(const std::string path, std::string &str) {
    std::fstream stream(path, std::ios::out | std::ios::binary);
    stream.write(str.c_str(), str.size());
    stream.flush();
    stream.close();
}

void nlogCompilerUtils::replaceAll(std::string &str, const std::string &a, const std::string &ar) {
    uint64_t ind;
    while ((ind = str.find(a)) != std::string::npos)
        str.replace(ind, a.size(), ar);
}

peg_parser::Parser::Result nlogCompilerUtils::parse(peg_parser::Parser &parser, std::string *data) {
    return parser.parseAndGetError(std::string_view(data->begin(), data->end()));
}
