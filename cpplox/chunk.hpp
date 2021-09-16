#ifndef chunk_h
#define chunk_h

#include "pch.pch"
#include "valuearray.hpp"

#ifdef NANBOXING
#include "nanvalue.hpp"
#else
#include "value.hpp"
#endif


class VM;
class ValueArray;

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
    OP_JUMP_IF_EMPTY,
    OP_JUMP,
    OP_LOOP,
    OP_DUP,
    OP_CALL,
    OP_CLOSURE,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_CLOSE_UPVALUE,
    OP_CLASS,
    OP_SET_PROPERTY,
    OP_GET_PROPERTY,
    OP_DEL,
    OP_METHOD,
    OP_INVOKE,
    OP_INHERIT,
    OP_GET_SUPER,
    OP_SUPER_INVOKE,
    OP_RANDOM_ACCESS,
    OP_COLLECTION,
    OP_RANGE
};

//Data structure that represent a line in source code
struct Line{
    size_t start;
    size_t line;
};

class Chunk {
    
    VM* vm;
    
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
    
    
    /// Adds a constant to the constant vector
    /// @param value constant to be added
    /// @return position added
    int addConstant(Value value);
    
    Chunk(VM* vm);
    
    /// Free the chunk from memroy
    void freeChunk();
    
    
    /// Write bytecode to chunk
    /// @param byte The bytecode to be writen
    /// @param line The line from source code the corresponding bytecode comes from. Used for debug purpose
    void writeChunk(uint8_t byte, int line);
    
    
    /// Using binary search to find the line a bytecode is corresponding to
    /// @param index the index of the bytecode
    /// @return the line of the source code that the bytecode came from
    int getLine(size_t index);
    
    //write a constant to the chunk and it location to the byte code
    void writeConstant(Value value, int line);
};


#endif /* chunk_h */
