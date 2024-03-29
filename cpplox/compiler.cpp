#include "compiler.hpp"
#include "memory.hpp"
#include "debug.hpp"
#include "flags.hpp"
#include "util.hpp"
#include <filesystem>

//Table containing precedence and compiling rules for all tokens
ParseRule rules[53] = {
    [TOKEN_LEFT_PAREN]    = {&Compiler::grouping, &Compiler::call,   PREC_CALL},
    [TOKEN_RIGHT_PAREN]   = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_RIGHT_BRACE]   = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_LEFT_BRACK]    = {nullptr,     &Compiler::randomAccess,   PREC_CALL},
    [TOKEN_RIGHT_BRACK]   = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_COMMA]         = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_DOT]           = {nullptr,     &Compiler::dot,   PREC_CALL},
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
    [TOKEN_COLON]         = {nullptr,     &Compiler::steps,   PREC_CONDITIONAL},
    [TOKEN_IDENTIFIER]    = {&Compiler::variable,     nullptr,   PREC_NONE},
    [TOKEN_STRING]        = {&Compiler::string, nullptr, PREC_NONE},
    [TOKEN_NUMBER]        = {&Compiler::number, nullptr, PREC_NONE},
    [TOKEN_FLOAT]         = {&Compiler::number, nullptr, PREC_NONE},
    [TOKEN_AND]           = {nullptr,     &Compiler::_and,   PREC_AND},
    [TOKEN_CLASS]         = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_ELSE]          = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_FALSE]         = {&Compiler::literal,     nullptr,   PREC_NONE},
    [TOKEN_FOR]           = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_FUN]           = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_IF]            = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_NUL]           = {&Compiler::literal,     nullptr,   PREC_NONE},
    [TOKEN_OR]            = {nullptr,     &Compiler::_or,   PREC_OR},
    [TOKEN_PRINT]         = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_RETURN]        = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_SUPER]         = {&Compiler::super,     nullptr,   PREC_NONE},
    [TOKEN_THIS]          = {&Compiler::_this,       nullptr,   PREC_NONE},
    [TOKEN_TRUE]          = {&Compiler::literal,     nullptr,   PREC_NONE},
    [TOKEN_VAR]           = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_CONST]         = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_WHILE]         = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_CONTINUE]      = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_BREAK]         = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_SWITCH]        = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_CASE]          = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_DEFAULT]       = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_DEL]           = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_IMPORT]        = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_ERROR]         = {nullptr,     nullptr,   PREC_NONE},
    [TOKEN_EOF]           = {nullptr,     nullptr,   PREC_NONE},
};


/// Compare two identifier (variable name) token to see if they are equal
/// @param a Token to be compared
/// @param b Token to be compared
/// @return True if the tokens are equal, false otherwise
bool identifierEqual(Token* a, Token* b) {
    if (a->length != b->length) return false;
    return a->source.compare(b->source) == 0;
}

Local::Local() : name(TOKEN_NUL, "", 0, 0, 0) {
    this->depth = -1;
    this->isConst = false;
    this->isCaptured = false;
}

Compiler::Compiler(VM* vm, FunctionType type, Compiler* enclosing, Scanner* scanner, Parser* parser, const std::string& current_source) : stringConstants(vm) {
    this->enclosing = enclosing;
    
    this->scanner = scanner;
    this->parser = parser;
    
    function = nullptr;
    this->type = type;
    
    this->localCount = 0;
    this->scopeDepth = 0;
    this->vm = vm;
    
    this->current_source = current_source;
    
    vm->current = this;
    
    function = ObjFunction::newFunction(vm, type);
    
    if (type != TYPE_SCRIPT && type != TYPE_IMPORT) {
        function->name = ObjString::copyString(vm, parser->previous.source);
    } else if(type == TYPE_IMPORT) {
        function->name = ObjString::copyString(vm, "<import>");
    }
    
    if(localCount + 1 >= locals.capacity()) {
        locals.resize(locals.capacity() == 0 ? 8 : locals.capacity() * 2);
    }
    
    Local* local = &locals[localCount++];
    local->depth = 0;
    
    if (type != TYPE_FUNCTION) {
        local->name.source = "this";
        local->name.length = 4;
    } else {
        local->name.source = "";
        local->name.length = 0;
    }
}

