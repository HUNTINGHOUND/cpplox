#ifndef debug_h
#define debug_h

#include "chunk.hpp"
#include "vm.hpp"

class Disassembler {
    
    
    /// Print out simple instructions, return updated offset
    /// @param name name of the instruction
    /// @param offset offset for iterating purposes
    static int simpleInstruction(const char* name, int offset) ;
    
    
    /// Print out OP_CONSTANT_LONG instructions, return updated offset
    /// @param name name of the instruction
    /// @param chunk chunk where is instruction is stored
    /// @param offset offset for iterating purposes
    static int constantLongInstruction(const char* name, Chunk* chunk, int offset);
    
    
    /// Print out OP_CONSTANT
    /// @param name name of the instruction
    /// @param chunk chunk where the instruction is stored
    /// @param offset offset for iterating purposes
    static int constantInstruction(const char* name, Chunk* chunk, int offset);
    
    static int globalVarInstruction(const char* name, Chunk* chunk, VM* vm, int offset);
    
    static int byteInstruction(const char* name, Chunk* chunk, int offset);
    
public:
    
    //disassemble a particular instruction
    static int disassembleInstruction(Chunk* chunk, VM* vm, int offset);
    
    //disassemble a chunk
    static void disassembleChunk(Chunk* chunk, VM* vm, const char* name);
};

#endif /* debug_h */
