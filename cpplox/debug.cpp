
#include <iostream>
#include <iomanip>
#include "debug.hpp"
#include "value.hpp"

int Disassembler::simpleInstruction(const std::string& name, int offset) {
    std::cout << name << std::endl;
    return offset + 1;
}

int Disassembler::constantInstruction(const std::string& name, Chunk* chunk, int offset) {
    uint8_t constant = chunk->code[offset + 1];
    std::cout << std::left << std::setw(16) << name << " " << std::right << std::setw(4) << (int)constant << " '";
    Value::printValue(chunk->constants.values[constant]);
    std::cout << "'" << std::endl;
    return offset + 2;
}

int Disassembler::constantLongInstruction(const std::string& name, Chunk* chunk, int offset) {
    uint32_t constant = chunk->code[offset + 1];
    constant |= chunk->code[offset + 2] << 8;
    constant |= chunk->code[offset + 3] << 16;
    constant |= chunk->code[offset + 4] << 24;
    std::cout << std::left << std::setw(16) << name << " " << std::right << std::setw(4) << (int)constant << " '";
    Value::printValue(chunk->constants.values[constant]);
    std::cout << "'" << std::endl;
    return offset + 5;
}

int Disassembler::globalVarInstruction(const std::string& name, Chunk* chunk, VM *vm, int offset) {
    uint8_t constant = chunk->code[offset + 1];
    std::cout << std::left << std::setw(16) << name << " " << std::right << std::setw(4) << (int)constant << std::endl;
    return offset + 2;
}

int Disassembler::byteInstruction(const std::string& name, Chunk *chunk, int offset) {
    uint8_t slot = chunk->code[offset + 1];
    std::cout << std::left << std::setw(16) << name << " " << std::right << std::setw(4) << (int)slot << std::endl;
    return offset + 2;
}

int Disassembler::jumpInstruction(const std::string& name, int sign, Chunk *chunk, int offset) {
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];
    std::cout << std::left << std::setw(16) << name << " " << std::right << std::setw(4) << (int)offset << " "
    << " -> " << offset + 3 + sign * jump << std::endl;
    return offset + 3;
}

int Disassembler::invokeInstruction(const std::string &name,Chunk *chunk, int offset) {
    uint8_t constant = chunk->code[offset + 1];
    uint8_t argCount = chunk->code[offset + 2];
    std::cout << std::left << std::setw(16) << name << " (" << std::setw(0) << argCount << " args) '" << std::setw(4) << constant;
    Value::printValue(chunk->constants.values[constant]);
    std::cout << std::setw(0) << "'" << std::endl;
    return offset + 3;
}

void Disassembler::disassembleChunk(Chunk* chunk, VM* vm, const std::string& name) {
    std::cout << "== " << name << " ==" << std::endl;
    
    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, vm, offset);
    }
}


int Disassembler::disassembleInstruction(Chunk* chunk, VM* vm, int offset) {
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
        case OP_NOT:
            return simpleInstruction("OP_NOT", offset);
        case OP_EQUAL:
            return simpleInstruction("OP_EQUAL", offset);
        case OP_GREATER:
            return simpleInstruction("OP_GREATER", offset);
        case OP_LESS:
            return simpleInstruction("OP_LESS", offset);
        case OP_CONDITIONAL:
            return simpleInstruction("OP_CONDITIONAL", offset);
        case OP_PRINT:
            return simpleInstruction("OP_PRINT", offset);
        case OP_POP:
            return simpleInstruction("OP_POP", offset);
        case OP_DEFINE_GLOBAL:
            return globalVarInstruction("OP_DEFINE_GLOBAL", chunk, vm, offset);
        case OP_GET_GLOBAL:
            return globalVarInstruction("OP_GET_GLOBAL", chunk, vm, offset);
        case OP_SET_GLOBAL:
            return globalVarInstruction("OP_SET_GLOBAL", chunk, vm, offset);
        case OP_GET_LOCAL:
            return byteInstruction("OP_GET_LOCAL", chunk, offset);
        case OP_SET_LOCAL:
            return byteInstruction("OP_SET_LOCAL", chunk, offset);
        case OP_JUMP:
            return jumpInstruction("OP_JUMP", 1, chunk, offset);
        case OP_JUMP_IF_FALSE:
            return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
        case OP_LOOP:
            return jumpInstruction("OP_LOOP", -1, chunk, offset);
        case OP_DUP:
            return simpleInstruction("OP_DUP", offset);
        case OP_CALL:
            return byteInstruction("OP_CALL", chunk, offset);
        case OP_GET_UPVALUE:
            return byteInstruction("OP_GET_UPVALUE", chunk, offset);
        case OP_SET_UPVALUE:
            return byteInstruction("OP_SET_UPVALUE", chunk, offset);
        case OP_CLOSURE: {
            offset++;
            uint8_t constant = chunk->code[offset];
            std::cout << std::left << std::setw(16) << "OP_CLOSURE" << std::right << std::setw(4) << constant << " ";
            Value::printValue(chunk->constants.values[constant]);
            std::cout << std::endl;
            
            ObjFunction* function = Value::as_function(
                                                       chunk->constants.values[constant]);
            for(int j = 0; j < function->upvalueCount; j++) {
                int isLocal = chunk->code[offset++];
                int index = chunk->code[offset++];
                std::cout << std::right << std::setw(4) << offset - 2;
                std::cout << "    |                     " << (isLocal ? "local" : "upvalue") << " " << index << std::endl;
            }
            return offset + 1;
        }
        case OP_CLOSE_UPVALUE:
            return simpleInstruction("OP_CLOSE_UPVALUE", offset);
        case OP_CLASS:
            return constantInstruction("OP_CLASS", chunk, offset);
        case OP_SET_PROPERTY:
            return constantInstruction("OP_SET_PROPERTY", chunk, offset);
        case OP_GET_PROPERTY:
            return constantInstruction("OP_GET_PROPERTY", chunk, offset);
        case OP_DEL:
            return constantInstruction("OP_DEL", chunk, offset);
        case OP_METHOD:
            return constantInstruction("OP_METHOD", chunk, offset);
        case OP_INVOKE:
            return invokeInstruction("OP_INVOKE", chunk, offset);
        case OP_INHERIT:
            return simpleInstruction("OP_INHERIT", offset);
        case OP_GET_SUPER:
            return constantInstruction("OP_GET_SUPER", chunk, offset);
        case OP_SUPER_INVOKE:
            return invokeInstruction("OP_SUPER_INVOKE", chunk, offset);
        default:
            std::cout << "Unknown instruction " << instruction <<std::endl;
            return offset + 1;
    }
}
