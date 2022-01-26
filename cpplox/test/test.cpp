#include <gtest/gtest.h>
#include "../pch.pch"
#include "../scanner.hpp"
#include "../chunk.hpp"
#include "../compiler.hpp"
#include "../table.hpp"
#include "../value.hpp"
#include "../object.hpp"

class Scanner_Test : public testing::Test {
protected:
    Scanner scan;
};

TEST_F(Scanner_Test, constructor) {
    EXPECT_EQ(scan.start, 0);
    EXPECT_EQ(scan.current, 0);
    EXPECT_EQ(scan.line, 1);
}

TEST_F(Scanner_Test, left_paren) {
    scan.setSource("(");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_LEFT_PAREN);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, right_paren) {
    scan.setSource(")");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_RIGHT_PAREN);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, left_brace) {
    scan.setSource("{");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_LEFT_BRACE);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, right_brace) {
    scan.setSource("}");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_RIGHT_BRACE);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, left_brack) {
    scan.setSource("[");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_LEFT_BRACK);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, right_brack) {
    scan.setSource("]");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_RIGHT_BRACK);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, comma) {
    scan.setSource(",");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_COMMA);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, dot) {
    scan.setSource(".");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_DOT);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, minus) {
    scan.setSource("-");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_MINUS);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, plus) {
    scan.setSource("+");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_PLUS);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, semicolon) {
    scan.setSource(";");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_SEMICOLON);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, slash) {
    scan.setSource("/");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_SLASH);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, star) {
    scan.setSource("*");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_STAR);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, bang) {
    scan.setSource("!");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_BANG);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, bang_equal) {
    scan.setSource("!=");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_BANG_EQUAL);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, equal) {
    scan.setSource("=");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_EQUAL);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, equal_equal) {
    scan.setSource("==");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_EQUAL_EQUAL);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, greater) {
    scan.setSource(">");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_GREATER);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, greater_equal) {
    scan.setSource(">=");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_GREATER_EQUAL);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, lesser) {
    scan.setSource("<");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_LESS);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, lesser_equal) {
    scan.setSource("<=");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_LESS_EQUAL);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, question_mark) {
    scan.setSource("?");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_QUESTION_MARK);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, colon) {
    scan.setSource(":");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_COLON);
    EXPECT_EQ(token.length, 1);
}

TEST_F(Scanner_Test, identifier) {
    scan.setSource("test");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(token.length, 4);
}

TEST_F(Scanner_Test, string) {
    scan.setSource("\"string\"");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_STRING);
    EXPECT_EQ(token.length, 8);
    EXPECT_EQ(token.source.substr(1, token.length - 2).compare("string"), 0);
}

TEST_F(Scanner_Test, number) {
    scan.setSource("123");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_NUMBER);
    EXPECT_EQ(token.length, 3);
    EXPECT_EQ(std::stoi(token.source), 123);
}

TEST_F(Scanner_Test, token_and) {
    scan.setSource("and");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_AND);
    EXPECT_EQ(token.length, 3);
}

TEST_F(Scanner_Test, token_class) {
    scan.setSource("class");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_CLASS);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_else) {
    scan.setSource("else");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_ELSE);
    EXPECT_EQ(token.length, 4);
}

TEST_F(Scanner_Test, token_false) {
    scan.setSource("false");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_FALSE);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_for) {
    scan.setSource("for");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_FOR);
    EXPECT_EQ(token.length, 3);
}

TEST_F(Scanner_Test, fun) {
    scan.setSource("fun");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_FUN);
    EXPECT_EQ(token.length, 3);
}

TEST_F(Scanner_Test, token_if) {
    scan.setSource("if");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_IF);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, nul) {
    scan.setSource("nul");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_NUL);
    EXPECT_EQ(token.length, 3);
}

TEST_F(Scanner_Test, token_or) {
    scan.setSource("or");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_OR);
    EXPECT_EQ(token.length, 2);
}

TEST_F(Scanner_Test, print) {
    scan.setSource("print");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_PRINT);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_return) {
    scan.setSource("return");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_RETURN);
    EXPECT_EQ(token.length, 6);
}

