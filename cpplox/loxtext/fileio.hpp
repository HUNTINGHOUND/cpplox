//
//  fileio.hpp
//  loxtext
//
//  Created by Morgan Xu on 6/1/21.
//

#ifndef fileio_hpp
#define fileio_hpp

#include "includes.hpp"

namespace FileIO {
void editorOpen(std::string& filename);
void editorRowstoString(std::string* buffer);
void editorSave();
}


#endif /* fileio_hpp */
