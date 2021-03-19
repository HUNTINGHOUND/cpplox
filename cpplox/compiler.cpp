#include "compiler.hpp"
#include <iostream>

#ifdef DEBUG_PRINT_CODE
#include "debug.hpp"
#endif


ParseRule rules[42] = {
  [TOKEN_LEFT_PAREN]    = {&Compiler::grouping, nullptr,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_RIGHT_BRACE]   = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_COMMA]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_DOT]           = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_MINUS]         = {&Compiler::unary, &Compiler::binary, PREC_TERM},
  [TOKEN_PLUS]          = {nullptr,     &Compiler::binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_SLASH]         = {nullptr,     &Compiler::binary, PREC_FACTOR},
  [TOKEN_STAR]          = {nullptr,     &Compiler::binary, PREC_FACTOR},
  [TOKEN_BANG]          = {&Compiler::unary,     nullptr,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {nullptr,     &Compiler::binary,   PREC_EQUALITY},
  [TOKEN_EQUAL]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {nullptr,     &Compiler::binary,   PREC_EQUALITY},
  [TOKEN_GREATER]       = {nullptr,     &Compiler::binary,   PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {nullptr,     &Compiler::binary,   PREC_COMPARISON},
  [TOKEN_LESS]          = {nullptr,     &Compiler::binary,   PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {nullptr,     &Compiler::binary,   PREC_COMPARISON},
  [TOKEN_QUESTION_MARK] = {nullptr,     &Compiler::condition, PREC_CONDITIONAL},
  [TOKEN_COLON]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {&Compiler::variable,     nullptr,   PREC_NONE},
  [TOKEN_STRING]        = {&Compiler::string, nullptr, PREC_NONE},
  [TOKEN_NUMBER]        = {&Compiler::number, nullptr, PREC_NONE},
  [TOKEN_AND]           = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_CLASS]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_ELSE]          = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_FALSE]         = {&Compiler::literal,     nullptr,   PREC_NONE},
  [TOKEN_FOR]           = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_FUN]           = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_IF]            = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_NUL]           = {&Compiler::literal,     nullptr,   PREC_NONE},
  [TOKEN_OR]            = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_PRINT]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_RETURN]        = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_SUPER]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_THIS]          = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_TRUE]          = {&Compiler::literal,     nullptr,   PREC_NONE},
  [TOKEN_VAR]           = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_WHILE]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_ERROR]         = {nullptr,     nullptr,   PREC_NONE},
  [TOKEN_EOF]           = {nullptr,     nullptr,   PREC_NONE},
};

Compiler::Compiler(const std::string& source, VM* vm) : scanner(source), parser(&scanner){
    this->vm = vm;
}

Parser::Parser(Scanner* scanner) : current(TOKEN_NUL, "", 0, 0, 0), previous(TOKEN_NUL, "", 0, 0, 0){
    this->scanner = scanner;
}

void Parser::errorAt(Token* token, const std::string& message) {
    if(panicMode) return;
    panicMode = true;
    std::cerr << "[line " << token->line << "] Error";
    
    if (token->type == TOKEN_EOF) {
        std::cerr << " at end";
    } else if (token->type == TOKEN_ERROR) {
        //nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->source.c_str());
    }
    
    std::cerr << ": " << message << std::endl;
    hadError = true;
}

void Parser::errorAtCurrent(const std::string& message) {
    errorAt(&this->current, message);
}

void Parser::error(const std::string& message) {
    errorAt(&this->previous, message);
}

void Parser::advance() {
    this->previous = this->current;
    
    for (;;) {
        this->current = scanner->scanToken();
        if (this->current.type != TOKEN_ERROR) break;
        
        errorAtCurrent(this->current.source);
    }
}

void Parser::consume(TokenType type, const std::string& message) {
    if (current.type == type) {
        advance();
        return;
    }
    
    errorAtCurrent(message);
}

bool Parser::check(TokenType type) {
    return current.type == type;
}

Chunk* Compiler::currentChunk() {
    return compilingChunk;
}

void Compiler::emitByte(uint8_t byte) {
    currentChunk()->writeChunk(byte, parser.previous.line);
}

void Compiler::endCompiler() {
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if(!parser.hadError) {
        Disassembler::disassembleChunk(currentChunk(), "code");
    }
#endif
}

void Compiler::condition(bool canAssign) {
    parsePrecedence(PREC_CONDITIONAL);
    
    parser.consume(TOKEN_COLON, "Expect ':' after conditional operator");
    
    parsePrecedence(PREC_ASSIGNMENT);
    emitByte(OP_CONDITIONAL);
}

void Compiler::emitReturn() {
    emitByte(OP_RETURN);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::number(bool canAssign) {
    double value = atof(parser.previous.source.c_str());
    emitConstant(Value::number_val(value));
}

void Compiler::emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
    int constant = currentChunk()->addConstant(value);
    if (constant > currentChunk()->constants.values.max_size()) {
        parser.error("Too many constants in one chunk.");
        return 0;
    }
    
    return (uint8_t)constant;
}