TEST_F(Scanner_Test, super) {
    scan.setSource("super");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_SUPER);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_this) {
    scan.setSource("this");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_THIS);
    EXPECT_EQ(token.length, 4);
}

TEST_F(Scanner_Test, token_true) {
    scan.setSource("true");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_TRUE);
    EXPECT_EQ(token.length, 4);
}

TEST_F(Scanner_Test, var) {
    scan.setSource("var");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_VAR);
    EXPECT_EQ(token.length, 3);
}

TEST_F(Scanner_Test, token_const) {
    scan.setSource("const");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_CONST);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_while) {
    scan.setSource("while");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_WHILE);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_continue) {
    scan.setSource("continue");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_CONTINUE);
    EXPECT_EQ(token.length, 8);
}

TEST_F(Scanner_Test, token_break) {
    scan.setSource("break");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_BREAK);
    EXPECT_EQ(token.length, 5);
}

TEST_F(Scanner_Test, token_switch) {
    scan.setSource("switch");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_SWITCH);
    EXPECT_EQ(token.length, 6);
}

TEST_F(Scanner_Test, token_case) {
    scan.setSource("case");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_CASE);
    EXPECT_EQ(token.length, 4);
}

TEST_F(Scanner_Test, token_default) {
    scan.setSource("default");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_DEFAULT);
    EXPECT_EQ(token.length, 7);
}

TEST_F(Scanner_Test, token_del) {
    scan.setSource("delete");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_DEL);
    EXPECT_EQ(token.length, 6);
}

TEST_F(Scanner_Test, import) {
    scan.setSource("import");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_IMPORT);
    EXPECT_EQ(token.length, 6);
}

TEST_F(Scanner_Test, error) {
    scan.setSource("@");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_ERROR);
    EXPECT_EQ(token.source.compare("Unexpected character."), 0);
}

TEST_F(Scanner_Test, eof) {
    scan.setSource("");
    Token token = scan.scanToken();
    
    EXPECT_EQ(token.type, TOKEN_EOF);
    EXPECT_EQ(token.length, 0);
}

