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

TEST_F(ValueArray_test, write_test) {
    arr.writeValueArray(ValueOP::nul_val());
    arr.writeValueArray(ValueOP::number_val(123));
    
    EXPECT_EQ(arr.count, 2);
    EXPECT_TRUE(ValueOP::is_nul(arr.values[0]));
    EXPECT_TRUE(ValueOP::is_number(arr.values[1]));
    EXPECT_EQ(ValueOP::as_number(arr.values[1]), 123);
}


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
    
    testing::internal::CaptureStdout();
    ValueOP::printValue(true_value);
    buffer = testing::internal::GetCapturedStdout();
    EXPECT_STREQ(buffer.c_str(), "true");
    
    testing::internal::CaptureStdout();
    ValueOP::printValue(false_value);
    buffer = testing::internal::GetCapturedStdout();
    EXPECT_STREQ(buffer.c_str(), "false");

    
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
    
    testing::internal::CaptureStdout();
    ValueOP::printValue(num_value);
    std::string buffer = testing::internal::GetCapturedStdout();
    EXPECT_STREQ(buffer.c_str(), "123");
    
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
    
    testing::internal::CaptureStdout();
    ValueOP::printValue(string_val);
    std::string buffer = testing::internal::GetCapturedStdout();
    EXPECT_STREQ(buffer.c_str(), "test");
    
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
    EXPECT_STREQ(f->name->chars.c_str(), "abc");
    
    EXPECT_EQ(function_val.type, VAL_OBJ);
    EXPECT_TRUE(ValueOP::is_obj(function_val));
    EXPECT_EQ(f, ValueOP::as_function(function_val));
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
