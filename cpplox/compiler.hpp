#ifndef compiler_hpp
#define compiler_hpp

#include "pch.pch"
#include "chunk.hpp"
#include "scanner.hpp"
#include "vm.hpp"

enum FunctionType : short;


/// Parser class that uses a scanner to parse through the tokens.
/// This class mainly contain helper function that makes parsing easier with error recovery.
class Parser{
    Scanner* scanner;
    
    /// Report error at the token specified
    /// @param token token where the error occured
    /// @param message error message
    void errorAt(Token* token, std::string message);
    
    
public:
    Token current;
    Token previous;
    
    bool hadError;
    bool panicMode;
    
    /// Consumes the current token, if type mismatch, report error
    /// @param type type of the token to be consumed
    /// @param message error message if type mismatch
    void consume(TokenType type, std::string message);
    
    /// Report error at previous token
    /// @param message error message
    void errorAtPrevious(std::string message);
    
    /// Advance current token, will continue when encounter error token and report error. 
    void advance();
    
    /// Report error at current token
    /// @param message error message
    void errorAtCurrent(std::string message);
    
    /// Check if the current token matches with the given type.
    /// @param type Type to check against.
    bool check(TokenType type);
    
    /// Constructor
    /// @param scanner scanner for the parser
    Parser(Scanner* scanner);
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

/// Class representing a local variable.
/// A local variable contains it's own name, it's depth in terms of scope and whether or not it's constant or captured as an upvalue.
/// isCaptured will be used to close an upvalue when it's poped out of scope. 
struct Local {
    Token name;
    int depth;
    bool isConst;
    bool isCaptured;
    
    /// Constructor for local variables, makes an unnamed local variable on the stack
    Local();
};

/// Struct to indicate a break statement
struct Break {
    /// Position of the break jump
    size_t position;
    /// The depth of the break statement
    int depth;
};

/// Struct for keeping track of Upvalues.
struct Upvalue {
    /// Position of the upvalue in the stack
    uint8_t index;
    /// Track if the upvalue is local to the ENCLOSING function
    bool isLocal;
};


/// Class representing a compiler. Compiles functions (including base script) into chunks of byte code
class Compiler {
    /// Current function compiling
    ObjFunction* function;
    
    /// Enclosing compiler / functon
    Compiler* enclosing;
    
    /// Virtual machine that this compiler is compiling for
    VM* vm;
    
    /// Type of the current function that is being compiled
    FunctionType type;
    
    std::string current_source;
    
    std::unordered_set<std::string> imported_module;
    std::unordered_set<std::string> compiled_source;
    
    /// Vector of breakstatements in the current function
    std::vector<Break> breakStatements;
    
    /// Vector of upvalues in the current function
    std::vector<Upvalue> upvalues;
    
    /// Start of the inner most loop in terms of byte code, used to patch loop
    int innermostLoopStart = -1;
    /// The depth of inner most loop, use to detect whther or not current loop exists
    int innermostLoopScopeDepth = 0;
    
    
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
    ObjFunction* endCompiler();
    
    /// Util method for writing OP_RETURN
    void emitReturn();
    
    /// Until function for appending two bytes to a chunk
    /// @param byte1 first byte to be appended
    /// @param byte2 second byte to be appended
    void emitBytes(uint8_t byte1, uint8_t byte2);
    
    /// Compile an expression
    void expression();
    
    /// Parse tokens based on their precedence. Tokens with precedence higher or equal to the given precedence will be parsed recursively to simulate higher priority.
    /// If the parser's current token does not have a prefix function (It is not a valid token to start a expression), error will be thrown.
    /// The function checks the current token's precedence BEFORE calling advance on parser. In the case where precedence is higher, the uncompiled token will be at current instead of previous.
    /// @param precedence All infix with precedence higher than equal this precedence will be invoked. 
    void parsePrecedence(Precedence precedence);
    
    /// Add given value to the constant table of the chunk. Returns the index of the constant in the constant table.
    /// @param value Value to be added to the constant table
    uint8_t makeConstant(Value value);
    
    /// Handle statements. Specifically variable declaration, function delcaration, class delcaration, and normal statements.
    void compileStatement();
    
    /// Handle statements. See definition for more detail.
    void statement();
    
    /// Match given type to current token of the parser.
    /// If the match, parser will advance and return true.
    /// Else parser will not advance and return false.
    /// @param type Type to match.
    bool match(TokenType type);
    
    /// Compile print statement
    void printStatement();
    
    /// Compile normal expression statement
    void expressionStatement();
    
    /// Called during panic, skip the statement that failed to compile and relax panic.
    void synchronize();
    
    /// Handles compilation of variable delcaration.
    /// @param isConst Is this variable constant
    void varDeclaration(bool isConst);
    
    /// Parse a variable identifier on the current token of parser. Declare said identifier.
    /// @param errorMessage Error message that will be displayed incase of error.
    /// @param isConst Whether or not the declared variable will be constant
    /// @param isFunc Whether or not the declared variable is a function
    uint8_t parseVariable(const std::string& errorMessage, bool isConst, bool isFunc);
    
