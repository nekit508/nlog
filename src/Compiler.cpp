#include "../include/Compiler.h"

#define DEBUG_MODE
#ifdef DEBUG_MODE
    #define debug(message) {std::cout << "[DEBUG] " << message << "\n";}
#else
    #define debug(message) {}
#endif

/*
jump 7 equal x false
jump 0 notEqual x false
jump 1 lessThan x false
jump 2 lessThanEq x false
jump 3 greaterThan x false
jump 4 greaterThanEq x false
jump 5 strictEqual x false
jump 6 always x false
*/

Compiler::Compiler() {
    root = new ast::Scope();

    ifTypes["notEqual"] = "equal";
    ifTypes["equals"] = "notEqual";
    ifTypes["greaterThanEq"] = "lessThan";
    ifTypes["greaterThan"] = "lessThanEq";
    ifTypes["lessThanEq"] = "greaterThan";
    ifTypes["lessThan"] = "greaterThanEq"; 

    gen = new peg_parser::ParserGenerator;
    setupParser();
}

Compiler::~Compiler() {
    delete gen;
    delete root;
}

void Compiler::setupParser() {
    (*gen)["wsnl"] << "[\r\t\n ]";
    (*gen)["wsnl"]->hidden = true;
    (*gen)["ws"] << "[\t ]";
    (*gen)["ws"]->hidden = true;

    (*gen)["code"        ] << "wsnl* part*"; // code

    (*gen)["part"        ] << "(compDir | syntax | (mlog ';') | (expression ';')) wsnl*"; // part of code, this can be expression or any syntax construction
    (*gen)["compDir"     ] << "'#' ws* (compDirPart ws*)* ';'"; // directive declaration
    (*gen)["mlog"        ] << "'mlog' wsnl* (mlogCode | ('{' wsnl* (mlogCode wsnl* ';' wsnl*)* '}'))"; // mlogCode insertion
    (*gen)["syntax"      ] << "if | while | funcDecl"; // part of code, that representing syntax construction (if-else, while etc.)
    (*gen)["expression"  ] << "rvalue"; // part of code that representing processor's command
    (*gen)["rvalue"      ] << "operator | exec | get | value | symbol"; // expression that returns value that can be stored
    (*gen)["value"       ] << "string | number"; // represents constant value (numbers and strings)
    (*gen)["operator"    ] << "binary | unary"; // special symbols that return values
    (*gen)["unary"       ] << "increment | decrement"; // operator that receives one rvalue
    (*gen)["binary"      ] << "set | equals | greater | smaller | greaterEq | smallerEq | plus"; // operator that receives two rvalues

    (*gen)["mlogCode"    ] << "[%A-Za-z0-9 ]*"; // raw mlog code

    (*gen)["compDirPart" ] << "string | symbol";

    (*gen)["if"          ]
            << "'if' wsnl* '(' wsnl* rvalue wsnl* ')' wsnl* '{' wsnl* code wsnl* '}' wsnl* ('elif' wsnl* '(' wsnl* rvalue wsnl* ')' wsnl* '{' wsnl* code wsnl* '}')? wsnl* (wsnl* 'else' wsnl* '{' wsnl* code wsnl* '}')?"; // syntax construction 'if'
    (*gen)["while"       ] << "'while' wsnl* '(' wsnl* rvalue wsnl* ')' wsnl* '{' wsnl* code wsnl* '}'"; // syntax construction while rvalue is true, execute cod
    (*gen)["funcDecl"    ] << "'func' wsnl* symbol wsnl* '(' (wsnl* symbol (wsnl* ',' wsnl* symbol)*)? wsnl* ')' wsnl* ':' wsnl* symbol wsnl* '{' wsnl* code wsnl* (return wsnl* expression? wsnl*)? '}'";
    (*gen)["return"      ] << "'return' wsnl* symbol wsnl* ';'"; // function return

    (*gen)["exec"        ] << "symbol '(' (wsnl* rvalue (wsnl* ',' wsnl* rvalue)*)? wsnl* ')'"; // rvalue execution
    (*gen)["symbol"      ] << "('%v' [a-zA-Z@] [a-zA-Z0-9]* '%') | ([a-zA-Z@] [a-zA-Z0-9]*)"; // letters that mean variable, method, class or modifier

    (*gen)["number"      ] << "[0-9]+ ('.' [0-9]+)?"; // constant number value
    (*gen).getRule("string")->node =
            peg_parser::grammar::Node::Rule(peg_parser::presets::createStringProgram("\"", "\"").parser.grammar);

    (*gen)["set"         ] << "symbol wsnl* '=' wsnl* rvalue"; // set symbol value to rvalue
    (*gen)["get"         ] << "rvalue wsnl* '.' wsnl* symbol"; // get symbol from rvalue
    (*gen)["equals"      ] << "rvalue wsnl* '==' wsnl* rvalue"; // equals
    (*gen)["notEquals"   ] << "rvalue wsnl* '!=' wsnl* rvalue"; // not equals
    (*gen)["greater"     ] << "rvalue wsnl* '>' wsnl* rvalue"; // grater
    (*gen)["smaller"     ] << "rvalue wsnl* '<' wsnl* rvalue"; // smaller
    (*gen)["greaterEq"   ] << "rvalue wsnl* '>=' wsnl* rvalue"; // grater or equal
    (*gen)["smallerEq"   ] << "rvalue wsnl* '<=' wsnl* rvalue"; // smaller or equal
    (*gen)["plus"        ] << "rvalue wsnl* '+' wsnl* rvalue"; // rvalue plus rvalue

    (*gen)["increment"   ] << "rvalue wsnl* '++'"; // add one to rvalue
    (*gen)["decrement"   ] << "rvalue wsnl* '--'"; // sub one from rvalue

    (*gen).setStart((*gen)["code"]);
}

