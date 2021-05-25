#ifndef debug_h
#define debug_h

#include "chunk.hpp"
#include "vm.hpp"

class Disassembler {
    
    
    /// Print out simple instructions, return updated offset
    /// @param name name of the instruction
    /// @param offset offset for iterating purposes
    static int simpleInstruction(const std::string& name, int offset) ;
    
    
    /// Print out OP_CONSTANT_LONG instructions, return updated offset
    /// @param name name of the instruction
    /// @param chunk chunk where is instruction is stored
    /// @param offset offset for iterating purposes
    static int constantLongInstruction(const std::string& name, Chunk* chunk, int offset);
    
    
    /// Print out OP_CONSTANT
    /// @param name name of the instruction
    /// @param chunk chunk where the instruction is stored
    /// @param offset offset for iterating purposes
    static int constantInstruction(const std::string& name, Chunk* chunk, int offset);
    
    static int globalVarInstruction(const std::string& name, Chunk* chunk, VM* vm, int offset);
    
    static int byteInstruction(const std::string& name, Chunk* chunk, int offset);
    
    static int jumpInstruction(const std::string& name, int sign, Chunk* chunk, int offset);
    
    static int invokeInstruction(const std::string& name, Chunk* chunk, int offset);
    
public:
    
    
    /// Disassemble specified OP instruction, return the new offset. This offset is usually the original value plus the length of the previous instruction.
    /// @param chunk The chunk the instruction is in.
    /// @param vm The virtual machine running this method.
    /// @param offset Offset from the start of the chunk which specify the location of the instruction.
    static int disassembleInstruction(Chunk* chunk, VM* vm, int offset);
    
    
    /// Disassemble an entire chunk.
    /// @param chunk The chunk that needs to be dissassembled.
    /// @param vm The virutal machine running this method.
    /// @param name Name of the chunk (for printing purposes only, this does not effect the dissassemble process).
    static void disassembleChunk(Chunk* chunk, VM* vm, const std::string& name);
};

#endif /* debug_h */