    /// Attempt to find a global value based on the name given.
    /// If the global value is found, return its index in the global value array.
    /// Else, make a new entry in the global value array.
    /// @param name Name of the global value to search
    /// @param isConst Whether or not the global value is constant if created.
    uint8_t globalConstant(Token* name, bool isConst);
    
    /// Add an identifier to the constant array and return its index inside the constant array.
    /// If string already exist in the constant array, return that position.
    /// @param name String to be added.
    uint8_t addIdentifierConstant(Token* name);
    
    /// Define variables.
    /// If called in scope, the previous local variable will be initialized.
    /// If called in global, the emit byte code to define global variable passed in with whatever is on the top of the stack.
    /// It is the caller's responsibility that when ever OP_DEFINE_GLOBAL is executed that the stack is not empty.
    /// It is not recommended to use this function alone unless absolutely necessary,
    /// @param global The index of the variable in the global value array.
    void defineVariable(uint8_t global);
    
    /// Compile a named variable, this function will emit the correct bytecode depending on whether or not the variable should be set or get.
    /// This function will attempt to search for variables in the following order:
    ///     1. Local variable
    ///     2. Closure variable
    ///     3. Global variable
    /// Note that due to the implmentation of global variable, a global variable can be delayed in delcaration as long as the variable is delcared before execution.
    /// @param name Name of the variable
    /// @param canAssign Whether or not this variable can be assigned
    void namedVariable(Token* name, bool canAssign);
    
    /// Increase scope of the compiler.
    void beginScope();
    
    /// Compile statment until either EOF or a right brace is encountered.
    /// Note that this function does NOT increase scope of the compiler.
    void block();
    
    /// Decrease the scope of the compiler.
    void endScope();
    
    /// Declare a local variable.
    /// Note that this function does not delcare global variable. Instead, use parseVariable.
    /// The name of the local variable will be parser->previous. In the case of duplicate name in the same scope, report error.
    /// @param isConst Whether or not the variable is constant
    void declareVariable(bool isConst);
    
    /// Add a local variable to the local variable stack.
    /// This function will not check for duplicate name. Only use this function unless absolutely neccessary and prefer declareVariable.
    /// @param name Name of the local variable
    /// @param isConst Whether or not the local variable is constant
    void addLocal(Token name, bool isConst);
    
    /// Resolve a local variable.
    /// @param name Name of the local variable to be found.
    /// @return The index of the return value. If the local variable can't be found, return -1.
    int resolveLocal(Token* name);
    
    /// Mark the last local variable as initialized by marking its depth.
    void markInitialized();
    
    /// Parse and compile an if statement.
    /// This will compile its conditional statement, else if, and else statement.
    void ifStatement();
    
    /// Emit a jump related byte code and related jump offset.
    /// The jump offset will be set at the maximum when unpatched.
    /// Note that any bytecode instruction not jump related to jump instruction will break the bytecodes.
    /// @param instruction Jump instruction to be emitted
    /// @return The position of the offset in the bytecode, can be passed to patchJump
    size_t emitJump(uint8_t instruction);
    
    /// Patch the jump offset to the current bytecode position.
    /// Note that the difference cannot be longer than max of uint16.
    /// @param offset Position of the jump offset to be patched
    void patchJump(size_t offset);
    
    /// Parse and compile a while statement.
    void whileStatement();
    
    /// Emit a OP_LOOP byte code and the proper offsets.
    /// @param loopStart Index of the bytecode where the loop starts
    void emitLoop(int loopStart);
    
    /// Parse and compile a for statement.
    void forStatement();
    
    /// Parse and compile an import statement.
    void importStatement();
    
    /// Parse and compile a continue statement
    void continueStatement();
    
    /// Parse and compile a break statement
    void breakStatement();
    
    /// Patch break statements that have depth more than or equal to the current loop depth.
    /// Once a break statement is patched, it will be popped out of the break statement stack.
    void patchBreaks();
    
    /// Parse and compile a switch statement
    void switchStatement();
    
    /// Parse and compile a function declaration
    void funDeclaration();
    
    /// Parse and compile the function body.
    ///
    /// A new compiler and chunk is created for the function and the current function will be set as the enclosing functon.
    /// @param type The type of the function to be compiled
    void _function(FunctionType type);
    
    /// Parse and compile an argument list of a function
    uint8_t argumentList();
    
    /// Parse a return statement.
    ///
    /// When no variable is returned, an OP_NUL will be emited to be treated as a NULL value.
    void returnStatement();
    
    /// Resolve upvalue.
    /// 
    /// The compiler will look into enclosing compiler and attempt to find a upvalue.
    /// This function will walk up the compiler chain to find a variable with the given name.
    /// The base case will either be if there is no enclosing function, or if the current function if a TYPE_IMPORT type.
    ///
    /// If the variable is found, it will be added to the upvalue list of the current function with isLocal set to true. The local index will then be passed down the chain and added to each calling compiler.
    /// @param name The name the function is looking for.
    /// @return The position of the upvalue in the upvalue list if found, else return -1
    int resolveUpvalue(Token* name);
    
