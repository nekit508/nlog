#include <malloc.h>
#include "../include/Compiler.h"

int main(int argc, char** argv) {
    std::string inputFile, outputFile;

    std::vector<std::string> includeDirectories;

    for (int i = 0; i < argc; ++i) {
        std::string arg(argv[i]);

        if (arg.starts_with("-i")) {
            inputFile = arg.substr(2);
        } else if (arg.starts_with("-o")) {
            outputFile = arg.substr(2);
        } else if (arg.starts_with("-cp")) {
            includeDirectories.emplace_back(arg.substr(3));
        }
    }

    if (inputFile.empty()) {
        std::printf("Error: no main file declared.");
    } else {
        if (outputFile.empty()) {
            outputFile = inputFile + ".mlog";
        }

        auto *compiler = new Compiler();

        compiler->parseFile(compiler->root, inputFile);

        std::string output;
        compiler->root->writeToString(output);
        nlogCompilerUtils::writeFile(outputFile, output);

        delete compiler;
    }

    return 0;
}