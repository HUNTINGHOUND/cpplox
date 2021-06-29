#include "output.hpp"
#include "filetypes.hpp"


void Output::editorScroll() {
    E.rx = E.cx;
    if(E.cy < E.numsrows) {
        E.rx = editorRowCxtoRx(E.row[E.cy], E.cx);
    }
    
    if(E.cy < E.rowoff) {
        E.rowoff = E.cy;
    }
    if(E.cy >= E.rowoff + E.screenrows) {
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if(E.rx < E.coloff) {
        E.coloff = E.rx;
    }
    if(E.rx >= E.coloff + E.screencols) {
        E.coloff = E.rx - E.screencols + 1;
    }
}

void Output::editorRefreashScreen() {
    editorScroll();
    std::string buffer = "";
    
    buffer += "\x1b[?25l";
    buffer += "\x1b[H";
    
    editorDrawRows(&buffer);
    editorDrawStatusBar(&buffer);
    editorDrawMessageBar(&buffer);
    
    buffer += "\x1b[";
    buffer += std::to_string((E.cy - E.rowoff) + 1);
    buffer += ";";
    buffer += std::to_string((E.rx - E.coloff) + 1);
    buffer += "H";
    
    buffer += "\x1b[?25h";
    
    write(STDOUT_FILENO, buffer.c_str(), buffer.size());
}

void Output::editorDrawRows(std::string* buffer) {
    int y;
    for (y = 0; y < E.screenrows; y++) {
        int filerow = y + E.rowoff;
        if(filerow >= E.numsrows) {
            if(E.numsrows == 0 && y == E.screenrows / 3) {
                std::string welcome = "Lox builtin text editor -- version ";
                welcome += EDITOR_VERSION;
                
                int welcomelen = (int)welcome.size();
                if(welcome.size() < E.screencols) welcomelen = E.screencols;
                
                int padding = (E.screencols - welcomelen) / 2;
                if(padding) {
                    buffer->append("~");
                    padding--;
                }
                while(padding--) buffer->append(" ");
                
                buffer->append(welcome.substr(0,welcomelen));
                
            } else {
                buffer->append("~");
            }
        } else {
            int len = (int)E.row[filerow].render.size() - E.coloff;
            if(len < 0) len = 0;
            if(len > E.screencols) len = E.screencols;
            
            std::string_view tobeprinted = E.row[filerow].render;
            auto hlit = E.row[filerow].hl.begin() + E.coloff;
            tobeprinted.remove_prefix(E.coloff);
            
            
            int curr_color = -1;
            for(int i = 0; i < len; i++) {
                if(iscntrl(tobeprinted[i])) {
                    char sym = (tobeprinted[i] <= 26) ? '@' + tobeprinted[i] : '?';
                    buffer->append("\x1b[7m");
                    buffer->push_back(sym);
                    buffer->append("\x1b[m");
                    
                    if(curr_color != -1) {
                        std::string buf;
                        buf += "\x1b[";
                        buf += std::to_string(curr_color);
                        buf += "m";
                        
                        buffer->append(buf);
                    }
                } else if(*hlit == HL_NORMAL) {
                    if(curr_color != -1) {
                        buffer->append("\x1b[39m");
                        curr_color = -1;
                    }
                    buffer->push_back(tobeprinted[i]);
                } else {
                    int color = editorSyntaxToColor(*hlit);
                    if(color != curr_color) {
                        curr_color = color;
                        std::string buf = "";
                        
                        buf += "\x1b[";
                        buf += std::to_string(color);
                        buf += "m";
                        
                        buffer->append(buf);
                    }
                    buffer->push_back(tobeprinted[i]);
                }
                hlit++;
            }
            
            buffer->append("\x1b[39m");
        }
        
        buffer->append("\x1b[K");
        buffer->append("\r\n");
    }
}

void Output::editorUpdateRows(Erow& row) {
    row.render = "";
    
    for(int j = 0; j < row.chars.size(); j++) {
        if(row.chars[j] == '\t') {
            row.render += ' ';
            while(row.render.size() % tablen != 0) row.render += ' ';
        } else {
            row.render += row.chars[j];
        }
    }
    
    editorUpdateSyntax(row);
}

int Output::editorRowCxtoRx(Erow &row, int cx) {
    int rx = 0;
    int j;
    for(j = 0; j < cx; j++) {
        if(row.chars[j] == '\t')
            rx += (tablen - 1) - (rx % tablen);
        rx++;
    }
    
    return rx;
}

void Output::editorDrawStatusBar(std::string* buffer) {
    buffer->append("\x1b[7m");
    
    std::stringstream status, rstatus;
    status << std::setw(20) << (E.filename.empty() ? "[No Name]" : E.filename) << " - " << E.numsrows << " lines "
    << (E.dirty != 0 ? "(modified)" : "");
    std::string statusStr = status.str();
    int len = (int)statusStr.size();
    
    rstatus << (E.syntax ? E.syntax->filetype : "no ft") << " | " << E.cy + 1 << "/" << E.numsrows;
    std::string rstatusStr = rstatus.str();
    int rlen = (int)rstatusStr.size();
    
    if(len > E.screencols) len = E.screencols;
    buffer->append(statusStr.substr(0, len));
    
    while(len < E.screencols) {
        if (E.screencols - len == rlen) {
            buffer->append(rstatusStr);
            break;
        } else {
            buffer->append(" ");
            len++;
        }
    }
    
    buffer->append("\x1b[m");
    buffer->append("\r\n");
}

void Output::editorSetStatusMessage(const std::string& fmt, std::vector<std::string> args) {
    E.statusmsg = "";
    int argsi = 0;
    for(int i = 0; i < fmt.size(); i++) {
        if(fmt[i] == '%') {
            E.statusmsg += args[argsi++];
        } else {
            E.statusmsg += fmt[i];
        }
    }
    E.statusmsg_time = time(nullptr);
}

void Output::editorDrawMessageBar(std::string *buffer) {
    buffer->append("\x1b[K");
    int msglen = (int)E.statusmsg.size();
    if(msglen > E.screencols) msglen = E.screencols;
    if(msglen && time(nullptr) - E.statusmsg_time < 5)
        buffer->append(E.statusmsg.substr(0,msglen));
}

void Output::editorRowInsertChar(Erow& row, int at, int c) {
    if(at < 0 || at > row.chars.size()) at = row.chars.size();
    
    std::string tobeinserted = "";
    tobeinserted += (char)c;
    row.chars.insert(at, tobeinserted);
    editorUpdateRows(row);
}

void Output::editorRowDelChar(Erow& row, int at) {
    if(at < 0 || at >= row.chars.size()) return;
    row.chars.erase(row.chars.begin() + at);
    editorUpdateRows(row);
    E.dirty++;
}

void Output::editorRowAppendString(Erow& row, std::string& string) {
    row.chars += string;
    editorUpdateRows(row);
    E.dirty++;
}

int Output::editorRowRxToCx(Erow& row, int rx) {
    int cur_rx = 0;
    for(int cx = 0; cx < row.chars.size(); cx++) {
        if(row.chars[cx] == '\t') {
            cur_rx += (tablen - 1) - (cur_rx % tablen);
        }
        
        cur_rx++;
        
        if(cur_rx > rx) return cx;
    }
    
    return (int)row.chars.size();
}

void Output::editorUpdateSyntax(Erow& row) {
    row.hl = std::vector<uint8_t>(row.render.size());
    std::fill(row.hl.begin(), row.hl.end(), HL_NORMAL);
    
    if(E.syntax == nullptr) return;
    
    
    std::string_view scs = E.syntax->singleline_comment_start;
    std::string_view renderView = row.render;
    
    bool prev_sep = true;
    int in_string = 0;
    
    int i = 0;
    while(i < row.hl.size()) {
        uint8_t prev_hl = (i > 0) ? row.hl[i - 1] : HL_NORMAL;
        
        if(scs.size() != 0 && in_string == 0) {
            if(renderView.substr(i, scs.length()).compare(scs) == 0) {
                std::fill(row.hl.begin() + i, row.hl.end(), HL_COMMENT);
                break;
            }
        }
        
        if(E.syntax->flags & hl_highlight_strings) {
            if(in_string != 0) {
                row.hl[i] = HL_STRING;
                if(renderView[i] == '\\' && i + 1 < row.render.size()) {
                    row.hl[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if(renderView[i] == in_string) in_string = 0;
                i++;
                prev_sep = true;
                continue;
            } else {
                if(renderView[i] == '"' || renderView[i] == '\'') {
                    in_string = renderView[i];
                    row.hl[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }
        
        if(E.syntax->flags & hl_highlight_numbers) {
            if((isdigit(renderView[i]) && (prev_sep || prev_hl == HL_NUMBER)) ||
               (renderView[i] == '.' && prev_hl == HL_NUMBER)) {
                row.hl[i] = HL_NUMBER;
                i++;
                prev_sep = 0;
                continue;
            }
        }
        
        
        if(prev_sep) {
            auto keywordIt = E.syntax->keywords.begin();
            for(;keywordIt != E.syntax->keywords.end(); keywordIt++) {
                
                std::string_view word = *keywordIt;
                bool kw2 = word.back() == '|';
                if(kw2) word.remove_suffix(1);;
                
                if(i + word.size() <= row.render.size() &&
                   word.compare(row.render.substr(i, word.size())) == 0 &&
                   is_seperator(row.render[i + word.size()])) {
                
                    std::fill(row.hl.begin() + i, row.hl.begin() + i + word.size(), kw2 ? HL_KEYWORD2 : HL_KEYWORD1);
                    i += word.size();
                    break;
                }
            }
            
            prev_sep = false;
        }
        
        prev_sep = is_seperator(renderView[i]);
        i++;
    }
    
}

int Output::editorSyntaxToColor(int hl) {
    switch (hl) {
        case HL_KEYWORD2: return 32;
        case HL_KEYWORD1: return 33;
        case HL_COMMENT: return 26;
        case HL_STRING: return 35;
        case HL_NUMBER: return 31;
        default: return 37;
    }
}

bool Output::is_seperator(int c) {
    return std::isspace(c) || c == '\0' || std::strchr(",.()+-/*=~%<>[];", c) != nullptr;
}

void Output::editorSelectSyntaxHighlight() {
    E.syntax = nullptr;
    if (E.filename.empty()) return;
    
    
    std::string_view fname = E.filename;
    size_t pos = fname.find('.');
    
    for(unsigned int i = 0; i < HLDB.size(); i++) {
        EditorSyntax* s = &HLDB[i];
        for(unsigned int j = 0; j < s->filematch.size(); j++) {
            bool is_ext = (s->filematch[j][0] == '.');
            if((is_ext && pos != std::string::npos && s->filematch[j].compare(fname.substr(pos, std::string::npos)) == 0) ||
               (!is_ext && fname.find(s->filematch[j]) != std::string::npos)) {
                E.syntax = s;
                
                for(int filerow = 0; filerow < E.numsrows; filerow++) {
                    editorUpdateSyntax(E.row[filerow]);
                }
                
                return;
            }
        }
    }
}
