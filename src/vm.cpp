#include "vm.h"
#include "exec_format.h"
#include "isa.h"
#include <iostream>

int VirtualMachine::readMem(int addr) const {
    int idx = addr / isa::WORD_SIZE; // hashvuma memory index-y
    if (addr < 0 || idx < 0 || idx >= static_cast<int>(mem_.size())) return 0; // stuguma sahmanic dursa te che
    return static_cast<int>(mem_[idx]); // veradardznuma memory-i arjeqy
}

void VirtualMachine::writeMem(int addr, int value) {
    int idx = addr / isa::WORD_SIZE;
    if (addr < 0 || idx < 0 || idx >= static_cast<int>(mem_.size())) return;
    mem_[idx] = static_cast<uint32_t>(value); // grum e value-n memory
}

void VirtualMachine::push(int v) {
    sp_ -= 4; // stack pointer-y ijacnuma 
    writeMem(sp_, v); // value-n gruma stack-um
}

int VirtualMachine::pop() {
    int v = readMem(sp_); // kardum e verjin value-n stack-ic
    sp_ += 4; // stack pointer-y avelanuma
    return v;
}

bool VirtualMachine::load(const std::string& execPath) {
    std::vector<uint32_t> code;
    std::vector<int> data;

    if (!readExecFile(execPath, code, data)) return false; // karduma exec fayly

    mem_.assign(isa::MEM_SIZE / isa::WORD_SIZE, 0); // allocatea anum memory

    int codeWords = static_cast<int>(code.size());

    for (int i = 0; i < codeWords; ++i) mem_[i] = code[i]; // code-y loada anum

    dataBase_ = codeWords * isa::WORD_SIZE; // data section-i skizb

    for (size_t i = 0; i < data.size(); ++i)
        writeMem(dataBase_ + static_cast<int>(i * 4), data[i]); // data-n gruma memory-um

    ip_ = 0; // instruction pointer reset
    sp_ = isa::STACK_TOP; // stack pointer reset
    bp_ = isa::STACK_TOP; // base pointer reset
    running_ = true; // VM-y start-a anum

    return true;
}

uint32_t VirtualMachine::fetch() {
    return mem_[ip_++]; // vercnuma hajord instruction-y
}

void VirtualMachine::step() {
    uint32_t insnPc = ip_; // himikva instruction-i address

    uint32_t w = fetch(); // vercnuma instruction-y

    uint8_t op = isa::opcode(w); // opcode
    uint8_t rd = isa::rd(w); // destination register
    uint8_t rs1 = isa::rs1(w); // source register 1
    uint8_t rs2 = isa::rs2(w); // source register 2
    int16_t imm = isa::imm12(w); // immediate value

    auto R = [&](uint8_t i) -> int& { return regs_[i]; }; // register access

    switch (op) {
    case isa::MOV_REG: R(rd) = R(rs1); break; // register copy
    case isa::MOV_IMM: R(rd) = imm; break; // immediate value register
    case isa::LOAD_DATA: R(rd) = readMem(dataBase_ + imm); break; // data section-ic load
    case isa::STORE_DATA: writeMem(dataBase_ + imm, R(rs2)); break; // data section-um store
    case isa::LOAD_LOCAL: R(rd) = readMem(bp_ + imm); break; // local variable load
    case isa::STORE_LOCAL: writeMem(bp_ + imm, R(rs2)); break; // local variable store
    case isa::ADD: R(rd) = R(rs1) + R(rs2); break; // gumarum
    case isa::SUB: R(rd) = R(rs1) - R(rs2); break; // hanum
    case isa::MUL: R(rd) = R(rs1) * R(rs2); break; // bazmapatkum
    case isa::DIV: R(rd) = (R(rs2) == 0) ? 0 : R(rs1) / R(rs2); break; // bajanum

    case isa::CMP:
        cmpEq_ = (R(rs1) == R(rs2)); // havasar
        cmpLt_ = (R(rs1) < R(rs2)); // poqr
        cmpGt_ = (R(rs1) > R(rs2)); // mec
        break;

    case isa::BR_EQ:
        if (cmpEq_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm); // jump ete ==
        break;

    case isa::BR_NE:
        if (!cmpEq_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm); // jump ete !=
        break;

    case isa::BR_LT:
        if (cmpLt_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm); // jump ete <
        break;

    case isa::BR_GT:
        if (cmpGt_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm); // jump ete >
        break;

    case isa::BR_LTE:
        if (cmpEq_ || cmpLt_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm); // jump ete <=
        break;

    case isa::BR_GTE:
        if (cmpEq_ || cmpGt_) ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm); // jump ete >=
        break;

    case isa::JMP:
        ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm); // unconditional jump
        break;

    case isa::PUSH:
        push(R(rs1)); // push stack
        break;

    case isa::POP:
        R(rd) = pop(); // pop stack
        break;

    case isa::PUSH_BP:
        push(bp_); // pahuma base pointer-y
        break;

    case isa::PUSH_NEXT_PC:
        push(static_cast<int>(ip_) + 1); // pahuma return address-y
        break;

    case isa::CALL: {
        bp_ = sp_ + 4; // stexcuma nor stack frame
        ip_ = static_cast<uint32_t>(static_cast<int>(ip_) + imm); // ancnuma function
        break;
    }

    case isa::RET: {
        int retVal = R(rs1); // return value
        int frameBp = bp_; // himikva BP

        int retPc = readMem(frameBp - 4); // return address

        bp_ = readMem(frameBp); // verakangnuma hin BP

        sp_ = frameBp - 4; // verakangnuma SP

        ip_ = static_cast<uint32_t>(retPc); // veradardz depiq callic heto

        R(1) = retVal; // result-y pahuma R1-um

        break;
    }

    case isa::PRINT:
        std::cout << R(rs1) << std::endl; // tpuma arjeqy
        break;

    case isa::INPUT: {
        int x;
        std::cin >> x; // user input
        R(rd) = x; // pahuma register-um
        break;
    }

    case isa::EXIT:
        running_ = false; // kangnecnuma VM-n
        break;

    case isa::ALLOC_STACK:
        sp_ -= imm; // texa rezerv anum local popoxakanneri hamar
        break;

    default:
        break;
    }
}
int VirtualMachine::run() {
    int steps = 0;
    const int maxSteps = 500000; // maximum qayleri qanak

    while (running_ && ip_ < mem_.size()) {

        if (++steps > maxSteps) {
            std::cerr << "VM error: step limit exceeded at ip=" << ip_ << "\n"; // protection infinite loop-ic
            return 1;
        }

        step(); // execute-a anum mek instruction
    }

    return regs_[1]; // veradardznuma result-y
}