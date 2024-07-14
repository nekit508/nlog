#include <iostream>
#include <fstream>
#include <sstream>
#include "peg_parser/presets.h"
#include "peg_parser/generator.h"
#include <stack>
#include <regex>
#include <utility>
#include <list>

#include "../include/ast.h"
#include "../include/utils.h"
#include "../include/Scope.h"
#include "../include/Command.h"
#include "../include/Var.h"

#define DEBUG_MODE

#ifdef DEBUG_MODE
    #define debug(message) {std::cout << "[DEBUG] " << message << "\n";}
#else
    #define debug(message) {}
#endif

void setupParser(peg_parser::ParserGenerator<void>& gen) {
    gen["ws"] << "[\r\t\n ]";
    gen["ws"]->hidden = true;
    
    gen["code"        ] << "part*"; // code

    gen["part"        ] << "(compDir | syntax | (mlog ';') | (expression ';')) ws*"; // part of code, this can be expression or any syntax construction
    gen["compDir"     ] << "'#' ws* compDirLine ws* ';'"; // directive declaration
    gen["mlog"        ] << "'mlog' ws* (mlogCode | ('{' ws* (mlogCode ws* ';' ws*)* '}'))"; // mlogCode insertion
    gen["syntax"      ] << "if | while | funcDecl"; // part of code, that representing syntax construction (if-else, while etc.)
    gen["expression"  ] << "rvalue"; // part of code that representing processor's command
    gen["rvalue"      ] << "operator | exec | get | value | symbol"; // expression that returns value that can be stored
    gen["value"       ] << "string | number"; // represents constant value (numbers and strings)
    gen["operator"    ] << "binary | unary"; // special symbols that return values
    gen["unary"       ] << "increment | decrement"; // operator that receives one rvalue
    gen["binary"      ] << "set | equals | greater | smaller | greaterEq | smallerEq | plus"; // operator that receives two rvalues

    gen["mlogCode"    ] << "[%A-Za-z0-9 ]*"; // raw mlog code

    gen["compDirLine" ] << "(compDirPart ' '+)* compDirPart"; // line of directive declaration
    gen["compDirPart" ] << "[A-Za-z0-9\\.]*"; // part of directive line

    gen["if"          ]
            << "'if' ws* '(' ws* rvalue ws* ')' ws* '{' ws* code ws* '}' ws* ('elif' ws* '(' ws* rvalue ws* ')' ws* '{' ws* code ws* '}')? ws* (ws* 'else' ws* '{' ws* code ws* '}')?"; // syntax construction 'if'
    gen["while"       ] << "'while' ws* '(' ws* rvalue ws* ')' ws* '{' ws* code ws* '}'"; // syntax construction while rvalue is true, execute cod
    gen["funcDecl"    ] << "'func' ws* symbol ws* '(' ws* (symbol (ws* ',' ws* symbol)*)? ws* ')' ws* ':' ws* symbol ws* '{' ws* code ws* (return expression? ws*)? '}'";
    gen["return"      ] << "'return' ws* symbol ws* ';'"; // function return

    gen["exec"        ] << "symbol '(' ws* (rvalue (ws* ',' ws* rvalue)*)? ws* ')'"; // rvalue execution
    gen["symbol"      ] << "[a-zA-Z@] [a-zA-Z0-9]*"; // letters that mean variable, method, class or modifier

    gen["number"      ] << "[0-9]+ ('.' [0-9]+)?"; // constant number value
    gen["string"      ] << "('\"' ([a-zA-Z])* '\"')"; // constant string value

    gen["set"         ] << "symbol ws* '=' ws* rvalue"; // set symbol value to rvalue
    gen["get"         ] << "rvalue ws* '.' ws* symbol"; // get symbol from rvalue
    gen["equals"      ] << "rvalue ws* '==' ws* rvalue"; // equals
    gen["greater"     ] << "rvalue ws* '>' ws* rvalue"; // grater
    gen["smaller"     ] << "rvalue ws* '<' ws* rvalue"; // smaller
    gen["greaterEq"   ] << "rvalue ws* '>=' ws* rvalue"; // grater or equal
    gen["smallerEq"   ] << "rvalue ws* '<=' ws* rvalue"; // smaller or equal
    gen["plus"        ] << "rvalue ws* '+' ws* rvalue"; // rvalue plus rvalue

    gen["increment"   ] << "rvalue ws* '++'"; // add one to rvalue
    gen["decrement"   ] << "rvalue ws* '--'"; // sub one to rvalue

    gen.setStart(gen["code"]);
}