Parser::Parser(Scanner* scanner) : current(TOKEN_NUL, "", 0, 0, 0), previous(TOKEN_NUL, "", 0, 0, 0){
    this->scanner = scanner;
}

void Parser::errorAt(Token* token, std::string message) {
    if(panicMode) return;
    panicMode = true;
    std::cerr << "[line " << token->line << "] Error";
    
    if (token->type == TOKEN_EOF) {
        std::cerr << " at end";
    } else if (token->type == TOKEN_ERROR) {
        //nothing
    } else {
        std::cerr << " at " << token->source;
    }
    
    std::cerr << ": " << message << std::endl;
    hadError = true;
}

void Parser::errorAtCurrent(std::string message) {
    errorAt(&this->current, message);
}

void Parser::errorAtPrevious(std::string message) {
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

void Parser::consume(TokenType type, std::string message) {
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
    return &function->chunk;
}

void Compiler::emitByte(uint8_t byte) {
    currentChunk()->writeChunk(byte, parser->previous.line);
}

ObjFunction* Compiler::endCompiler() {
    if(type == TYPE_SCRIPT) {
        Value main_loc;
        Value identifier = ValueOP::obj_val(ObjString::copyString(vm, "main"));
        if(vm->globalNames.tableGet(identifier, &main_loc)) {
            uint8_t main_index = static_cast<uint8_t>(ValueOP::as_number(main_loc).number.whole);
            emitBytes(OP_GET_GLOBAL, main_index);
            emitBytes(OP_CALL, 0);
            emitByte(OP_POP);
        }
    }
    
    emitReturn();
    ObjFunction* function = this->function;
    
    vm->current = enclosing;
    
    if(DEBUG_PRINT_CODE) {
        if(!parser->hadError) {
            Disassembler::disassembleChunk(currentChunk(), vm, function->name != nullptr
                                           ? function->name->chars : "<script>");
        }
    }
    
    return function;
}

void Compiler::condition(bool canAssign) {
    parsePrecedence(PREC_OR);
    
    parser->consume(TOKEN_COLON, "Expect ':' after conditional operator");
    
    parsePrecedence(PREC_ASSIGNMENT);
    emitByte(OP_CONDITIONAL);
}

void Compiler::emitReturn() {
    if(type == TYPE_INITIALIZER) {
        emitBytes(OP_GET_LOCAL, 0);
    } else {
        emitByte(OP_NUL);
    }
    emitByte(OP_RETURN);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::number(bool canAssign) {
    Number num;
    if(parser->previous.type == TOKEN_FLOAT) {
        double value = std::stod(parser->previous.source);
        num.is_float = true;
        num.number.decimal = value;
    } else {
        long long value = std::stoll(parser->previous.source);
        num.is_float = false;
        num.number.whole = value;
    }
    emitConstant(ValueOP::number_val(num));
}

void Compiler::emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
    int constant = currentChunk()->addConstant(value);
    if (constant > 256) {
        parser->errorAtPrevious("Too many constants in one chunk.");
        return 0;
    }
    
    return (uint8_t)constant;
}

void Compiler::grouping(bool canAssign) {
    expression();
    parser->consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

void Compiler::unary(bool canAssign) {
    TokenType operatorType = parser->previous.type;
    
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
    ObjString* string = ObjString::copyString(vm, parser->previous.source.substr(1, parser->previous.source.size() - 2));
    emitConstant(ValueOP::obj_val(string));
}

void Compiler::binary(bool canAssign) {
    TokenType operatorType = parser->previous.type;
    
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
    parser->advance();
    ParseFn prefixRule = ParseRule::getRule(parser->previous.type)->prefix;
    if (prefixRule == nullptr) {
        parser->errorAtPrevious("Expect Expression.");
        return;
    }
    
    bool canAssign = precedence <= PREC_ASSIGNMENT;
    std::invoke(prefixRule, *this, canAssign);
    
    while (precedence <= ParseRule::getRule(parser->current.type)->precedence) {
        parser->advance();
        ParseFn infixRule = ParseRule::getRule(parser->previous.type)->infix;
        std::invoke(infixRule, *this, canAssign);
    }
    
    if (!canAssign && match(TOKEN_EQUAL)) {
        parser->errorAtPrevious("Invalid assignment target.");
    }
}

void Compiler::literal(bool canAssign) {
    switch(parser->previous.type) {
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

ObjFunction* Compiler::compile(const std::string& src) {
    scanner->setSource(src);
    
    parser->hadError = false;
    parser->panicMode = false;
    
    parser->advance();
    
    while(!match(TOKEN_EOF)) {
        compileStatement();
    }
    
    ObjFunction* function = endCompiler();
    return parser->hadError ? nullptr : function;
}

ParseRule*  ParseRule::getRule(TokenType type) {
    return &rules[type];
}

void Compiler::compileStatement() {
    bool isConst = false;
    if (match(TOKEN_VAR) || (isConst = match(TOKEN_CONST))) {
        varDeclaration(isConst);
    } else if (match(TOKEN_FUN)) {
        funDeclaration();
    } else if (match(TOKEN_CLASS)) {
        classDeclaration();
    } else {
        statement();
    }
    
    if(parser->panicMode) synchronize();
}

void Compiler::statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else if (match(TOKEN_IF)) {
        ifStatement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else if (match(TOKEN_WHILE)){
        whileStatement();
    } else if (match(TOKEN_FOR)) {
        forStatement();
    } else if(match(TOKEN_CONTINUE)) {
        continueStatement();
    } else if(match(TOKEN_BREAK)) {
        breakStatement();
    } else if(match(TOKEN_SWITCH)) {
        switchStatement();
    } else if(match(TOKEN_RETURN)) {
        returnStatement();
    } else if(match(TOKEN_DEL)) {
        delStatement();
    } else if(match(TOKEN_IMPORT)) {
        importStatement();
    } else {
        expressionStatement();
    }
}

bool Compiler::match(TokenType type) {
    if(!parser->check(type)) return false;
    parser->advance();
    return true;
}

void Compiler::printStatement() {
    expression();
    parser->consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

void Compiler::expressionStatement() {
    expression();
    parser->consume(TOKEN_SEMICOLON, "Expect ';' after expression");
    emitByte(OP_POP);
}

void Compiler::synchronize() {
    parser->panicMode = false;
    
    while (parser->current.type != TOKEN_EOF) {
        if(parser->previous.type == TOKEN_SEMICOLON) return;
        
        switch(parser->current.type) {
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
        
        
        parser->advance();
    }
}

void Compiler::varDeclaration(bool isConst) {
    uint8_t global = parseVariable("Expect variable name.", isConst, false);
    
    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NUL);
    }
    
    parser->consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    
    defineVariable(global);
}

uint8_t Compiler::parseVariable(const std::string& errorMessage, bool isConst, bool isFunc) {
    parser->consume(TOKEN_IDENTIFIER, errorMessage);
    
    declareVariable(isConst);
    if (scopeDepth > 0) return 0;
    
    if(parser->previous.source.compare("main") == 0 && !isFunc) {
        parser->errorAtPrevious("Identifier \"main\" is reserved for global scope main function declaration.");
        return 0;
    }
    
    return globalConstant(&parser->previous, isConst);
}

uint8_t Compiler::globalConstant(Token *name, bool isConst) {
    Value index;
    Value identifier = ValueOP::obj_val(ObjString::copyString(vm, name->source));
    if (vm->globalNames.tableGet(identifier, &index)) {
        return (uint8_t)(ValueOP::as_number(index).number.whole);
    }
    
    uint8_t newIndex = (uint8_t)vm->globalValues.count;
    Value v = ValueOP::number_val(newIndex);
    ValueOP::setConst(isConst, v);
    vm->globalNames.tableSet(identifier, v);
    vm->globalValues.writeValueArray(ValueOP::empty_val());
    
    return newIndex;
}

void Compiler::defineVariable(uint8_t global) {
    if (scopeDepth > 0) {
        markInitialized();
        return;
    }
    
    emitBytes(OP_DEFINE_GLOBAL, global);
}

void Compiler::variable(bool canAssign) {
    namedVariable(&parser->previous, canAssign);
}

void Compiler::namedVariable(Token* name, bool canAssign) {
    
    uint8_t getOp, setOp;
    int arg = resolveLocal(name);
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else if((arg = resolveUpvalue(name)) != -1) {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    } else {
        arg = globalConstant(name, false);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }
    
    if (canAssign && match(TOKEN_EQUAL)) {
        if(setOp == OP_SET_GLOBAL) {
            Value index;
            Value identifier = ValueOP::obj_val(ObjString::copyString(vm, name->source));
            vm->globalNames.tableGet(identifier, &index);
            if(ValueOP::isConst(index)) {
                parser->errorAtPrevious("Cannot assign to constant variable.");
                return;
            }
        } else {
            if(locals[arg].isConst) {
                parser->errorAtPrevious("Cannot assign to constant variable.");
                return;
            }
        }
        
        expression();
        emitBytes(setOp, arg);
    } else {
        emitBytes(getOp, arg);
    }
}

void Compiler::block() {
    while (!parser->check(TOKEN_RIGHT_BRACE) && !parser->check(TOKEN_EOF)) {
        compileStatement();
    }
    
    parser->consume(TOKEN_RIGHT_BRACE, "Expect '}' after block,");
}

void Compiler::beginScope() {
    scopeDepth++;
}

void Compiler::endScope() {
    scopeDepth--;
    
    while (localCount > 0 && locals[localCount - 1].depth > scopeDepth) {
        if (locals[localCount - 1].isCaptured) {
            emitByte(OP_CLOSE_UPVALUE);
        } else {
            emitByte(OP_POP);
        }
        localCount--;
    }
}

void Compiler::declareVariable(bool isConst) {
    if (scopeDepth == 0) return;
    
    Token* name = &parser->previous;
    for(int i = localCount - 1; i >= 0; i--) {
        Local* local = &locals[i];
        if (local->depth != -1 && local->depth < scopeDepth) {
            break;
        }
        
        if (identifierEqual(name, &local->name)) {
            parser->errorAtPrevious("Already variable with this name in this scope.");
        }
    }
    
    addLocal(*name, isConst);
}

void Compiler::addLocal(Token name, bool isConst) {
    if(localCount + 1 == locals.max_size()) {
        parser->errorAtPrevious("Too many local variables in function.");
        return;
    }
    
    if(localCount + 1 >= locals.capacity()) {
        locals.resize(locals.capacity() == 0 ? 8 : locals.capacity() * 2);
    }
    
    Local* local = &locals[localCount++];
    local->name = name;
    local->isConst = isConst;
}

int Compiler::resolveLocal(Token* name) {
    for (int i = localCount - 1; i >= 0; i--) {
        Local* local = &locals[i];
        if (identifierEqual(name, &local->name)) {
            if(local->depth == -1) {
                parser->errorAtPrevious("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }
    
    return -1;
}

void Compiler::markInitialized() {
    if(scopeDepth == 0) return;
    locals[localCount - 1].depth = scopeDepth;
}

void Compiler::ifStatement() {
    parser->consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    parser->consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
    
    size_t thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    
    size_t elseJump = emitJump(OP_JUMP);
    
    patchJump(thenJump);
    emitByte(OP_POP);
    
    if(match(TOKEN_ELSE)) statement();
    patchJump(elseJump);
}

size_t Compiler::emitJump(uint8_t instruction) {
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
}

void Compiler::patchJump(size_t offset) {
    size_t jump = currentChunk()->count - offset - 2;
    if (jump > UINT16_MAX) {
        parser->errorAtPrevious("Too much code to jump over.");
    }
    
    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

void Compiler::patchBreaks() {
    for(int i = (int)breakStatements.size() - 1; i >= 0 && breakStatements[i].depth >= innermostLoopScopeDepth; i--) {
        patchJump(breakStatements[i].position);
        breakStatements.pop_back();
    }
}

void Compiler::_and(bool canAssign) {
    size_t endJump = emitJump(OP_JUMP_IF_FALSE);
    
    emitByte(OP_POP);
    parsePrecedence(PREC_AND);
    
    patchJump(endJump);
}

void Compiler::_or(bool canAssign) {
    size_t elseJump = emitJump(OP_JUMP_IF_FALSE);
    size_t endJump = emitJump(OP_JUMP);
    
    patchJump(elseJump);
    emitByte(OP_POP);
    
    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

void Compiler::whileStatement() {
    int surroundingLoopStart = innermostLoopStart;
    int surroundingLoopScopeDepth = innermostLoopScopeDepth;
    innermostLoopStart = (int)currentChunk()->count;
    innermostLoopScopeDepth = scopeDepth;
    
    parser->consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    parser->consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition");
    
    size_t exitJump = emitJump(OP_JUMP_IF_FALSE);
    
    emitByte(OP_POP);
    statement();
    
    emitLoop(innermostLoopStart);
    
    patchJump(exitJump);
    emitByte(OP_POP);
    
    patchBreaks();
    
    innermostLoopStart = surroundingLoopStart;
    innermostLoopScopeDepth = surroundingLoopScopeDepth;
}

void Compiler::emitLoop(int loopStart) {
    emitByte(OP_LOOP);
    
    int offset = (int)currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX) parser->errorAtPrevious("Loop body too large.");
    
    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

void Compiler::forStatement() {
    beginScope();
    
    parser->consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for.");
    bool isConst = false;
    if (match(TOKEN_SEMICOLON)) {
        
    } else if (match(TOKEN_VAR) || (isConst = match(TOKEN_CONST))) {
        varDeclaration(isConst);
    } else {
        expressionStatement();
    }
    
    int surroundingLoopStart = innermostLoopStart;
    int surroundingLoopScopeDepth = innermostLoopScopeDepth;
    innermostLoopStart = (int)currentChunk()->count;
    innermostLoopScopeDepth = scopeDepth;
    
    int exitJump = -1;
    if(!match(TOKEN_SEMICOLON)) {
        expression();
        parser->consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");
        
        exitJump = (int)emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP);//Popping the condition to keep the stack clean
    }
    
    if(!match(TOKEN_RIGHT_PAREN)) {
        size_t bodyJump = emitJump(OP_JUMP);
        
        int incrementStart = (int)currentChunk()->count;
        expression();
        emitByte(OP_POP);
        parser->consume(TOKEN_RIGHT_PAREN, "Expect ')'.");
        
        emitLoop(innermostLoopStart);
        innermostLoopStart = incrementStart;
        patchJump(bodyJump);
    }
    
    
    statement();
    
    emitLoop(innermostLoopStart);
    if(exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP);
    }
    
    patchBreaks();
    
    
    innermostLoopStart = surroundingLoopStart;
    innermostLoopScopeDepth = surroundingLoopScopeDepth;
    
    endScope();
}


void Compiler::continueStatement() {
    if (innermostLoopScopeDepth == -1) {
        parser->errorAtPrevious("Cannot use 'continue' outside of a loop.");
    }
    
    parser->consume(TOKEN_SEMICOLON, "Expect ';' after 'continue'.");
    
    for (int i = localCount - 1; i >= 0 && locals[i].depth > innermostLoopScopeDepth; i--) {
        emitByte(OP_POP);
    }
    
    emitLoop(innermostLoopStart);
}

void Compiler::breakStatement() {
    if (innermostLoopScopeDepth == -1) {
        parser->errorAtPrevious("Cannot use 'break' outside of a loop");
    }
    
    parser->consume(TOKEN_SEMICOLON, "Expect ';' after 'break'.");
    
    for (int i = localCount - 1; i >= 0 && locals[i].depth > innermostLoopScopeDepth; i--) {
        emitByte(OP_POP);
    }
    
    breakStatements.push_back(Break{emitJump(OP_JUMP),innermostLoopScopeDepth});
}


void Compiler::switchStatement() {
#define BEFORE_CASES 0
#define BEFORE_DEFAULT 1
#define AFTER_DEFAULT 2
    
    
    parser->consume(TOKEN_LEFT_PAREN, "Expect '(' after 'switch'.");
    expression();
    parser->consume(TOKEN_RIGHT_PAREN, "Expect ')' after value.");
    parser->consume(TOKEN_LEFT_BRACE, "Exprect '{' before cases.");
    
    beginScope();
    int surroundingLoopStart = innermostLoopStart;
    int surroundingLoopScopeDepth = innermostLoopScopeDepth;
    innermostLoopStart = (int)currentChunk()->count;
    innermostLoopScopeDepth = scopeDepth;
    
    int state = BEFORE_CASES;
    int previousCaseSkip = -1;
    int caseCount = 0;
    
    while (!match(TOKEN_RIGHT_BRACE) && !parser->check(TOKEN_EOF)) {
        if(match(TOKEN_CASE) || match(TOKEN_DEFAULT)) {
            TokenType caseType = parser->previous.type;
            
            
            if (state == AFTER_DEFAULT) {
                parser->errorAtPrevious("Can't have another case or default after the default case.");
            }
            
            if(state == 1) {
                size_t previousCaseEnds = emitJump(OP_JUMP);
                caseCount++;
                if(caseCount == MAX_CASES) {
                    parser->errorAtPrevious("Too many cases in switch statement");
                }
                
                patchJump(previousCaseSkip);
                emitByte(OP_POP);
                
                patchJump(previousCaseEnds);
            }
            
            if (caseType == TOKEN_CASE) {
                state = BEFORE_DEFAULT;
                
                emitByte(OP_DUP);
                expression();
                
                parser->consume(TOKEN_COLON, "Expect ':' after case value.");
                
                emitByte(OP_EQUAL);
                previousCaseSkip = (int)emitJump(OP_JUMP_IF_FALSE);
                
                emitByte(OP_POP);
                
            } else {
                state = 2;
                parser->consume(TOKEN_COLON, "Expect ':' after default.");
                previousCaseSkip = -1;
            }
        } else {
            if(state == 0) {
                parser->errorAtPrevious("Cannot have statements before any case.");
            }
            statement();
        }
    }
    
    patchBreaks();
    if(state == 1) {
        patchJump(previousCaseSkip);
        emitByte(OP_POP);
    }
    
    emitByte(OP_POP);
    
    innermostLoopStart = surroundingLoopStart;
    innermostLoopScopeDepth = surroundingLoopScopeDepth;
    endScope();
#undef BEFORE_CASES
#undef BEFORE_DEFAULT
#undef AFTER_DEFAULT
}

void Compiler::funDeclaration() {
    uint8_t global = parseVariable("Expect function name", false, true);
    markInitialized();
    _function(TYPE_FUNCTION);
    defineVariable(global);
}

void Compiler::_function(FunctionType type) {
    Compiler compiler(vm, type, this, scanner, parser, current_source);
    compiler.beginScope();
    
    parser->consume(TOKEN_LEFT_PAREN, "Expect '(' after function name,");
    
    if(!compiler.parser->check(TOKEN_RIGHT_PAREN)) {
        bool found_default = false;
        do {
            compiler.function->arity++;
            if(compiler.function->arity > 255) {
                parser->errorAtCurrent("Can't have more than 255 paramethers.");
            }
            
            uint8_t paramConstant = compiler.parseVariable("Expect parameter name.", false, false);
            if(match(TOKEN_EQUAL)) {
                parser->consume(TOKEN_LEFT_BRACE, "Expect '{' after default parameter.");
                found_default = true;
                compiler.function->defaults++;
                size_t jumploc = compiler.emitJump(OP_JUMP_IF_EMPTY);
                compiler.expression();
                size_t end_jump = compiler.emitJump(OP_JUMP);
                compiler.patchJump(jumploc);
                compiler.emitByte(OP_POP);
                compiler.patchJump(end_jump);
                parser->consume(TOKEN_RIGHT_BRACE, "Expect '}' after default parameter definition.");
            } else if(found_default) {
                parser->errorAtCurrent("Cannot declare non-default parameters after default parameters.");
                
            }
            
            compiler.defineVariable(paramConstant);
        } while (match(TOKEN_COMMA));
    }
    
    parser->consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    
    parser->consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    compiler.block();
    
    ObjFunction* function = compiler.endCompiler();
    
    uint8_t functionConstant = makeConstant(ValueOP::obj_val(function));
    if(function->upvalueCount > 0) {
        emitBytes(OP_CLOSURE, functionConstant);
        
        for(int i = 0; i < function->upvalueCount; i++) {
            emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
            emitByte(compiler.upvalues[i].index);
        }
    } else {
        emitBytes(OP_CONSTANT, functionConstant);
    }
}

void Compiler::call(bool canAssign) {
    uint8_t argCount = argumentList();
    emitBytes(OP_CALL, argCount);
}

uint8_t Compiler::argumentList() {
    uint8_t argCount = 0;
    if(!parser->check(TOKEN_RIGHT_PAREN)) {
        do {
            expression();
            
            if(argCount == 255) {
                parser->errorAtPrevious("Can't have more than 255 arguments.");
            }
            argCount++;
        } while(match(TOKEN_COMMA));
    }
    
    parser->consume(TOKEN_RIGHT_PAREN, "Expect ')' after argument");
    return argCount;
}

void Compiler::returnStatement() {
    if(type == TYPE_SCRIPT) {
        parser->errorAtPrevious("Can't return from top-level code");
    }
    
    if(match(TOKEN_SEMICOLON)) {
        emitReturn();
    } else {
        if(type == TYPE_INITIALIZER) {
            parser->errorAtPrevious("Can't return a value from an initializer.");
        }
        expression();
        parser->consume(TOKEN_SEMICOLON, "Expect ';' after return.");
        emitByte(OP_RETURN);
    }
}

int Compiler::resolveUpvalue(Token *name) {
    if(enclosing == nullptr || type == TYPE_IMPORT) return -1;
    
    int local = enclosing->resolveLocal(name);
    if(local != -1) {
        enclosing->locals[local].isCaptured = true;
        return addUpvalue((uint8_t)local, true);
    }
    
    int upvalue = enclosing->resolveUpvalue(name);
    if(upvalue != -1) {
        return addUpvalue((uint8_t)upvalue, false);
    }
    
    return -1;
}

int Compiler::addUpvalue(uint8_t index, bool isLocal) {
    int upvalueCount = function->upvalueCount;
    
    for (int i = 0; i < upvalueCount; i++) {
        Upvalue* upvalue = &upvalues[i];
        if(upvalue->index == index && upvalue->isLocal == isLocal) {
            return i;
        }
    }
    
    if(upvalueCount == upvalues.max_size()) {
        parser->errorAtPrevious("Too many closure variable in function");
        return 0;
    }
    
    upvalues.push_back(Upvalue{index, isLocal});
    return function->upvalueCount++;
}

void Compiler::markCompilerRoots() {
    Compiler* compiler = this;
    while(compiler != nullptr) {
        markObject(vm, (Obj*)compiler->function);
        compiler = compiler->enclosing;
    }
}

void Compiler::classDeclaration() {
    parser->consume(TOKEN_IDENTIFIER, "Expect class name.");
    Token className = parser->previous;
    uint8_t global = globalConstant(&parser->previous, false);
    declareVariable(false);
    
    uint8_t name = addIdentifierConstant(&parser->previous);
    emitBytes(OP_CLASS, name);
    defineVariable(global);
    
    ClassCompiler classCompiler;
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = vm->currentClass;
    vm->currentClass = &classCompiler;
    
    if (match(TOKEN_LESS)) {
        parser->consume(TOKEN_IDENTIFIER, "Expect superclass name.");
        variable(false);
        
        if(identifierEqual(&className, &parser->previous)) {
            parser->errorAtPrevious("A class can't inherit from itself.");
        }
        
        beginScope();
        addLocal(Token::createToken("super"), true);
        defineVariable(0);
        
        namedVariable(&className, false);
        emitByte(OP_INHERIT);
        classCompiler.hasSuperclass = true;
    }
    
    namedVariable(&className, false);
    parser->consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    while(!parser->check(TOKEN_RIGHT_BRACE) && !parser->check(TOKEN_EOF)) {
        method();
    }
    parser->consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
    emitByte(OP_POP);
    
    if(classCompiler.hasSuperclass) {
        endScope();
    }
    
    vm->currentClass = vm->currentClass->enclosing;
}

void Compiler::dot(bool canAssign) {
    parser->consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = addIdentifierConstant(&parser->previous);
    
    if(canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(OP_SET_PROPERTY, name);
    } else if(match(TOKEN_LEFT_PAREN)) {
        uint8_t argCount = argumentList();
        emitBytes(OP_INVOKE, name);
        emitByte(argCount);
    } else {
        emitBytes(OP_GET_PROPERTY, name);
    }
}

uint8_t Compiler::addIdentifierConstant(Token *name) {
    ObjString* string = ObjString::copyString(vm, name->source);
    Value indexValue;
    
    if(stringConstants.tableGet(ValueOP::obj_val(string),&indexValue)) {
        return (uint8_t)(ValueOP::as_number(indexValue).number.whole);
    }
    
    uint8_t index = makeConstant(ValueOP::obj_val(string));
    stringConstants.tableSet(ValueOP::obj_val(string), ValueOP::number_val(index));
    return index;
}

void Compiler::delStatement() {
    parser->consume(TOKEN_IDENTIFIER, "Can only delete fields from a class instance.");
    variable(true);
    
    parser->consume(TOKEN_DOT, "Can only delete fields.");
    parser->consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    
    uint8_t name = addIdentifierConstant(&parser->previous);
    emitBytes(OP_DEL, name);
    
    parser->consume(TOKEN_SEMICOLON, "Expect ';' after del statement.");
}

void Compiler::method() {
    parser->consume(TOKEN_IDENTIFIER, "Expect method name.");
    uint8_t constant = addIdentifierConstant(&parser->previous);
    
    FunctionType type = TYPE_METHOD;
    
    if(parser->previous.length == 4 && parser->previous.source.compare("init") == 0) {
        type = TYPE_INITIALIZER;
    }
    
    _function(type);
    emitBytes(OP_METHOD, constant);
}

void Compiler::_this(bool canAssign) {
    if(vm->currentClass == nullptr) {
        parser->errorAtPrevious("Can't use 'this' outside of a class.");
        return;
    }
    variable(false);
}

void Compiler::super(bool canAssign) {
    if(!vm->currentClass) {
        parser->errorAtPrevious("Can't use 'super' outside of a class.");
    } else if (!vm->currentClass->hasSuperclass) {
        parser->errorAtPrevious("Can't use 'super' int a class with no superclass.");
    }
    parser->consume(TOKEN_DOT, "Expect '.' after 'super'");
    parser->consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = addIdentifierConstant(&parser->previous);
    
    Token t = Token::createToken("this");
    Token s = Token::createToken("super");
    namedVariable(&t, false);
    if(match(TOKEN_LEFT_PAREN)) {
        uint8_t argCount = argumentList();
        namedVariable(&s, false);
        emitBytes(OP_SUPER_INVOKE, name);
        emitByte(argCount);
    } else {
        namedVariable(&s, false);
        emitBytes(OP_GET_SUPER, name);
    }
}

void Compiler::randomAccess(bool canAssign) {
    parsePrecedence(PREC_CALL);
    parser->consume(TOKEN_RIGHT_BRACK, "Expect ']' after expression.");
    if(parser->check(TOKEN_EQUAL)) {
        parser->advance();
        expression();
        Token name = Token::createToken("indexAssign");
        emitBytes(OP_INVOKE, addIdentifierConstant(&name));
        emitByte(2);
    } else {
        Token name = Token::createToken("indexAccess");
        emitBytes(OP_INVOKE, addIdentifierConstant(&name));
        emitByte(1);
    }
}


void Compiler::steps(bool canAssign) {
    parsePrecedence(PREC_OR);
    if(match(TOKEN_COLON)) {
        expression();
    } else {
        emitConstant(ValueOP::number_val(1));
    }
    
    emitByte(OP_RANGE);
}


void Compiler::importStatement() {
    parser->consume(TOKEN_STRING, "Expect module name after import statement");
    
    size_t path_index = current_source.rfind("/");
    std::filesystem::path module_option = current_source.substr(0, path_index == std::string::npos ? 0 : path_index + 1) + parser->previous.source.substr(1, parser->previous.source.size() - 2);
    
    module_option += ".lox";
    std::filesystem::path module_absolute = std::filesystem::absolute(module_option);
    std::string module_string = module_absolute.string();
    
    parser->consume(TOKEN_SEMICOLON, "Expect ; after import statement");
    
    if(imported_module.count(module_string) || compiled_source.count(module_string)) {
        return;
    }
    imported_module.insert(module_string);
    
    
    std::string import = "";
    try {
        import = readFile(module_string.c_str());
    } catch(std::string e) {
        parser->errorAtPrevious(e);
    }
    
    Scanner scanner;
    Parser parser(&scanner);
    
    Compiler importScript(this->vm, TYPE_IMPORT, this, &scanner, &parser, module_string);
    importScript.compiled_source.insert(current_source);
    importScript.compiled_source.insert(compiled_source.begin(), compiled_source.end());
    
    ObjFunction* importedFunction = importScript.compile(import);
    
    imported_module.insert(importScript.imported_module.begin(), importScript.imported_module.end());
    
    emitBytes(OP_CONSTANT, makeConstant(ValueOP::obj_val(importedFunction)));
    emitBytes(OP_CALL, 0);
    emitByte(OP_POP);
}
