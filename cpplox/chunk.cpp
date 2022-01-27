#include "chunk.hpp"
#include "memory.hpp"
#include "valuearray.hpp"

Chunk::Chunk() {
    count = 0;
    lineCount = 0;
    vm = nullptr;
}

Chunk::Chunk(VM* vm) : constants(vm) {
    count = 0;
    lineCount = 0;
    this->vm = vm;
}


void Chunk::writeChunk(uint8_t byte, int line) {
    code.push_back(byte);
    count++;
    
    if(this->lineCount > 0 &&
       line == lines.back().line) {
        //still on the same line
        return;
    }
    
    Line linestart;
    linestart.line = line;
    linestart.start = this->count - 1;
    
    lines.push_back(linestart);
    this->lineCount++;
}

int Chunk::addConstant(Value value) {
    this->constants.writeValueArray(value);
    return (int)this->constants.count - 1;
}

int Chunk::getLine(size_t index) {
    int left = 0;
    int right = (int)this->lineCount - 1;
    while(left < right - 1) {
        int mid = left + (right - left) / 2;
        
        if(this->lines[mid].start == index) {
            return (int)this->lines[mid].line;
        } else if (this->lines[mid].start < index) {
            left = mid;
        } else {
            right = mid - 1;
        }
    }
    
    return this->lines[right].start > index ? (int)this->lines[left].line : (int)this->lines[right].line;
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
