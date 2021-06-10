#include "fileio.hpp"
#include "terminal.hpp"
#include "output.hpp"
#include "input.hpp"

void FileIO::editorOpen(std::string& filename) {
    E.filename = filename;
    
    Output::editorSelectSyntaxHighlight();
    
    std::fstream file(filename);
    if(!file.is_open()) Terminal::kill("fstream open");
    
    std::string line("");
    while(std::getline(file, line)) {
        while (line.size() > 0 && (line.back() == '\n' ||
                                   line.back() == '\r'))
            line.pop_back();
        E.row.push_back(Erow{{},line, ""});
        Output::editorUpdateRows(E.row.back());
        E.numsrows++;
    }
    file.close();
}

void FileIO::editorRowstoString(std::string* buffer) {
    int rowsread = 0;
    while(rowsread < E.numsrows) {
        buffer->append(E.row[rowsread++].chars);
        buffer->append("\r\n");
    }
    
}

void FileIO::editorSave() {
    if(E.filename.empty()) {
        E.filename = Input::editorPrompt("Save as: % (ESC to cancel)", nullptr);
        std::fstream log("log.txt", std::fstream::out);
        log << "executed out";
        log.close();
        if(E.filename.empty()) {
            Output::editorSetStatusMessage("Save aborted.", {});
            return;
        }
        
        Output::editorSelectSyntaxHighlight();
    }
    
    std::string buffer = "";
    editorRowstoString(&buffer);
    
    std::fstream file(E.filename, std::fstream::out | std::fstream::trunc);
    if (file.is_open()) {
        file << buffer;
        file.close();
        E.dirty = 0;
        Output::editorSetStatusMessage("File '%' saved to disk", {E.filename});
        return;
    }
    
    Output::editorSetStatusMessage("Can't save file.", {});
}