void parse(ast::Scope* scope, peg_parser::SyntaxTree* tree) {
    static std::stack<std::string> outputStack;

    std::string rule = tree->rule->name;

    debug(rule);

    if (rule == "mlogCode") {
        std::string commands = tree->string();

        size_t ind;
        while ((ind = commands.find_first_of('\n') )!= std::string::npos) {
            ast::Command *command = scope->createCommand();
            command->string = commands.substr(0, ind);
            commands.erase(0, ind+2);
        }

        ast::Command *command = scope->createCommand();
        command->string = commands;
    } else if (rule == "funcDecl") {
        std::string functionName = tree->inner[0]->string();
        auto size = tree->inner.size();

        peg_parser::SyntaxTree *code, *ret;
        auto* end = tree->inner[size-1].get();
        size_t offset;
        if (end->rule->name == "return") {
            offset = 3;
            ret = end;
            code = tree->inner[size-2].get();

            parse(scope, ret);
        } else if (end->rule->name == "code") {
            offset = 2;
            code = end;
        }

        std::string functionType = tree->inner[size - offset]->string();
        auto* functionScope = scope->createScope();
        functionScope->parent = scope;
        functionScope->type =
                functionType == "plain" ? 0 :
                functionType == "retPtr" ? 1 :
                functionType == "stack" ? 2 :
                throw ast::Error(tree, "Unknown function type " + functionType + ".");
        functionName += "_" + std::to_string(size - 1 - offset);
        functionScope->name = functionName;

        for (size_t i = 1; i < size - offset; i++) {
            std::string parameterName = tree->inner[i]->string();

            auto* var = new ast::Var();
            var->name = parameterName;

            functionScope->vars.push_back(var);
        }

        parse(functionScope, code);

        debug("Registered function with name " + functionName + ".")
    } else if (rule == "exec") {
        std::string functionName = tree->inner[0]->string();
        auto size = tree->inner.size();
        functionName += "_" + std::to_string(size-1);

        debug("Found function with name " + functionName + ".")
        auto* functionScope = scope->findScope(functionName);
        if (functionScope == nullptr)
            throw ast::Error(tree, "Function " + functionName + " not declared in this scope.");

        for (size_t i = 1; i < size; i++) {
            outputStack.emplace();
            parse(scope, tree->inner[i].get());
            std::string str = outputStack.top();
            outputStack.pop();

            functionScope->vars[i-1]->value = str;
        }

        if (functionScope->type == 0) {
            ast::copyInto(functionScope->getFirstCommand(),
                          functionScope->getLastCommand(),
                          scope->getLastCommand());
        }
    } else if (rule == "string" || rule == "number" || rule == "symbol") {
        if (!outputStack.empty()) {
            auto& str = outputStack.top();
            str = tree->string();
        }
    } else {
        for (const auto &item: tree->inner)
            parse(scope, item.get());
    }
}

ast::Scope* parse(std::string in, std::string out) {
    peg_parser::ParserGenerator<void> gen;
    setupParser(gen);

    std::string data = nlogCompilerUtils::readFile(in);

    peg_parser::Parser::Result result = nlogCompilerUtils::parse(gen.parser, data);
    auto* scope = new ast::Scope();

    std::cout << data << "\n";

    try {
        parse(scope, result.syntax.get());
        nlogCompilerUtils::writeFile(out, ast::translate(scope->getFirstCommand()));
        return scope;
    } catch (ast::Error& e) {
        e.print();
        return nullptr;
    }
}

int main() {
    ast::Command* c = new ast::StartCommand;
    c->replaceVars();

    parse("std.nlog", "std.mlog");

    return 0;
}