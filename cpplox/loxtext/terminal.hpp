
#ifndef terminal_hpp
#define terminal_hpp

#include "includes.hpp"

struct EditorSyntax {
    std::vector<std::string> filematch;
    std::vector<std::string> keywords;
    std::string filetype;
    std::string singleline_comment_start;
    int flags;
};

enum editorHighLight {
    HL_NORMAL = 0,
    HL_NUMBER,
    HL_MATCH,
    HL_STRING,
    HL_COMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2
};

struct Erow {
    std::vector<uint8_t> hl;
    std::string chars;
    std::string render;
};


struct EditorConfig {
    std::vector<Erow> row;
    std::string statusmsg;
    std::string filename;
    termios og_termios;
    EditorSyntax* syntax;
    time_t statusmsg_time;
    int cx, cy;
    int rx;
    int rowoff;
    int coloff;
    int screenrows;
    int screencols;
    int numsrows;
    int dirty;
};

extern EditorConfig E;

enum editorKey {
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    PAGE_UP,
    PAGE_DOWN,
    HOME_KEY,
    END_KEY,
    DEL_KEY
};

namespace Terminal {
void kill(const std::string& errorMessage);
char ctrl_key(char k);
void enableRawMode();
int editorReadKey();
void editorProcessKeypress();
int getWindowSize(int* rows, int* cols);
void initEditor();
int getCursorPosition(int* rows, int* cols);
};

#endif /* terminal_hpp */
