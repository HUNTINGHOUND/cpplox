//
//  table.hpp
//  cpplox
//
//  Created by Morgan Xu on 3/13/21.
//

#ifndef table_h
#define table_h

#include "flags.hpp"
#include "value.hpp"
#include <vector>

#define TABLE_MAX_LOAD 0.65

struct Entry {
    Value key;
    Value value;
};

class Table {
    
    Entry* findEntry(std::vector<Entry>& entries, Value key);
    void adjustCapacity(int capacity);
    
public:
    
    int count;
    int capacity;
    std::vector<Entry> entries;
    

    
    Table();
    void freeTable();
    bool tableSet(Value key, Value value);
    void tableAddAll(Table* from);
    bool tableGet(Value key, Value* value);
    bool tableDelete(Value key);
    ObjString* tableFindString(const char* chars, int length, uint32_t hash);
};

#endif /* table_h */