void Compiler::grouping(bool canAssign) {
    expression();
    parser.consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

void Compiler::unary(bool canAssign) {
    TokenType operatorType = parser.previous.type;
    
    parsePrecedence(PREC_UNARY);
    
    switch (operatorType) {
        case TOKEN_BANG:
            emitByte(OP_NOT);
            break;
        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;
            
        default:
            return; //unreachable
    }
}

void Compiler::string(bool canAssign) {
    ObjString* string = ObjString::copyString(vm, parser.previous.source.c_str() + 1,
                                              parser.previous.length - 2);
    
    emitConstant(Value::obj_val(string));
}

void Compiler::binary(bool canAssign) {
    TokenType operatorType = parser.previous.type;
    
    ParseRule* rule = ParseRule::getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));
    
    switch(operatorType) {
        case TOKEN_BANG_EQUAL:
            emitBytes(OP_EQUAL, OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL);
            break;
        case TOKEN_GREATER:
            emitByte(OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            emitBytes(OP_LESS, OP_NOT);
            break;
        case TOKEN_LESS:
            emitByte(OP_LESS);
            break;
        case TOKEN_LESS_EQUAL:
            emitBytes(OP_GREATER, OP_NOT);
            break;
        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;
        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;
            
        default:
            return; //unreachable
    }
}


void Compiler::parsePrecedence(Precedence precedence) {
    parser.advance();
    ParseFn prefixRule = ParseRule::getRule(parser.previous.type)->prefix;
    if (prefixRule == nullptr) {
        parser.error("Expect Expression.");
        return;
    }
    
    bool canAssign = precedence <= PREC_ASSIGNMENT;
    std::invoke(prefixRule, *this, canAssign);
    
    while (precedence <= ParseRule::getRule(parser.current.type)->precedence) {
        parser.advance();
        ParseFn infixRule = ParseRule::getRule(parser.previous.type)->infix;
        std::invoke(infixRule, *this, canAssign);
    }
    
    if (canAssign && match(TOKEN_EQUAL)) {
        parser.error("Invalid assignment target.");
    }
}

void Compiler::literal(bool canAssign) {
    switch(parser.previous.type) {
        case TOKEN_FALSE:
            emitByte(OP_FALSE);
            break;
        case TOKEN_NUL:
            emitByte(OP_NUL);
            break;
        case TOKEN_TRUE:
            emitByte(OP_TRUE);
            break;
            
        default:
            return; //unreachable
    }
}

void Compiler::expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

bool Compiler::compile(Chunk* chunk) {
    compilingChunk = chunk;
    
    parser.hadError = false;
    parser.panicMode = false;
    
    parser.advance();
    
    while(!match(TOKEN_EOF)) {
        declaration();
    }
    
    endCompiler();
    return !parser.hadError;
}

ParseRule*  ParseRule::getRule(TokenType type) {
    return &rules[type];
}

void Compiler::declaration() {
    if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }
    
    if(parser.panicMode) synchronize();
}

void Compiler::statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else {
        expressionStatement();
    }
}

bool Compiler::match(TokenType type) {
    if(!parser.check(type)) return false;
    parser.advance();
    return true;
}

void Compiler::printStatement() {
    expression();
    parser.consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

void Compiler::expressionStatement() {
    expression();
    parser.consume(TOKEN_SEMICOLON, "Expect ';' after expression");
    emitByte(OP_POP);
}

void Compiler::synchronize() {
    parser.panicMode = false;
    
    while (parser.current.type != TOKEN_EOF) {
        if(parser.previous.type == TOKEN_SEMICOLON) return;
        
        switch(parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
                
            default:
                //Do nothing
                ;
        }
        
        
        parser.advance();
    }
}

void Compiler::varDeclaration() {
    uint8_t global = parseVariable("Expect variable name.");
    
    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NUL);;
    }
    
    parser.consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    
    defineVariable(global);
}

uint8_t Compiler::parseVariable(const std::string& errorMessage) {
    parser.consume(TOKEN_IDENTIFIER, errorMessage);
    return identifierConstant(&parser.previous);
}

uint8_t Compiler::identifierConstant(Token *name) {
    Value index;
    Value identifier = Value::obj_val(ObjString::copyString(vm, name->source.c_str(), name->length));
    if (vm->globalsNames.tableGet(identifier, &index)) {
        return (uint8_t)Value::as_number(index);
    }
    
    uint8_t newIndex = (uint8_t)vm->globalValue.count;
    vm->globalValue.writeValueArray(Value::empty_val());
    
    vm->globalsNames.tableSet(identifier, Value::number_val((double)newIndex));
    return newIndex;
}

void Compiler::defineVariable(uint8_t global) {
    emitBytes(OP_DEFINE_GLOBAL, global);
}

void Compiler::variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

void Compiler::namedVariable(Token name, bool canAssign) {
    uint8_t arg = identifierConstant(&name);
    
    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(OP_SET_GLOBAL, arg);
    } else {
        emitBytes(OP_GET_GLOBAL, arg);
    }
}


