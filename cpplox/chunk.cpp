

#include <cstdlib>

#include "chunk.hpp"
#include "memory.hpp"

Chunk::Chunk(VM* vm) : constants(vm) {
    this->count = 0;
    this->lineCount = 0;
    this->capacity = 0;
    this->lineCapacity = 0;
    this->vm = vm;
}

void Chunk::freeChunk() {
    free_array<uint8_t>(this->code, this->capacity);
    free_array<Line>(this->lines, this->capacity);
    this->constants.freeValueArray();
}

void Chunk::writeChunk(uint8_t byte, int line) {
    if (this->capacity == this->count) {
        size_t newCapa = grow_capacity(this->capacity);
        code = grow_array<uint8_t>(this->code, this->capacity ,newCapa, vm);
        this->capacity = newCapa;
    }
    
    this->code[this->count] = byte;
    this->count++;
    
    if(this->lineCount > 0 &&
       line == this->lines[this->lineCount - 1].line) {
        //still on the same line
        return;
    }
    
    if(this->lineCapacity == this->lineCount) {
        size_t newlineCapa = grow_capacity(this->lineCapacity);
        lines = grow_array<Line>(this->lines, this->lineCapacity, newlineCapa, vm);
        lineCapacity = newlineCapa;
    }
    Line linestart;
    linestart.line = line;
    linestart.start = this->count - 1;
    
    this->lines[this->lineCount] = linestart;
    this->lineCount++;
}

int Chunk::addConstant(Value value) {
    this->constants.writeValueArray(value);
    return this->constants.count - 1;
}

int Chunk::getLine(size_t index) {
    int left = 0;
    int right = this->lineCount - 1;
    while(left < right - 1) {
        int mid = left + (right - left) / 2;
        
        if(this->lines[mid].start == index) {
            return this->lines[mid].line;
        } else if (this->lines[mid].start < index) {
            left = mid;
        } else {
            right = mid - 1;
        }
    }
    
    return this->lines[right].start > index ? this->lines[left].line : this->lines[right].line;
}

void Chunk::writeConstant(Value value, int line) {
    int constant = addConstant(value);
    if(constant < 256) {
        writeChunk(OP_CONSTANT, line);
        writeChunk((uint8_t)constant, line);
    } else {
        writeChunk(OP_CONSTANT_LONG, line);
        writeChunk((uint8_t)(constant & 0xff), line);
        writeChunk((uint8_t)(constant >> 8 & 0xff), line);
        writeChunk((uint8_t)(constant >> 16 & 0xff), line);
        writeChunk((uint8_t)(constant >> 24 & 0xff), line);
    }
}
