#ifndef chunk_h
#define chunk_h

#include "value.hpp"
#include <vector>

enum OpCode {
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_RETURN,
    OP_NOT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NUL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_CONDITIONAL,
    OP_PRINT,
    OP_POP,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_SET_LOCAL,
    OP_GET_LOCAL,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_LOOP,
    OP_DUP,
    OP_CALL
};

//Data structure that represent a line in source code
struct Line{
    int start;
    int line;
};

class Chunk {
    
public:
    
    //current count of bytes in code
    size_t count;
    size_t capacity;
    //current count of lines(Data structure) in lines
    size_t lineCount;
    size_t lineCapacity;
    
    uint8_t* code;
    Line* lines;
    
    //constants each chunk keeps
    ValueArray constants;
    
    //adds a constant to the constant vector return position added
    int addConstant(Value value);
    
    Chunk();
    
    //free chunk from memory
    void freeChunk();
    
    //write operation to chunk
    void writeChunk(uint8_t byte, int line);
    
    //get which line in the source code the operation is compiled from, error report purpose
    int getLine(size_t index);
    
    //write a constant to the chunk and it location to the byte code
    void writeConstant(Value value, int line);
};


#endif /* chunk_h */
