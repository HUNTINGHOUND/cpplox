//
//  editorOP.cpp
//  loxtext
//
//  Created by Morgan Xu on 6/2/21.
//


#include "editorOP.hpp"
#include "output.hpp"

void EditorOP::editorInsertChar(int c) {
    if(E.cy == E.numsrows) {
        E.row.push_back(Erow{{},"", ""});
        E.numsrows++;
        Output::editorUpdateRows(E.row.back());
    }
    
    Output::editorRowInsertChar(E.row[E.cy], E.cx, c);
    E.cx++;
    E.dirty++;
}

void EditorOP::editorDelChar() {
    if(E.cy == E.numsrows) return;
    if(E.cx == 0 && E.cy == 0) return;
    
    Erow* row = &E.row[E.cy];
    if(E.cx > 0) {
        Output::editorRowDelChar(*row, E.cx - 1);
        E.cx--;
    } else {
        E.cx = E.row[E.cy - 1].chars.size();
        Output::editorRowAppendString(E.row[E.cy - 1], row->chars);
        E.row.erase(E.row.begin() + E.cy);
        E.numsrows--;
        E.cy--;
    }
}

void EditorOP::editorInsertNewLine() {
    if(E.cx == 0) {
        E.row.insert(E.row.begin() + E.cy, Erow{{},"", ""});
        Output::editorUpdateRows(E.row[E.cy]);
    } else {
        Erow* row = &E.row[E.cy];
        
        if(E.cx != row->chars.size()) {
            std::fstream log("log.txt", std::fstream::out);
            log << row->chars;
            log.close();
            E.row.insert(E.row.begin() + E.cy + 1, Erow{{},row->chars.substr(E.cx, row->chars.size() - E.cx),""});
            row = &E.row[E.cy];
            row->chars.erase(E.cx, row->chars.size() - E.cx);
        } else {
            E.row.insert(E.row.begin() + E.cy + 1, Erow{{},"",""});
        }
        
        Output::editorUpdateRows(E.row[E.cy]);
        Output::editorUpdateRows(E.row[E.cy + 1]);
    }
    
    E.numsrows++;
    E.cy++;
    E.cx = 0;
    
}
