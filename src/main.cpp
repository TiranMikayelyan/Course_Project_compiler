#include "codegen.h"
#include "exec_format.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

static std::string readFile(const std::string& path) {
    std::ifstream in(path); // bacuma fayly
    if (!in) throw std::runtime_error("cannot open: " + path);

    std::ostringstream ss; // string stream file-i parunakutyuny pahhelu hamar
    ss << in.rdbuf(); // file-i bolor tvyalnery gruma ss-i mej

    return ss.str(); // veradardznuma file-i texty
}

static void usage() {
    std::cout << "Usage:\n"
        << "  course_compiler compile <source.lang> -o <out.exec>\n"
        << "  course_compiler run <out.exec>\n";
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) { usage(); return 1; } // ete argument chka usagea tpum

        std::string cmd = argv[1]; // hramany

        if (cmd == "compile" && argc >= 5 && std::string(argv[3]) == "-o") {
            std::string srcPath = argv[2]; // source file-i chanaparh
            std::string outPath = argv[4]; // output exec file-i anun

            Lexer lex(readFile(srcPath)); // stexcuma lexer
            Parser parser(lex.tokenize()); // stexcuma parser tokennerov

            auto prog = parser.parseProgram(); // sarquma AST

            CodeGen gen; // code generator
            CompiledProgram compiled = gen.compile(prog); // compilea anum

            if (!writeExecFile(outPath, compiled))
                throw std::runtime_error("failed to write " + outPath);

            std::cout << "Compiled -> " << outPath
                << "  (code words=" << compiled.code.size()
                << ", data words=" << compiled.data.size() << ")\n";

            if (argc >= 6 && std::string(argv[5]) == "--dump") {
                for (size_t i = 0; i < compiled.code.size(); ++i)
                    std::cout << i << ": 0x" << std::hex << compiled.code[i] << std::dec << "\n";
            }

            return 0;
        }

        if (cmd == "run" && argc >= 3) {
            VirtualMachine vm; // stexcuma VM

            if (argc >= 4 && std::string(argv[3]) == "--trace")
                vm.setTrace(true); // miacnuma trace mode

            if (!vm.load(argv[2]))
                throw std::runtime_error("failed to load exec");

            vm.run(); // sksuma execution-y
            return 0;
        }

        usage();
        return 1;

    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n"; // error-i texty
        return 1;
    }
}