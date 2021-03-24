#ifndef compiler_hpp
#define compiler_hpp

#include "chunk.hpp"
#include "scanner.hpp"
#include "vm.hpp"


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
    
    bool check(TokenType type);
    
    
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

struct Local {
    Token name;
    int depth;
    bool isConst = false;
    Local();
};

class Compiler {
    
    VM* vm;
    
    
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
    
    void declaration();
    
    void statement();
    
    bool match(TokenType type);
    
    void printStatement();
    
    void expressionStatement();
    
    void synchronize();
    
    void varDeclaration(bool isConst);
    
    uint8_t parseVariable(const std::string& errorMessage, bool isConst);
    
    uint8_t identifierConstant(Token* name, bool isConst);
    
    void defineVariable(uint8_t global);
    
    void namedVariable(Token name, bool canAssign);
    
    void beginScope();
    
    void block();
    
    void endScope();
    
    void declareVariable(bool isConst);
    
    void addLocal(Token name, bool isConst);
    
    int resolveLocal(Token* name);
    
    void markInitialized();
    
    void ifStatement();
    
    int emitJump(uint8_t instruction);
    
    void patchJump(int offset);
    
    void whileStatement();
    
    void emitLoop(int loopStart);
    
public:
    
    Scanner scanner;
    Parser parser;
    
    std::vector<Local> locals;
    int localCount;
    int scopeDepth;
    
    //curent chunk to be written to
    Chunk* compilingChunk = nullptr;
    
    void number(bool canAssign);
    
    void grouping(bool canAssign);
    
    void unary(bool canAssign);
    
    void binary(bool canAssign);
    
    void condition(bool canAssign);
    
    void literal(bool canAssign);
    
    void string(bool canAssign);
    
    void variable(bool canAssign);
    
    void _and(bool canAssign);
    
    void _or(bool canAssign);
    
    
    /// Constructor
    /// @param source source code to be compiled
    Compiler(const std::string& source, VM* vm);
    
    
    /// Compile the source code passed to the constructor and write the byte code to the chunk given. Return false if parser encounters error
    /// @param chunk chunk the bytecode should be written to
    bool compile(Chunk* chunk);
};

using ParseFn = void (Compiler::*)(bool canAssign);

struct ParseRule {
public:
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
    
    static ParseRule* getRule(TokenType type);
};


#endif /* compiler_hpp */
