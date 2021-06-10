#include "input.hpp"
#include "terminal.hpp"
#include "editorOP.hpp"
#include "fileio.hpp"
#include "output.hpp"
#include "find.hpp"

void Input::editorProcessKeypress() {
    static int quit_times = QUIT_TIMES;
    int c = Terminal::editorReadKey();
    
    if(c == Terminal::ctrl_key('q')) {
        if(E.dirty && quit_times > 0) {
            Output::editorSetStatusMessage("WARNING: File has unsaved changes. Press Ctrl-Q % more times to quit.", {std::to_string(quit_times)});
            quit_times--;
            return;
        }
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
    } else if (c == Terminal::ctrl_key('h')) {
        EditorOP::editorDelChar();
        return;
    } else if (c == Terminal::ctrl_key('l')) {
        return;
    } else if (c == Terminal::ctrl_key('s')) {
        FileIO::editorSave();
        return;
    } else if (c == Terminal::ctrl_key('f')) {
        Find::editorFind();
        return;
    }
    
    switch (c) {
        case '\r':
            EditorOP::editorInsertNewLine();
            break;
            
        case PAGE_UP:
        case PAGE_DOWN: {
            if (c == PAGE_UP) {
                E.cy = E.rowoff;
            } else if (c == PAGE_DOWN) {
                E.cy = E.rowoff + E.screenrows - 1;
                if(E.cy > E.numsrows) E.cy = E.numsrows;
            }
            
            int times = E.screenrows;
            while(times--) {
                editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            
            break;
        }
            
        case ARROW_LEFT:
        case ARROW_RIGHT:
        case ARROW_UP:
        case ARROW_DOWN:
            editorMoveCursor(c);
            break;
            
        case HOME_KEY:
            E.cx = 0;
            break;
        case END_KEY:
            if(E.cy < E.numsrows)
                E.cx = (int)E.row[E.cy].chars.size();
            break;
            
        case BACKSPACE:
        case DEL_KEY:
            if(c == DEL_KEY) editorMoveCursor(ARROW_RIGHT);
            EditorOP::editorDelChar();
            break;
            
        case '\x1b':
            break;
            
            
        default:
            EditorOP::editorInsertChar(c);
            break;
    }
    
    quit_times = QUIT_TIMES;
}

void Input::editorMoveCursor(int key) {
    Erow* row = (E.cy >= E.numsrows) ? nullptr : &E.row[E.cy];
    
    switch (key) {
        case ARROW_LEFT:
            if(E.cx > 0) {
                E.cx--;
            } else if(E.cy > 0) {
                E.cy--;
                E.cx = (int)E.row[E.cy].chars.size();
            }
            break;
        case ARROW_RIGHT:
            if(row && E.cx < row->chars.size()) {
                E.cx++;
            } else if(row && E.cx == row->chars.size()) {
                E.cy++;
                E.cx = 0;
            }
            break;
        case ARROW_UP:
            if(E.cy > 0) E.cy--;
            break;
        case ARROW_DOWN:
            if(E.cy < E.numsrows) E.cy++;
            break;
    }
    
    row = (E.cy >= E.numsrows) ? nullptr : &E.row[E.cy];
    int rowlen = row ? (int)row->chars.size() : 0;
    if(E.cx > rowlen) {
        E.cx = rowlen;
    }
}

std::string Input::editorPrompt(const std::string& prompt, std::function<void(std::string&, int)>* callback) {
    std::string buf = "";
    while(true) {
        Output::editorSetStatusMessage(prompt, {buf});
        Output::editorRefreashScreen();
        
        int c = Terminal::editorReadKey();
        if(c == DEL_KEY || c == Terminal::ctrl_key('h') || c == BACKSPACE) {
            if(buf.size() != 0) buf.pop_back();
        } else if(c == '\x1b') {
            Output::editorSetStatusMessage("", {});
            if(callback) (*callback)(buf, c);
            return "";
        } else if(c == '\r') {
            if(buf.size() != 0) {
                Output::editorSetStatusMessage("", {});
                if(callback) (*callback)(buf, c);
                std::fstream log("log.txt", std::fstream::out);
                log << "executed";
                log.close();
                return buf;
            }
        } else if(!iscntrl(c) && c < 128) {
            buf += (char)c;
        }
        
        if(callback) (*callback)(buf,c);
    }
    
}
