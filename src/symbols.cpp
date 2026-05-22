#include "symbols.h"
#include <stdexcept>

int SymbolTable::allocDataWord(int init) {
    int off = nextDataWord_ * 4; // hashvum e data offset-y
    dataInit_.push_back(init); // avelacnum e skzbnakan arjeqy
    ++nextDataWord_; // ancnum e hajord data bari
    return off;
}

std::string SymbolTable::mangleStatic(const std::string& var) const {
    return currentFunc_.empty() ? ("__file_" + var) : (currentFunc_ + "::" + var); // sarqum e unique anun static variable-i hamar
}

void SymbolTable::registerFunction(const std::string& name, size_t nParams) {
    functions_[name] = nParams; // grancum e function-y
}

bool SymbolTable::hasFunction(const std::string& name) const {
    return functions_.count(name) > 0; // stugum e function ka te che
}

std::unordered_map<std::string, Symbol>& SymbolTable::activeScope() {
    if (currentFunc_.empty()) return globals_; // ete function chka, ashxatum enq global scope-um
    return funcScopes_[currentFunc_]; // veradardznum e current function-i scope-y
}

const std::unordered_map<std::string, Symbol>& SymbolTable::activeScope() const {
    if (currentFunc_.empty()) return globals_;
    auto it = funcScopes_.find(currentFunc_);
    if (it == funcScopes_.end()) throw std::runtime_error("no active function scope");
    return it->second;
}

void SymbolTable::declareGlobal(const std::string& name, StorageKind kind, int initValue) {
    std::string key = (kind == StorageKind::Static) ? mangleStatic(name) : name; // static-i hamar unique key
    if (globals_.count(key)) throw std::runtime_error("duplicate global: " + name); // stuguma krknvum e te che
    Symbol s;
    s.name = name;
    s.kind = kind;
    s.dataOffset = allocDataWord(initValue); // allocatea anum data section-um
    s.initValue = initValue;
    globals_[key] = s; // pahuma global map-um
}

void SymbolTable::beginFunction(const std::string& name) {
    currentFunc_ = name; // nshuma current function-y
    nextLocalSlot_ = -4; // local variable-nery sksvelu en BP-4-ic
    if (!funcScopes_.count(name))
        funcScopes_[name] = {}; // stexcuma nor scope
}

void SymbolTable::endFunction() {
    if (!currentFunc_.empty())
        localBytes_[currentFunc_] = -nextLocalSlot_; // pahuma local variable-neri chapy
    currentFunc_.clear(); // maqruma current function-y
    nextLocalSlot_ = 0;
}

int SymbolTable::localBytesFor(const std::string& func) const {
    auto it = localBytes_.find(func);
    return it != localBytes_.end() ? it->second : 0; // veradardznuma local byte-eri qanaky
}

Symbol& SymbolTable::declareParam(const std::string& name, int regIndex) {
    auto& scope = activeScope();
    if (scope.count(name)) throw std::runtime_error("duplicate param: " + name); // stuguma parameter-y krknvum a te che
    Symbol s;
    s.name = name;
    s.kind = StorageKind::Param;
    s.regIndex = regIndex; // pahuma parameter-i register-y
    scope[name] = s;
    return scope[name];
}

Symbol& SymbolTable::declareLocal(const std::string& name, StorageKind kind, int initValue) {
    if (kind == StorageKind::Global || kind == StorageKind::Static) {
        std::string key = (kind == StorageKind::Static) ? mangleStatic(name) : name;
        if (globals_.count(key)) return globals_[key]; // ete ka, veradardznuma 
        Symbol s;
        s.name = name;
        s.kind = kind;
        s.owner = currentFunc_;
        s.dataOffset = allocDataWord(initValue); // allocate a anum data section-um
        s.initValue = initValue;
        globals_[key] = s;
        return globals_[key];
    }

    auto& scope = activeScope();
    if (scope.count(name)) throw std::runtime_error("duplicate local: " + name); // stuguma local-y krknvuma te che

    Symbol s;
    s.name = name;
    s.kind = StorageKind::Local;
    s.owner = currentFunc_;

    nextLocalSlot_ -= 4; // stack-um texa rezerv anum
    s.stackOffset = nextLocalSlot_;

    s.initValue = initValue;
    scope[name] = s;

    return scope[name];
}

Symbol* SymbolTable::lookup(const std::string& name) const {
    if (!currentFunc_.empty()) {
        auto fit = funcScopes_.find(currentFunc_);
        if (fit != funcScopes_.end()) {
            auto it = fit->second.find(name);
            if (it != fit->second.end()) return const_cast<Symbol*>(&it->second); // pntruma function scope-um
        }
    }

    auto g = globals_.find(name);
    if (g != globals_.end() && g->second.kind == StorageKind::Global)
        return const_cast<Symbol*>(&g->second); // pntruma global variable

    if (!currentFunc_.empty()) {
        std::string stKey = currentFunc_ + "::" + name; // static variable-i key
        auto st = globals_.find(stKey);
        if (st != globals_.end()) return const_cast<Symbol*>(&st->second);
    }

    std::string fileKey = "__file_" + name; // file-level static key
    auto fs = globals_.find(fileKey);

    if (fs != globals_.end()) return const_cast<Symbol*>(&fs->second);

    return nullptr; // ete chi gtnvel
}