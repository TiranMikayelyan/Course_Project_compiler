#pragma once
#include <string>
#include <unordered_map>
#include <vector>

enum class StorageKind { Global, Static, Local, Param };

struct Symbol {
    std::string name;
    StorageKind kind = StorageKind::Local; // variable-i tesaky
    int dataOffset = -1; // data section-i offset
    int stackOffset = 0; // stack-i offset
    int regIndex = -1; // parameter-i register-y (R2, R3, ...)
    int initValue = 0; // skzbnakan arjeq
    std::string owner; // vor function-in e patkanum
};

class SymbolTable {
public:
    void declareGlobal(const std::string& name, StorageKind kind, int initValue);
    void beginFunction(const std::string& name); // sksum e nor function scope
    void endFunction(); // pakuum e function scope-y
    int localBytesFor(const std::string& func) const; // local variable-neri size
    Symbol& declareLocal(const std::string& name, StorageKind kind, int initValue = 0);
    Symbol& declareParam(const std::string& name, int stackOffset);
    Symbol* lookup(const std::string& name) const; // symbol-i pntrum
    const std::vector<int>& dataImage() const { return dataInit_; }
    void registerFunction(const std::string& name, size_t nParams);
    bool hasFunction(const std::string& name) const;

private:
    std::string currentFunc_; // himikva function-i anuny
    int nextDataWord_ = 0; // hajord data offset
    int nextLocalSlot_ = 0; // hajord local stack slot
    std::vector<int> dataInit_; // global/static data
    std::unordered_map<std::string, Symbol> globals_;
    std::unordered_map<std::string, std::unordered_map<std::string, Symbol>> funcScopes_;
    std::unordered_map<std::string, size_t> functions_;
    std::unordered_map<std::string, int> localBytes_;
    std::string mangleStatic(const std::string& var) const; // static variable-i unique anun
    int allocDataWord(int init); // data section-um tex allocate anum
    std::unordered_map<std::string, Symbol>& activeScope(); // active scope
    const std::unordered_map<std::string, Symbol>& activeScope() const;
};