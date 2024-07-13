#include <iostream>
#include <fstream>
#include <sstream>
#include <peg_parser/presets.h>
#include <peg_parser/generator.h>
#include <stack>
#include <regex>
#include <vector>
#include <unordered_map>

#include "include/utils.hpp"

using std::string;

peg_parser::ParserGenerator parser;

struct Code {
    std::string mlog, header;

    Code& operator+=(Code c) {
        mlog += c.mlog;
        header  += c.header;
        return *this;
    }

    string full() {
        return mlog + header;
    }
};

struct Var {
    std::string str;

    bool used = false;
};

struct Function {
    std::string name;
    std::vector<std::string> args;
    Code code;
};

struct State {
    std::unordered_map<string, Function> functions;

    uint64_t anchors;

    uint64_t vars;
    std::stack<Var> currentVar;

    void addFunc(Function func) {
        functions[func.name] = func;
    }

    const Function getFunc(string name) {
        return functions.contains(name) ? functions[name] : Function();
    }

    Var* createVar() {
        return createVar(std::string("var").append(std::to_string(vars++)));
    }

    Var* createVar(std::string str) {
        Var v;
        v.str = str;
        currentVar.push(v);
        return getVar();
    }

    Var* getVar() {
        return currentVar.size() != 0 ? &currentVar.top() : createVar();
    }

    Var delVar() {
        Var v = currentVar.top();
        currentVar.pop();
        return v;
    }
};

peg_parser::Parser::Result parse(string path) {
    auto str = readFile(path);
    auto result = parser.parser.parseAndGetError(*str);

    return result;
}

