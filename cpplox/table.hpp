//
//  table.hpp
//  cpplox
//
//  Created by Morgan Xu on 3/13/21.
//

#ifndef table_h
#define table_h

#include "flags.hpp"
#include <vector>

#ifdef NAN_BOXING
#include "nanvalue.hpp"
#else
#include "value.hpp"
#endif

#define TABLE_MAX_LOAD 0.65

struct Entry {
    Value key = ValueOP::empty_val();
    Value value = ValueOP::empty_val();
};

class Table {
    
    VM* vm;
    Entry* findEntry(Entry* entries, Value key, size_t capacity);
    void adjustCapacity(size_t capacity);
    
public:
    
    int count;
    size_t capacity;
    
    Entry* entries;
    

    
    Table(VM* vm);
    void freeTable();
    bool tableSet(Value key, Value value);
    void tableAddAll(Table* from);
    bool tableGet(Value key, Value* value);
    bool tableDelete(Value key);
    void removeWhite(VM* vm);
    ObjString* tableFindString(const char* chars, size_t length, uint32_t hash);
};

#endif /* table_h */
