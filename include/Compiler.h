#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include "peg_parser/presets.h"
#include "peg_parser/generator.h"
#include <stack>
#include <regex>
#include <utility>
#include <list>

#include "ast.h"
#include "utils.h"
#include "Scope.h"
#include "Command.h"
#include "Var.h"
#include "Output.h"
#include <iostream>

typedef std::shared_ptr<peg_parser::SyntaxTree> ST;

struct Compiler {
    peg_parser::ParserGenerator<void>* gen;
    static std::unordered_map<std::string, std::string> ifTypes;
    ast::Scope *root;

    Compiler();

    ~Compiler();

    void setupParser();

    void parse(ast::Scope *scope, const std::shared_ptr<peg_parser::SyntaxTree>& tree);

    void parseFile(ast::Scope *scope, std::string inputFile);

    void parseText(ast::Scope *scope, std::string *inputText);
};