Code parse(std::shared_ptr<peg_parser::SyntaxTree> tree) {
    static State state;

    Code out;
    auto rule = tree->rule->name;
    auto outVar = state.getVar();

    std::cout << rule << "\n";

    if (rule == "set") {
        state.createVar(tree->inner[0]->string());
        Code code = parse(tree->inner[1]);
        Var var = state.delVar();

        if (var.used)
            out += code;
        else
            out.mlog += ("set " + tree->inner[0]->string() + " " + (var.used ? var.str : code.mlog) + "\n");

        state.delVar();
    }
    else if (rule == "plus") {
        outVar->used = true;

        state.createVar();
        Code l = parse(tree->inner[0]);
        Var lv = state.delVar();

        state.createVar();
        Code r = parse(tree->inner[1]);
        Var rv = state.delVar();

        if (lv.used)
            out += l;
        if (rv.used)
            out += r;

        out.mlog += ("op add " + outVar->str + " " + (lv.used ? lv.str : l.mlog) + " " + (rv.used ? rv.str : r.mlog) + "\n");
    }
    else if (rule == "number" || rule == "string" || rule == "symbol") {
        outVar->used = false;
        out.mlog += tree->string();
    }
    else if (rule == "get") {
        state.createVar();
        Code l = parse(tree->inner[0]);
        Var lv = state.delVar();

        state.createVar();
        Code r = parse(tree->inner[1]);
        Var rv = state.delVar();
        __glibcxx_assert(!rv.used);
        // right operand is always symbol

        if (lv.used)
            out += l;
        string left = lv.used ? lv.str : l.mlog;
        string right = r.mlog;

        if (right == "enabled") {
            outVar->used = true;
            out.mlog += ("sensor " + outVar->str + " " + left + " @" + right + "\n");
        }
    }
    else if (rule == "if") {
        string jumps, code;

        string endAnchor = std::to_string(state.anchors++);
        bool hasElse = false;
        for (uint64_t i = 0; i < tree->inner.size(); i += 2) {
            string anchor = std::to_string(state.anchors++);

            if (i + 1 == tree->inner.size() && tree->inner.size() % 2 == 1) {
                hasElse = true;

                jumps += ("jump %al" + anchor + " always 0 0\n");

                state.createVar();
                code += ("%a" + anchor + " " + parse(tree->inner[i]).mlog);
                state.delVar();
            } else {
                state.createVar();
                Code cond = parse(tree->inner[i]);
                Var condVar = state.delVar();

                if (condVar.used)
                    out += cond;
                jumps += ("jump %al" + anchor + " equal " + (condVar.used ? condVar.str : cond.mlog) + " true\n");

                state.createVar();
                code += ("%a" + anchor + " " + parse(tree->inner[i+1]).mlog + "jump %al" + endAnchor + " always 0 0\n");
                state.delVar();
            }
        }

        code.append("%a" + endAnchor + " ");

        out.mlog.append(jumps + code);
    }
    else if (rule == "exec") {
        state.createVar();
        Code func = parse(tree->inner[0]);
        Var funcVar = state.delVar();

        string args, operand;

        if (funcVar.used)
            out += func;
        operand.append(funcVar.used ? funcVar.str : func.mlog);

        Function function = state.getFunc(operand + std::to_string(tree->inner.size()-1));
        int a = 1;
        std::cout << a++ << "\n";
        if (function.name.size() == 0)
            return out;
        for (uint64_t i = 1; i < tree->inner.size(); i++) {
            std::cout << function.args[i] << "\n";
            state.createVar(function.args[i]);
            Code arg = parse(tree->inner[i]);
            Var argVar = state.delVar();

            if (argVar.used)
                out += arg;

            string argument(argVar.used ? argVar.str : arg.mlog);

            std::cout << function.code.mlog << ";" << "%" + function.args[i] << ";" << argument << "\n";
            replaceAll(function.code.mlog, "%" + function.args[i], argument);

        }
        std::cout << a++ << "\n";
        out += function.code;
    }
    else if (rule == "funcDecl") {
        outVar->used = true;
        Function func;

        auto name = tree->inner[0];
        auto code = tree->inner[tree->inner.size()-1];
        auto modifier = tree->inner[tree->inner.size()-2];

        state.createVar();
        Code nameCode = parse(name);
        Var nameVar = state.delVar();
        __glibcxx_assert(!nameVar.used);
        // name is always symbol
        func.name = nameCode.mlog + std::to_string(tree->inner.size() -3);

        state.createVar();
        Code modifierCode = parse(modifier);
        Var modifierVar = state.delVar();
        __glibcxx_assert(!modifierVar.used);
        // modifier is always symbol

        std::vector<string> args;
        if (modifierCode.mlog == "plain") {
            for (uint64_t i = 2; i < tree->inner.size()-2; i++) {
                state.createVar();
                Code argCode = parse(tree->inner[i]);
                Var argVar = state.delVar();
                __glibcxx_assert(!argVar.used);
                // arguments are always symbols

                args.push_back(argCode.mlog);
                string argName = func.name + "_" + argCode.mlog;
                func.args.push_back(argName);
            }

            state.createVar();
            Code codeCode = parse(code);
            Var codeVar = state.delVar();

            for (uint64_t i = 0; i < args.size(); i++) {
                replaceAll(codeCode.mlog, " " + args[i] + " ", " %" + func.args[i] + " ");
            }

            func.code = codeCode;

            outVar->str = codeVar.str;
        }

        std::cout << func.name << "\n";
        state.addFunc(func);
    }
    else if (rule == "mlogCode") {
        out.mlog.append(tree->string() + "\n");
    }
    else if (rule == "compDirLine") {
        if (tree->inner[0]->string() == "include") {
            string path = tree->inner[1]->string();

            std::cout << path << "\n";

            state.createVar();
            Code code = parse(parse(path).syntax);
            state.delVar();

            out += code;
        }
    }
    else {
        for (auto t: tree->inner)
            out += parse(t);
    }

    return out;
}

string processAnchors(string code) {
    uint64_t line = 0;
    std::vector<uint64_t> lines, nums;

    string temp;
    for (uint64_t i = 0; i < code.size(); i++) {
        char chr = code[i];
        if (chr == '\n') {
            line++;
            temp.clear();
        } else {
            temp += chr;

            if (chr == '%') {
                string n;
                if (code.size() - i > 1 && code[i+1] == 'a' && isdigit(code[i+2])) {
                    uint64_t j = i+1;
                    while (code[j+1] != ' ') {
                        j++;
                        n += code[j];
                    }

                    uint64_t num = std::stoull(n);

                    nums.push_back(num);
                    lines.push_back(line);
                }
            }


        }

        if (i == code.size() - 1) {
            if (temp.size() > 0) {
                code += "end";
                break;
            }
        }
    }

    for (uint64_t i = 0; i < lines.size(); i++) {
        string a = string("%a").append(std::to_string(nums[i])).append(" ");
        string al = string("%al").append(std::to_string(nums[i]));
        string alr = std::to_string(lines[i]);

        code.replace(code.find(a), a.size(), "");

        replaceAll(code, al, alr);
    }

    return code;
}

