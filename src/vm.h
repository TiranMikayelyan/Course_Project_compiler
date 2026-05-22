#pragma once
#include "isa.h"
#include <cstdint>
#include <string>
#include <vector>

class VirtualMachine {
public:
    bool load(const std::string& execPath); // loads anum executable fayly
    int run(); // sksums program-i execution-y

private:
    std::vector<uint32_t> mem_; // himnakan memory
    int regs_[16]{}; // registerner
    uint32_t ip_ = 0; // instruction pointer
    int sp_ = isa::STACK_TOP; // stack pointer
    int bp_ = isa::STACK_TOP; // base pointer
    int dataBase_ = 0; // data section-i skizb
    bool cmpEq_ = false, cmpLt_ = false, cmpGt_ = false; // compare flag-ner
    bool running_ = false; // VM-i ashxatelu vijak
    bool trace_ = false; // debug/trace mode

public:
    void setTrace(bool on) { trace_ = on; } // miacnum/anjatuma trace mode-y

    int readMem(int addr) const; // karduma memory-ic
    void writeMem(int addr, int value); // gruma memory-i mej
    void push(int v); // valuea push anum stack
    int pop(); // stack-ic vercnuma value
    uint32_t fetch(); // vercnuma hajord instruction-y
    void step(); // katarela mek instruction
};