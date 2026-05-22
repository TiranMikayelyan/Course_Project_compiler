#include "exec_format.h" 
#include "isa.h" 
#include <fstream> // file-eri het ashxatanqi hamar
#include <stdexcept> // exception-neri hamar

#pragma pack(push, 1) // memory alignment-y 1 byte a sarqum

struct FileHeader {

    uint32_t magic; // file-i signature
    uint32_t headerSize; // header-i chapy
    uint32_t sectionCount; // section-neri qanaky
};

struct PackedSectionHeader {

    uint32_t type; // section-i tesaky
    uint32_t size; // section-i chapy
    uint32_t offset; // section-i texy file-i mej
};

#pragma pack(pop) // veradardznuma normal alignment

bool writeExecFile(const std::string& path, const CompiledProgram& prog) {

    // 3 section-i hamar memory enq pahum
    std::vector<PackedSectionHeader> sections(3);

    // file-i header-i chapy
    uint32_t hdrSize =
        static_cast<uint32_t>(
            sizeof(FileHeader) +
            3 * sizeof(PackedSectionHeader)
            );

    uint32_t off = hdrSize;

    // code section
    sections[0].type =
        static_cast<uint32_t>(SectionType::Code);

    sections[0].size =
        static_cast<uint32_t>(
            prog.code.size() * isa::WORD_SIZE
            );

    sections[0].offset = off;

    off += sections[0].size;

    // data section
    sections[1].type =
        static_cast<uint32_t>(SectionType::Data);

    sections[1].size =
        static_cast<uint32_t>(
            prog.data.size() * isa::WORD_SIZE
            );

    sections[1].offset = off;

    off += sections[1].size;

    // symbol table section
    sections[2].type =
        static_cast<uint32_t>(SectionType::SymbolTable);

    sections[2].size =
        static_cast<uint32_t>(
            prog.functions.size() * 8
            );

    sections[2].offset = off;

    // binary file bacuma grelu hamar
    std::ofstream out(path, std::ios::binary);

    if (!out)
        return false;

    // file-i glxavor header
    FileHeader fh{
        isa::EXEC_MAGIC,
        hdrSize,
        3
    };

    // gruma header-y
    out.write(
        reinterpret_cast<const char*>(&fh),
        sizeof(fh)
    );

    // gruma section-neri header-ner
    out.write(
        reinterpret_cast<const char*>(sections.data()),
        sections.size() * sizeof(PackedSectionHeader)
    );

    // code section
    for (auto w : prog.code)

        out.write(
            reinterpret_cast<const char*>(&w),
            sizeof(w)
        );

    // data section
    for (auto d : prog.data)

        out.write(
            reinterpret_cast<const char*>(&d),
            sizeof(d)
        );

    // symbol table
    for (auto& kv : prog.functions) {

        uint32_t nameHash = 0;

        // funkciai anuni hash-i stexcum
        for (char c : kv.first)

            nameHash =
            nameHash * 31u +
            static_cast<uint8_t>(c);

        uint32_t pc = kv.second;

        // hash-i grum
        out.write(
            reinterpret_cast<const char*>(&nameHash),
            4
        );

        // pc-i grum
        out.write(
            reinterpret_cast<const char*>(&pc),
            4
        );
    }

    return true;
}

bool readExecFile(
    const std::string& path,
    std::vector<uint32_t>& code,
    std::vector<int>& data
) {

    // binary file bacuma kardalu hamar
    std::ifstream in(path, std::ios::binary);

    if (!in)
        return false;

    FileHeader fh{};

    // karduma file-i header-y
    in.read(
        reinterpret_cast<char*>(&fh),
        sizeof(fh)
    );

    // stuguma signature-y
    if (fh.magic != isa::EXEC_MAGIC)

        throw std::runtime_error(
            "bad exec signature"
        );

    // karduma section-neri header-ner
    std::vector<PackedSectionHeader>
        sections(fh.sectionCount);

    in.read(
        reinterpret_cast<char*>(sections.data()),
        fh.sectionCount * sizeof(PackedSectionHeader)
    );

    // ancnuma bolor section-nerov
    for (auto& s : sections) {

        // gnuma section-i texy
        in.seekg(s.offset);

        // code section
        if (
            s.type ==
            static_cast<uint32_t>(SectionType::Code)
            ) {

            code.resize(s.size / isa::WORD_SIZE);

            in.read(
                reinterpret_cast<char*>(code.data()),
                s.size
            );
        }

        // data section
        else if (
            s.type ==
            static_cast<uint32_t>(SectionType::Data)
            ) {

            data.resize(s.size / isa::WORD_SIZE);

            in.read(
                reinterpret_cast<char*>(data.data()),
                s.size
            );
        }
    }

    return true;
}