/*
read result cell1 0
write result cell1 0
print "frog"
set result 1
op add result 1 2
op cos result 1 b
wait 0.5
stop
end
jump -1 notEqual x false
printflush message1
sensor result block1 @enabled
sensor result block1 @water
sensor result block1 @copper
 */

#pragma clang diagnostic push
#pragma ide diagnostic ignored "err_typecheck_invalid_operands"
#pragma ide diagnostic ignored "err_typecheck_subscript_value"
#pragma ide diagnostic ignored "err_ovl_no_viable_member_function_in_call"
int main() {
    parser["code"        ] << "part*"; // code

    parser["part"        ] << "(compDir | syntax | (mlog ';') | (expression ';')) ' '*"; // part of code, this can be expression or any syntax construction
    parser["compDir"     ] << "'#' ' '* compDirLine ' '* ';'"; // directive declaration
    parser["mlog"        ] << "'mlog' ' '* mlogCode"; // mlogCode insertion
    parser["syntax"      ] << "if | while | funcDecl"; // part of code, that representing syntax construction (if-else, while and etc)
    parser["expression"  ] << "rvalue"; // part of code that representing processor's command
    parser["rvalue"      ] << "operator | exec | get | value | symbol"; // expression that returns value that can be stored
    parser["value"       ] << "string | number"; // represents constant value (numbers and strings)
    parser["operator"    ] << "binary | unary"; // special symbols that return values
    parser["unary"       ] << "increment | decrement"; // operator that receives one rvalue
    parser["binary"      ] << "set | equals | greater | smaller | greaterEq | smallerEq | plus"; // operator that receives two rvalues

    parser["mlogCode"    ] << "[A-Za-z0-9 ]*"; // raw mlog code

    parser["compDirLine" ] << "(compDirPart ' '+)* compDirPart"; // line of directive declaration
    parser["compDirPart" ] << "[A-Za-z0-9\\.]*"; // part of directive line

    parser["if"          ]
        << "'if' ' '* '(' ' '* rvalue ' '* ')' ' '* '{' ' '* code ' '* '}' ' '* ('elif' ' '* '(' ' '* rvalue ' '* ')' ' '* '{' ' '* code ' '* '}')? ' '* (' '* 'else' ' '* '{' ' '* code ' '* '}')?"; // syntax construction 'if'
    parser["while"       ] << "'while' ' '* '(' ' '* rvalue ' '* ')' ' '* '{' ' '* code ' '* '}'"; // syntax construction while rvalue is true, execute cod
    parser["funcDecl"    ] << "'func' ' '* symbol ' '* '(' ' '* (rvalue (' '* ',' ' '* rvalue)*)? ' '* ')' ' '* ':' ' '* symbol ' '* '{' ' '* code ' '* '}'";

    parser["exec"        ] << "rvalue '(' ' '* (rvalue (' '* ',' ' '* rvalue)*)? ' '* ')'"; // rvalue execution
    parser["symbol"      ] << "[a-zA-Z@] [a-zA-Z0-9]*"; // letters that mean variable, method, class or modifier

    parser["number"      ] << "[0-9]+ ('.' [0-9]+)?"; // constant number value
    parser["string"      ] << "'\"' [a-zA-Z0-9 \\[\\]]* '\"'"; // constant string value

    parser["set"         ] << "symbol ' '* '=' ' '* rvalue"; // set symbol value to rvalue
    parser["get"         ] << "rvalue ' '* '.' ' '* symbol"; // get symbol from rvalue
    parser["equals"      ] << "rvalue ' '* '==' ' '* rvalue"; // equals
    parser["greater"     ] << "rvalue ' '* '>' ' '* rvalue"; // grater
    parser["smaller"     ] << "rvalue ' '* '<' ' '* rvalue"; // smaller
    parser["greaterEq"   ] << "rvalue ' '* '>=' ' '* rvalue"; // grater or equal
    parser["smallerEq"   ] << "rvalue ' '* '<=' ' '* rvalue"; // smaller or equal
    parser["plus"        ] << "rvalue ' '* '+' ' '* rvalue"; // rvalue plus rvalue

    parser["increment"   ] << "rvalue ' '* '++'"; // add one to rvalue
    parser["decrement"   ] << "rvalue ' '* '--'"; // sub one to rvalue

    parser.setStart(parser["code"]);

    auto r = parse("in.nlog");
    std::cout << r.syntax->fullString << "\n";
    string output = processAnchors(parse(r.syntax).full());
    writeFile(string("out.mlog"), output);

    return 0;
}
#pragma clang diagnostic pop

