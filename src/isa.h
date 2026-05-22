#pragma once 

#include <cstdint>

// 32-bit instruction-i strukturan 
// [ opcode:8 | rd:4 | rs1:4 | rs2:4 | imm12:12 ]
// imm12-y petqa sign-extend lini ogtagorcelis
//// instruction set architecture
namespace isa {

    enum Op : uint8_t {

        NOP = 0, // voch mi gorts chi anum

        MOV_REG = 1, // register-ic register texapoxum
        MOV_IMM = 2, // immediate arjeq -> register

        LOAD_DATA = 3, // data memory-ic bernel
        STORE_DATA = 4, // data memory mej pahpanel

        LOAD_LOCAL = 5, // local stack-ic bernel
        STORE_LOCAL = 6, // local stack mej pahpanel

        ADD = 10, // gumarum
        SUB = 11, // hanum
        MUL = 12, // bazmapatkum
        DIV = 13, // bajanum

        CMP = 20, // hamematutyun

        BR_EQ = 30, // jump ete havasara
        BR_NE = 31, // jump ete havasar che
        BR_LT = 32, // jump ete poqra
        BR_GT = 33, // jump ete meca
        BR_LTE = 34, // jump ete poqr kam havasara
        BR_GTE = 35, // jump ete mec kam havasara

        JMP = 40, // anpayman jump

        PUSH = 50, // stack-i mej push
        POP = 51, // stack-ic pop

        CALL = 60, // funkciai kanch
        RET = 61, // funkciaic durs gal

        PRINT = 70, // arjeqi tpum
        INPUT = 71, // mutq kardu
        EXIT = 72, // cragri avart

        ALLOC_STACK = 73, // stack-i tex hatkacnel
        PUSH_BP = 74, // BP pahpanel
        PUSH_NEXT_PC = 75, // hajord instruction-i hascen pahpanel
    };

    constexpr int WORD_SIZE = 4; // mek instruction-i chap
    constexpr int REG_COUNT = 16; // register-neri qanak
    constexpr int MEM_SIZE = 65536; // yndhanur memory-i chap
    constexpr int STACK_SIZE = 16384; // stack-i chap

    constexpr int STACK_TOP = MEM_SIZE; // stack-i verjin hascen
    constexpr int STACK_BASE = STACK_TOP - STACK_SIZE; // stack-i skizby

    constexpr uint32_t EXEC_MAGIC =
        0x45584B21u; // executable file-i signature

    inline uint32_t encode(
        uint8_t op,
        uint8_t rd,
        uint8_t rs1,
        uint8_t rs2,
        int16_t imm12
    ) {

        // stexcuma 32-bit instruction

        uint32_t w =
            static_cast<uint32_t>(op) << 24;

        w |=
            (static_cast<uint32_t>(rd) & 0xF)
            << 20;

        w |=
            (static_cast<uint32_t>(rs1) & 0xF)
            << 16;

        w |=
            (static_cast<uint32_t>(rs2) & 0xF)
            << 12;

        w |=
            (static_cast<uint32_t>(imm12) & 0xFFF);

        return w;
    }

    inline uint8_t opcode(uint32_t w) {

        // instruction-ic opcode-y hanuma
        return static_cast<uint8_t>(w >> 24);
    }

    inline uint8_t rd(uint32_t w) {

        // rd field-y hanuma
        return static_cast<uint8_t>((w >> 20) & 0xF);
    }

    inline uint8_t rs1(uint32_t w) {

        // rs1 field-y hanuma
        return static_cast<uint8_t>((w >> 16) & 0xF);
    }

    inline uint8_t rs2(uint32_t w) {

        // rs2 field-y hanuma
        return static_cast<uint8_t>((w >> 12) & 0xF);
    }

    inline int16_t imm12(uint32_t w) {

        // imm12-y hanuma instruction-ic
        int16_t v =
            static_cast<int16_t>(w & 0xFFF);

        // sign extension
        if (v & 0x800)
            v |= static_cast<int16_t>(0xF000);

        return v;
    }

}