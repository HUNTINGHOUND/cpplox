
#include <iostream>
#include <iomanip>
#include "debug.hpp"
#include "value.hpp"

int Disassembler::simpleInstruction(const char* name, int offset) {
    std::cout << name << std::endl;
    return offset + 1;
}

int Disassembler::constantInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t constant = chunk->code[offset + 1];
    std::cout << std::left << std::setw(16) << name << " " << std::right << std::setw(4) << (int)constant << " '";
    ValueArray::printValue(chunk->constants.values[constant]);
    std::cout << "'" << std::endl;
    return offset + 2;
}

int Disassembler::constantLongInstruction(const char* name, Chunk* chunk, int offset) {
    uint32_t constant = chunk->code[offset + 1];
    constant |= chunk->code[offset + 2] << 8;
    constant |= chunk->code[offset + 3] << 16;
    constant |= chunk->code[offset + 4] << 24;
    std::cout << std::left << std::setw(16) << name << " " << std::right << std::setw(4) << (int)constant << " '";
    ValueArray::printValue(chunk->constants.values[constant]);
    std::cout << "'" << std::endl;
    return offset + 5;
}

void Disassembler::disassembleChunk(Chunk* chunk, const char* name) {
    std::cout << "== " << name << " ==" << std::endl;
    
    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

int Disassembler::disassembleInstruction(Chunk* chunk, int offset) {
    std::cout << std::setw(4) << offset << " ";
    if(offset > 0 && chunk->getLine(offset) == chunk->getLine(offset - 1)) {
        std::cout << "   | ";
    } else {
        std::cout << std::setw(4) << chunk->getLine(offset) << " ";
    }
    
    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OP_CONSTANT_LONG:
            return constantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OP_NUL:
            return simpleInstruction("OP_NUL", offset);
        case OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);
        default:
            std::cout << "Unknown instruction " << instruction <<std::endl;
            return offset + 1;
    }
}