TEST_F(Scanner_Test, line) {
    scan.setSource("line1\nline2\nline3");
    
    Token line1 = scan.scanToken(), line2 = scan.scanToken(), line3 = scan.scanToken();
    EXPECT_EQ(line1.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(line2.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(line3.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(line1.line, 1);
    EXPECT_EQ(line2.line, 2);
    EXPECT_EQ(line3.line, 3);
}

TEST_F(Scanner_Test, spaces) {
    scan.setSource("first     second     third\n   fourth");
    
    Token one = scan.scanToken(), two = scan.scanToken(), three = scan.scanToken(), four = scan.scanToken();
    EXPECT_EQ(one.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(two.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(three.type, TOKEN_IDENTIFIER);
    EXPECT_EQ(four.type, TOKEN_IDENTIFIER);
    
    EXPECT_EQ(one.source.compare("first"), 0);
    EXPECT_EQ(two.source.compare("second"), 0);
    EXPECT_EQ(three.source.compare("third"), 0);
    EXPECT_EQ(four.source.compare("fourth"), 0);
}


class Chunk_test : public testing::Test {
protected:
    Chunk chunk;
};

TEST_F(Chunk_test, constructor) {
    EXPECT_EQ(chunk.count, 0);
    EXPECT_EQ(chunk.lineCount, 0);
}

TEST_F(Chunk_test, test_write_chunk) {
    chunk.writeChunk(OP_NUL, 1);
    chunk.writeChunk(OP_ADD, 2);
    
    EXPECT_EQ(chunk.code[0], OP_NUL);
    EXPECT_EQ(chunk.code[1], OP_ADD);
}

TEST_F(Chunk_test, test_lines) {
    chunk.writeChunk(OP_NUL, 1);
    chunk.writeChunk(OP_NUL, 1);
    chunk.writeChunk(OP_NUL, 2);
    chunk.writeChunk(OP_NUL, 2);
    chunk.writeChunk(OP_NUL, 3);
    chunk.writeChunk(OP_NUL, 4);
    
    EXPECT_EQ(chunk.lineCount, 4);
    EXPECT_EQ(chunk.getLine(0), 1);
    EXPECT_EQ(chunk.getLine(1), 1);
    EXPECT_EQ(chunk.getLine(2), 2);
    EXPECT_EQ(chunk.getLine(3), 2);
    EXPECT_EQ(chunk.getLine(4), 3);
    EXPECT_EQ(chunk.getLine(5), 4);
}

TEST_F(Chunk_test, test_constant) {
    std::vector<int> position;
    for(int i = 0; i < 100; i++) position.push_back(chunk.addConstant(ValueOP::number_val(i)));
    for(int i = 0; i < 100; i++) {
        EXPECT_EQ(chunk.constants.values[i].as.number, i);
        EXPECT_EQ(position[i], i);
    }
}

TEST_F(Chunk_test, test_add_constant) {
    chunk.writeConstant(ValueOP::number_val(1), 1);
    chunk.writeConstant(ValueOP::number_val(2), 2);
    chunk.writeConstant(ValueOP::number_val(3), 3);
    chunk.writeConstant(ValueOP::number_val(4), 4);
    for(int i = 0; i < 4; i++) {
        EXPECT_EQ(chunk.constants.values[i].as.number, i + 1);
    }
}

TEST_F(Chunk_test, test_large_constant) {
    for(int i = 0; i < 257; i++) chunk.writeConstant(ValueOP::number_val(i), i);
    EXPECT_EQ(chunk.code[512], OP_CONSTANT_LONG);
    
    long constant = chunk.code[513];
    constant |= chunk.code[514] << 8;
    constant |= chunk.code[515] << 16;
    constant |= chunk.code[516] << 24;
    EXPECT_EQ(constant, 256);
}

class Parser_test : public testing::Test {
protected:
    Scanner scan;
    Parser* parser;
    
    void SetUp() override {
        parser = new Parser(&scan);
        parser->panicMode = false;
        parser->hadError = false;
    }
    
    void TearDown() override {
        delete parser;
    }
};

TEST_F(Parser_test, consume) {
    scan.setSource("123 abc");
    parser->advance();
    parser->consume(TOKEN_NUMBER, "Should be number");
    EXPECT_EQ(parser->hadError, false);
    parser->consume(TOKEN_IDENTIFIER, "Should be identifier");
    EXPECT_EQ(parser->hadError, false);
}

TEST_F(Parser_test, error_at_previous) {
    scan.setSource("123\nabc");
    parser->advance();
    parser->advance();
    
    testing::internal::CaptureStderr();
    parser->errorAtPrevious("test message");
    std::string message1 = testing::internal::GetCapturedStderr();
    EXPECT_EQ(parser->panicMode, true);
    EXPECT_EQ(parser->hadError, true);
    EXPECT_STREQ(message1.c_str(), "[line 1] Error at 123: test message\n");
    
    testing::internal::CaptureStderr();
    parser->errorAtCurrent("test message");
    std::string message2 = testing::internal::GetCapturedStderr();
    EXPECT_EQ(parser->hadError, true);
    EXPECT_STREQ(message2.c_str(), "");
}

TEST_F(Parser_test, error_at_current) {
    scan.setSource("123\nabc");
    parser->advance();
    parser->advance();
    
    testing::internal::CaptureStderr();
    parser->errorAtCurrent("test message");
    std::string message = testing::internal::GetCapturedStderr();
    EXPECT_EQ(parser->panicMode, true);
    EXPECT_EQ(parser->hadError, true);
    EXPECT_STREQ(message.c_str(), "[line 2] Error at abc: test message\n");
}

TEST_F(Parser_test, test_check) {
    scan.setSource("123");
    parser->advance();
    
    EXPECT_TRUE(parser->check(TOKEN_NUMBER));
}

TEST_F(Parser_test, advance_test) {
    scan.setSource("123 # # # abc");
    
    parser->advance();
    EXPECT_EQ(parser->current.type, TOKEN_NUMBER);
    
    parser->advance();
    EXPECT_EQ(parser->current.type, TOKEN_IDENTIFIER);
}

TEST_F(Parser_test, check_test) {
    scan.setSource("123");
    parser->advance();
    
    EXPECT_EQ(parser->check(TOKEN_NUMBER), true);
}

class ValueArray_test : public testing::Test {
protected:
    ValueArray arr;
};

testing::AssertionResult print_value_test(Value val, const std::string& comp) {
    testing::internal::CaptureStdout();
    ValueOP::printValue(val);
    std::string buffer = testing::internal::GetCapturedStdout();
    if(buffer.compare(comp))
        return testing::AssertionFailure() << "printed out \"" << buffer << "\" rather than \"" << comp << "\"";
    else
        return testing::AssertionSuccess();
}

TEST_F(ValueArray_test, write_test) {
    arr.writeValueArray(ValueOP::nul_val());
    arr.writeValueArray(ValueOP::number_val(123));
    
    EXPECT_EQ(arr.count, 2);
    EXPECT_TRUE(ValueOP::is_nul(arr.values[0]));
    EXPECT_TRUE(ValueOP::is_number(arr.values[1]));
    EXPECT_EQ(ValueOP::as_number(arr.values[1]), 123);
}


// Note that these tests do not test the VM's ability to manipulate values and object but instead test the value and objects themselves.
class Value_test : public testing::Test {
protected:
    VM vm;
};

TEST_F(Value_test, bool_test) {
    std::string buffer;
    ObjString* string;
    
    Value true_value = ValueOP::bool_val(true);
    Value false_value = ValueOP::bool_val(false);
    
    EXPECT_EQ(true_value.type, VAL_BOOL);
    EXPECT_EQ(false_value.type, VAL_BOOL);
    EXPECT_TRUE(ValueOP::is_bool(true_value));
    EXPECT_TRUE(true_value.as.boolean);
    EXPECT_TRUE(ValueOP::as_bool(true_value));
    
    EXPECT_TRUE(ValueOP::is_bool(false_value));
    EXPECT_FALSE(false_value.as.boolean);
    EXPECT_FALSE(ValueOP::as_bool(false_value));
    
    EXPECT_TRUE(print_value_test(true_value, "true"));
    EXPECT_TRUE(print_value_test(false_value, "false"));

    
    string = ValueOP::to_string(true_value, &vm);
    EXPECT_STREQ(string->chars.c_str(), "true");
    
    string = ValueOP::to_string(false_value, &vm);
    EXPECT_STREQ(string->chars.c_str(), "false");
    
    EXPECT_FALSE(ValueOP::valuesEqual(true_value, false_value));
    EXPECT_FALSE(ValueOP::valuesEqual(false_value, true_value));
    EXPECT_TRUE(ValueOP::valuesEqual(true_value, ValueOP::bool_val(true)));
    EXPECT_TRUE(ValueOP::valuesEqual(false_value, ValueOP::bool_val(false)));
}

TEST_F(Value_test, number_test) {
    Value num_value = ValueOP::number_val(123);
    
    EXPECT_EQ(num_value.type, VAL_NUMBER);
    EXPECT_TRUE(ValueOP::is_number(num_value));
    EXPECT_EQ(num_value.as.number, 123);
    EXPECT_EQ(ValueOP::as_number(num_value), 123);
    
    EXPECT_TRUE(print_value_test(num_value, "123"));
    
    ObjString* string = ValueOP::to_string(num_value, &vm);
    EXPECT_STREQ(string->chars.c_str(), "123.000000");
    
    EXPECT_TRUE(ValueOP::valuesEqual(num_value, ValueOP::number_val(123)));
    EXPECT_FALSE(ValueOP::valuesEqual(num_value, ValueOP::number_val(124)));
}

TEST_F(Value_test, object_string_test) {
    ObjString* s = ObjString::copyString(&vm, "test", 4);
    Value string_val = ValueOP::obj_val(s);
    
    EXPECT_EQ(s->type, OBJ_STRING);
    EXPECT_STREQ(s->chars.c_str(), "test");
    
    EXPECT_EQ(ObjString::hashString("abcde", 5), ObjString::hashString("abcde", 5));
    
    EXPECT_EQ(string_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(string_val));
    EXPECT_TRUE(ValueOP::is_string(string_val));
    EXPECT_STREQ(ValueOP::as_string(string_val)->chars.c_str(), "test");
    
    EXPECT_TRUE(print_value_test(string_val, "test"));
    
    vm.push_stack(string_val);
    Value second_string_val = ValueOP::obj_val(ObjString::copyString(&vm, "test", 4));
    EXPECT_TRUE(ValueOP::valuesEqual(string_val, second_string_val));
}

TEST_F(Value_test, function_test) {
    ObjFunction* f = ObjFunction::newFunction(&vm, TYPE_FUNCTION);
    Value function_val = ValueOP::obj_val(f);
    vm.push_stack(function_val);
    
    f->name = ObjString::copyString(&vm, "abc", 3);
    
    EXPECT_EQ(f->type, OBJ_FUNCTION);
    EXPECT_TRUE(ValueOP::is_obj(function_val));
    EXPECT_TRUE(ValueOP::is_function(function_val));
    EXPECT_STREQ(f->name->chars.c_str(), "abc");
    
    EXPECT_TRUE(print_value_test(function_val, "<fn abc>"));
    
    EXPECT_EQ(function_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(function_val));
    EXPECT_EQ(f, ValueOP::as_function(function_val));
}

TEST_F(Value_test, native_test) {
    ObjNative* native_f = ObjNative::newNative(&VM::clockNative, 0, &vm);
    Value native_function_val = ValueOP::obj_val(native_f);
    
    EXPECT_EQ(native_f->type, OBJ_NATIVE);
    EXPECT_EQ(native_function_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(native_function_val));
    EXPECT_TRUE(ValueOP::is_native(native_function_val));
    
    EXPECT_TRUE(print_value_test(native_function_val, "<native fn>"));
    
    EXPECT_EQ(ValueOP::as_native(native_function_val), native_f);
    EXPECT_EQ(native_f->arity, 0);
}

TEST_F(Value_test, upvalue_test) {
    ObjUpvalue* upvalue = ObjUpvalue::newUpvalue(&vm.stack[0], &vm);
    Value upvalue_val = ValueOP::obj_val(upvalue);
    
    EXPECT_EQ(upvalue->type, OBJ_UPVALUE);
    EXPECT_EQ(upvalue_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(upvalue_val));
    EXPECT_TRUE(ValueOP::is_upvalue(upvalue_val));
    
    EXPECT_TRUE(print_value_test(upvalue_val, "upvalue"));
    
}

TEST_F(Value_test, closure_test) {
    ObjFunction* func = ObjFunction::newFunction(&vm, TYPE_FUNCTION);
    vm.push_stack(ValueOP::obj_val(func));
    func->name = ObjString::copyString(&vm, "abc", 3);
    ObjClosure* closure = ObjClosure::newClosure(func, &vm);
    Value closure_val = ValueOP::obj_val(closure);
    
    EXPECT_EQ(closure->type, OBJ_CLOSURE);
    EXPECT_EQ(closure_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(closure_val));
    EXPECT_TRUE(ValueOP::is_closure(closure_val));
    
    EXPECT_TRUE(print_value_test(closure_val, "<fn abc>"));
    
    EXPECT_EQ(ValueOP::as_closure(closure_val), closure);
    EXPECT_EQ(closure->upvalueCount, 0);
}

TEST_F(Value_test, class_test) {
    ObjString* name = ObjString::copyString(&vm, "test_class", 10);
    vm.push_stack(ValueOP::obj_val(name));
    ObjClass* _class = ObjClass::newClass(name, &vm);
    Value _class_val = ValueOP::obj_val(_class);
    
    EXPECT_EQ(_class->type, OBJ_CLASS);
    EXPECT_EQ(_class_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(_class_val));
    EXPECT_TRUE(ValueOP::is_class(_class_val));
    
    EXPECT_TRUE(print_value_test(_class_val, "test_class"));
    
    EXPECT_EQ(ValueOP::as_class(_class_val), _class);
    EXPECT_STREQ(_class->name->chars.c_str(), "test_class");
    EXPECT_EQ(_class->name, name);
    
    EXPECT_EQ(_class->initializer, nullptr);
}

TEST_F(Value_test, instance_test) {
    ObjString* name = ObjString::copyString(&vm, "test_class", 10);
    vm.push_stack(ValueOP::obj_val(name));
    ObjClass* _class = ObjClass::newClass(name, &vm);
    vm.push_stack(ValueOP::obj_val(_class));
    ObjInstance* instance = ObjInstance::newInstance(_class, &vm);
    Value instance_val = ValueOP::obj_val(instance);
    
    EXPECT_EQ(instance->type, OBJ_INSTANCE);
    EXPECT_EQ(instance_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(instance_val));
    EXPECT_TRUE(ValueOP::is_instance(instance_val));
    
    EXPECT_TRUE(print_value_test(instance_val, "test_class instance"));
    
    EXPECT_EQ(ValueOP::as_instance(instance_val), instance);
    EXPECT_EQ(instance->_class, _class);
}

TEST_F(Value_test, bound_method_test) {
    ObjString* name = ObjString::copyString(&vm, "test_class", 10);
    vm.push_stack(ValueOP::obj_val(name));
    ObjClass* _class = ObjClass::newClass(name, &vm);
    vm.push_stack(ValueOP::obj_val(_class));
    ObjInstance* instance = ObjInstance::newInstance(_class, &vm);
    vm.push_stack(ValueOP::obj_val(instance));
    ObjFunction* func = ObjFunction::newFunction(&vm, TYPE_FUNCTION);
    vm.push_stack(ValueOP::obj_val(func));
    func->name = ObjString::copyString(&vm, "abc", 3);
    ObjBoundMethod* bound_method = ObjBoundMethod::newBoundMethod(ValueOP::obj_val(instance), func, &vm);
    Value bound_method_val = ValueOP::obj_val(bound_method);
    
    EXPECT_EQ(bound_method->type, OBJ_BOUND_METHOD);
    EXPECT_EQ(bound_method_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(bound_method_val));
    EXPECT_TRUE(ValueOP::is_bound_method(bound_method_val));
    
    EXPECT_TRUE(print_value_test(bound_method_val, "<fn abc>"));
    
    EXPECT_EQ(ValueOP::as_bound_method(bound_method_val), bound_method);
    EXPECT_EQ(bound_method->method, func);
}

TEST_F(Value_test, native_class_method_test) {
    ObjNativeClassMethod* method = ObjNativeClassMethod::newNativeClassMethod(static_cast<NativeClassMethod>(&ObjCollectionClass::init), &vm);
    Value method_val = ValueOP::obj_val(method);
    
    EXPECT_EQ(method->type, OBJ_NATIVE_CLASS_METHOD);
    EXPECT_EQ(method_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(method_val));
    EXPECT_TRUE(ValueOP::is_native_method(method_val));
    
    EXPECT_TRUE(print_value_test(method_val, "native class method"));
    
    EXPECT_EQ(ValueOP::as_native_class_method(method_val), method);
}

TEST_F(Value_test, collection_test) {
    ObjString* name = ObjString::copyString(&vm, "collection_class", 10);
    vm.push_stack(ValueOP::obj_val(name));
    ObjCollectionClass* collection = ObjCollectionClass::newCollectionClass(name, &vm);
    Value collection_val = ValueOP::obj_val(collection);
    
    EXPECT_EQ(collection->type, OBJ_NATIVE_CLASS);
    EXPECT_EQ(collection->subType, NATIVE_COLLECTION);
    EXPECT_EQ(collection_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(collection_val));
    EXPECT_TRUE(ValueOP::is_native_class(collection_val));
    EXPECT_TRUE(ValueOP::is_native_subclass(collection_val, NATIVE_COLLECTION));
    
    EXPECT_TRUE(print_value_test(collection_val, "Collection Class"));
    
    EXPECT_EQ(ValueOP::as_native_subclass<ObjCollectionClass>(collection_val), collection);
}

TEST_F(Value_test, collection_instance_test) {
    ObjString* name = ObjString::copyString(&vm, "collection_class", 10);
    vm.push_stack(ValueOP::obj_val(name));
    ObjCollectionClass* collection = ObjCollectionClass::newCollectionClass(name, &vm);
    vm.push_stack(ValueOP::obj_val(collection));
    ObjCollectionInstance* instance = ObjCollectionInstance::newCollectionInstance(collection, &vm);
    Value instance_val = ValueOP::obj_val(instance);
    
    EXPECT_EQ(instance->type, OBJ_NATIVE_INSTANCE);
    EXPECT_EQ(instance->subType, NATIVE_COLLECTION_INSTANCE);
    EXPECT_EQ(instance_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(instance_val));
    EXPECT_TRUE(ValueOP::is_native_instance(instance_val));
    EXPECT_TRUE(ValueOP::is_native_subinstance(instance_val, NATIVE_COLLECTION_INSTANCE));
    
    EXPECT_TRUE(print_value_test(instance_val, "{}"));
    
    EXPECT_EQ(ValueOP::as_native_subinstance<ObjCollectionInstance>(instance_val), instance);
}

class Table_test : public testing::Test {
protected:
    VM vm;
    Table table;
};

TEST_F(Table_test, set_and_get_test) {
    Value num_key = ValueOP::number_val(10);
    Value string_key = ValueOP::obj_val(ObjString::copyString(&vm, "test", 4));
    Value bool_key = ValueOP::bool_val(false);
    Value nul_key = ValueOP::nul_val();
    
    table.tableSet(num_key, ValueOP::number_val(1));
    table.tableSet(string_key, ValueOP::number_val(2));
    table.tableSet(bool_key, ValueOP::number_val(3));
    table.tableSet(nul_key, ValueOP::number_val(4));
    
    Value res;
    ASSERT_TRUE(table.tableGet(num_key, &res));
    ASSERT_TRUE(ValueOP::is_number(res));
    EXPECT_EQ(ValueOP::as_number(res), 1);
    
    ASSERT_TRUE(table.tableGet(string_key, &res));
    ASSERT_TRUE(ValueOP::is_number(res));
    EXPECT_EQ(ValueOP::as_number(res), 2);
    
    ASSERT_TRUE(table.tableGet(bool_key, &res));
    ASSERT_TRUE(ValueOP::is_number(res));
    EXPECT_EQ(ValueOP::as_number(res), 3);
    
    ASSERT_TRUE(table.tableGet(nul_key, &res));
    EXPECT_TRUE(ValueOP::is_number(res));
    EXPECT_EQ(ValueOP::as_number(res), 4);
}

TEST_F(Table_test, override_test) {
    Value string_key = ValueOP::obj_val(ObjString::copyString(&vm, "test", 4));
    table.tableSet(string_key, ValueOP::number_val(5));
    
    Value res;
    ASSERT_TRUE(table.tableGet(string_key, &res));
    
    ASSERT_TRUE(ValueOP::is_number(res));
    EXPECT_EQ(ValueOP::as_number(res), 5);
    
    table.tableSet(string_key, ValueOP::number_val(35));
    
    ASSERT_TRUE(table.tableGet(string_key, &res));
    
    ASSERT_TRUE(ValueOP::is_number(res));
    EXPECT_EQ(ValueOP::as_number(res), 35);
}

TEST_F(Table_test, delete_test) {
    Value string_key = ValueOP::obj_val(ObjString::copyString(&vm, "test", 4));
    table.tableSet(string_key, ValueOP::number_val(10));
    
    Value res;
    ASSERT_TRUE(table.tableGet(string_key, &res));
    ASSERT_TRUE(ValueOP::is_number(res));
    EXPECT_EQ(ValueOP::as_number(res), 10);
    
    ASSERT_TRUE(table.tableDelete(string_key));
    EXPECT_FALSE(table.tableGet(string_key, &res));
}

TEST_F(Table_test, find_string_test) {
    EXPECT_EQ(table.tableFindString("no", 2, ObjString::hashString("no", 2)), nullptr);
    ObjString* name = ObjString::copyString(&vm, "test_test", 9);
    Value name_val = ValueOP::obj_val(name);
    vm.push_stack(name_val);
    
    table.tableSet(name_val, ValueOP::number_val(123));
    ObjString* res = table.tableFindString("test_test", 9, ObjString::hashString("test_test", 9));
    ASSERT_NE(res, nullptr);
    EXPECT_STREQ(res->chars.c_str(), "test_test");
}

class Compiler_test {
protected:
    Scanner scan;
    Parser parse;
    
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
