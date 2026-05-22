#pragma once 

#include "codegen.h" 
#include <cstdint>
#include <string>
#include <vector>

enum class SectionType : uint32_t {

    Code = 1, // code section
    Data = 2, // data section
    SymbolTable = 3 // symbol table section
};

struct SectionHeader {

    uint32_t type; // section-i tesaky
    uint32_t size; // section-i chapy
    uint32_t offset; // file-i mej section-i texy
};

bool writeExecFile(
    const std::string& path,
    const CompiledProgram& prog
); // executable file-a grum

bool readExecFile(
    const std::string& path,
    std::vector<uint32_t>& code,
    std::vector<int>& data
); // executable file-ic karduma code ev data