    /// Add a upvalue to the compiler's upvalue list
    ///
    /// The function will walk through the upvalue list to look if given upvalue was already capture.
    ///
    /// If true, return the index of the already captured value. If false, return the inserted index (end of the list).
    ///
    /// A global variable would never be captured as a upvalue
    /// @param index Index of the upvalue, depending on isLocal, index could refer to local variable index or upvalue index of the enclosing compiler.
    /// @param isLocal Whether or not the upvalue is local to the current compiler.
    /// @return The index of the inserted or found value in the upvalue list.
    int addUpvalue(uint8_t index, bool isLocal);
    
    /// Parse and compile a class declaration.
    /// This method will compile the class body and add the super class is necessary.
    void classDeclaration();
    
    /// Parse and compile a delete statement.
    /// Delete statement removes property of a class instance.
    void delStatement();
    
    /// Parse and compile a method.
    /// Very simmilar to _function() except for methods.
    void method();
    
public:
    
    Scanner* scanner;
    Parser* parser;
    
    std::vector<Local> locals;
    int localCount;
    int scopeDepth;
    
    Table stringConstants;
    
    
    /// Parse the parser->previous token and compile to number.
    /// Emit a OP_CONSTANT as well.
    /// @param canAssign Not used
    void number(bool canAssign);
    
    /// Break the normal precedence execution and start a brand new parse.
    /// @param canAssign Not used
    void grouping(bool canAssign);
    
    /// Parse unary expression like ! or -.
    /// Look at the parsing rule table for details.
    /// @param canAssign Not used
    void unary(bool canAssign);
    
    /// Parsing and compiling a binary expression.
    /// This function assumes that the left part of the binary expression has been parsed and compiled already.
    /// The function will compile the right part with the correct precedence.
    /// @param canAssign Not used
    void binary(bool canAssign);
    
    /// Parse a conditional statement (? and :)
    /// @param canAssign Not used
    void condition(bool canAssign);
    
    /// Parse literal token such as true, false, and nul
    /// @param canAssign Not used
    void literal(bool canAssign);
    
    /// Parse and generate a string object.
    /// In the case where the string is already created, intern the string.
    /// @param canAssign Not used
    void string(bool canAssign);
    
    /// Parse and compile a identifier. Essentially calls namedVariable on the previous token.
    /// @param canAssign Passed to namedVariable
    void variable(bool canAssign);
    
    /// Parse and compile an and statement.
    /// @param canAssign Not used
    void _and(bool canAssign);
    
    /// Parse and compile an or statement
    /// @param canAssign Not used
    void _or(bool canAssign);
    
    /// Parse and compile a call.
    /// This method compile the argument list and emit the OP_CALL bytecode.
    /// @param canAssign Not used.
    void call(bool canAssign);
    
    /// Parse a . symbol.
    /// It will also parse identifier after the dot and determine the appropriate bytecode to emit.
    /// @param canAssign Not used
    void dot(bool canAssign);
    
    /// Parse this keyword.
    /// Worth noting that "this" is located at the 0 location at the local variable stack of the call frame (usually reserved for the function themselve for non-method functions).
    /// @param canAssign Not used
    void _this(bool canAssign);
    
    /// Parse super.
    /// Note that super is upvalue (super class) captured by the method that uses it.
    /// @param canAssign Not used
    void super(bool canAssign);
    
    /// Parse random access operator.
    /// @param canAssign Not used
    void randomAccess(bool canAssign);
    
    /// Parse and compile range expression. 
    /// @param canAssign Not used
    void steps(bool canAssign);
    
    /// Move up the enclosing list and mark all the compiler for the garbage collector.
    void markCompilerRoots();
    
    /// Constructor for the bytecode compiler
    /// @param vm The pointer to the virtual machine which will read byte codes generated by the compiler
    /// @param type Type of the function which the compiler will be compiling for
    /// @param enclosing The pointer to the outter compiler enclosing this one (used for compiling methods and functions)
    /// @param scanner The pointer to the scanner which the compiler will read from
    /// @param parser The pointer to the parser which the compiler will read from
    Compiler(VM* vm, FunctionType type, Compiler* enclosing, Scanner* scanner, Parser* parser, std::string& current_source);
    
    
    /// Compile a given source code and return an ObjFunction pointer containing the compiled function. Note that the compiled function is the "<script>"  as this method is only called by the VM.
    /// @param src Source code to be compiled
    ObjFunction* compile(const std::string& src);
};

using ParseFn = void (Compiler::*)(bool canAssign);

/// Class to prepresent the parsing rules of a token.
/// Includes the rule if token is encoutered as a prefix, infix, and the precedence of the token.
struct ParseRule {
public:
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
    
    static ParseRule* getRule(TokenType type);
};

/// Compiler that represents a class. Used to determine whether or not the current class is nested and whether or not it has a super class.
class ClassCompiler {
public:
    ClassCompiler* enclosing;
    bool hasSuperclass;
};

#endif /* compiler_hpp */