void Compiler::parse(ast::Scope *scope, const ST& tree) {
    static std::stack<ast::Output> outputStack;


    std::string rule = tree->rule->name;

    debug("Rule: " + rule);

    if (rule == "mlogCode") {
        ast::Command *command = scope->createCommand();
        command->string = tree->string();
    } else if (rule == "if") {
        ST ifCondition = tree->inner[0];

        outputStack.emplace(scope);
        parse(scope, ifCondition);
        ast::Output value = outputStack.top();
        outputStack.pop();


    } else if (rule == "funcDecl") {
        auto* functionScope = scope->createScope();
        std::string functionName = tree->inner[0]->string();
        auto size = tree->inner.size();

        ST code, ret;
        auto end = tree->inner[size-1];
        size_t offset;
        if (end->rule->name == "return") {
            offset = 3;
            ret = end;
            code = tree->inner[size-2];

            outputStack.emplace(scope);
            parse(functionScope, ret);
            ast::Output value = outputStack.top();
            outputStack.pop();

            functionScope->value = value.value;
        } else if (end->rule->name == "code") {
            offset = 2;
            code = end;
        }

        std::string functionType = tree->inner[size - offset]->string();
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
            outputStack.emplace(scope);
            parse(scope, tree->inner[i]);
            ast::Output out = outputStack.top();
            outputStack.pop();

            functionScope->vars[i-1]->value = out.value;
        }

        if (functionScope->type == 0) {
            ast::copyInto(functionScope->getFirstCommand(),
                          functionScope->getLastCommand(),
                          scope->getLastCommand());

            if (!outputStack.empty()) {
                std::string value = std::string(functionScope->value);
                ast::replaceVars(value, functionScope);
                outputStack.top().value = value;
            }
        }
    } else if (rule == "string" || rule == "number" || rule == "symbol") {
        if (!outputStack.empty()) {
            outputStack.top().value = tree->string();
        }
    } else if (rule == "compDir") {
        std::vector<std::string> parts;

        for (const auto &item: tree->inner)
            parts.push_back(item->string());

        if (parts[0] == "include") {
            // remove " from string
            parseFile(scope, parts[1].substr(1, parts[1].size()-2));
        }
    } else {
        for (const auto &item: tree->inner)
            parse(scope, item);
    }
}

void Compiler::parseText(ast::Scope *scope, std::string *inputText) {
    auto var = nlogCompilerUtils::parse(gen->parser, inputText).syntax;
    parse(scope, var);
}

void Compiler::parseFile(ast::Scope *scope, std::string inputFile) {
    std::string* data = nlogCompilerUtils::readFile(inputFile);

    try {
        parseText(scope, data);
    } catch (ast::Error& e) {
        e.print();
    }

    delete data;
}
