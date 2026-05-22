#pragma once 

#include "ast.h" 
#include "symbols.h" 
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct CompiledProgram {
    std::vector<uint32_t> code; // machine code-i zangvac
    std::vector<int> data; // global/static tvyalner
    std::unordered_map<std::string, uint32_t> functions; // funkcia -> hasce
    uint32_t entryPc = 0; // cragri skzbakan hascen
};

class CodeGen {
public:
    CompiledProgram compile(const std::shared_ptr<ProgramNode>& prog); // AST-n veracnum e compiled program-i

private:
    SymbolTable syms_; // bolor symbolneri pahpanum
    std::vector<uint32_t> code_; // stexvac instrukcianeri zangvac
    std::unordered_map<std::string, size_t> funcPc_; // funkcianeri pc hascener
    std::vector<std::pair<size_t, std::string>> callPatches_; // heto patch anelu kancher
    std::string currentFunc_; // hima compile arvogh funkcian

    size_t emit(uint32_t w); // avelacnuma  instruction code-i mej
    size_t pc() const; // veradardznuma himikva pc-n
    void patch(size_t at, uint32_t w); // poxuma instruction-y tvyal hasceum

    void analyze(const std::shared_ptr<ProgramNode>& prog); // naxnakan analiz AST-i vra
    void analyzeBlock(const std::shared_ptr<BlockNode>& block); // analiz anum e block-i mej
    int evalConstExpr(const std::shared_ptr<ASTNode>& node); // hashvuma constant expression-i arjeqy

    uint8_t compileExpr(const std::shared_ptr<ASTNode>& node, uint8_t out = 1); // expressiony veracuma instrukcianeri
    void compileBranch(const std::string& op, size_t at, size_t truePc, size_t falsePc); // branch instrukciana stexcum
    void compileStmt(const std::shared_ptr<ASTNode>& st); // statement compile anelu hamar
    void compileBlock(const std::shared_ptr<BlockNode>& block); // block compile anelu hamar
    void compileFunction(const std::shared_ptr<FunctionNode>& fn); // funkcia compile anelu hamar
};