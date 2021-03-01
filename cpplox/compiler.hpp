#ifndef compiler_hpp
#define compiler_hpp

#include "chunk.hpp"
#include "scanner.hpp"


class Parser{
    Token current;
    Token previous;
    Scanner* scanner;
    bool hadError;
    bool panicMode;
    
    /// Consumes the current token, if type mismatch, report error
    /// @param type type of the token to be consumed
    /// @param message error message if type mismatch
    void consume(TokenType type, const std::string& message);
    
    /// Report error at previous token
    /// @param message error message
    void error(const std::string& message);
    
    /// Report error at current token
    /// @param message error message
    void errorAtCurrent(const std::string& message);
    
    /// Report error at the token specified
    /// @param token token where the error occured
    /// @param message error message
    void errorAt(Token* token, const std::string& message);
    
    /// Advance current token, well continue when encounter error token
    void advance();
    
    
    /// Constructor
    /// @param scanner scanner for the parser
    Parser(Scanner* scanner);
    
    friend class Compiler;
    
    
};

enum Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_CONDITIONAL,// ?:
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // .()
    PREC_PRIMARY
};



class Compiler {
    
    
    /// Appending a single byte to the current chunk
    /// @param byte byte to be appended
    void emitByte(uint8_t byte);
    
    /// Appending a constant to the current chunk
    /// @param value constant to be appended
    void emitConstant(Value value);
    
    /// return the current chunk to be writen
    /// @return return the current chunk
    Chunk* currentChunk();
    
    /// end the compiling process
    void endCompiler();
    
    /// Util method for writing OP_RETURN
    void emitReturn();
    
    /// Until function for appending two bytes to a chunk
    /// @param byte1 first byte to be appended
    /// @param byte2 second byte to be appended
    void emitBytes(uint8_t byte1, uint8_t byte2);
    
    //TODO: add documentation-------------------------------
    
    void expression();
    
    void parsePrecedence(Precedence precedence);
    
    uint8_t makeConstant(Value value);
    
public:
    
    Scanner scanner;
    Parser parser;
    
    //curent chunk to be written to
    Chunk* compilingChunk = nullptr;
    
    void number();
    
    void grouping();
    
    void unary();
    
    void binary();
    
    void condition();
    
    void literal();
    
    
    /// Constructor
    /// @param source source code to be compiled
    Compiler(const std::string& source);
    
    
    /// Compile the source code passed to the constructor and write the byte code to the chunk given. Return false if parser encounters error
    /// @param chunk chunk the bytecode should be written to
    bool compile(Chunk* chunk);
};

using ParseFn = void (Compiler::*)();

struct ParseRule {
public:
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
    
    static ParseRule* getRule(TokenType type);
};


#endif /* compiler_hpp */
