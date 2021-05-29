#include "table.hpp"
#include "memory.hpp"
#include "object.hpp"


Table::Table(VM* vm) {
    this->vm = vm;
    count = 0;
    capacity = 0;
    entries = nullptr;
}

void Table::freeTable() {
    free_array(entries, capacity, vm);
}

bool Table::tableSet(Value key, Value value) {
    if(count + 1 > capacity * TABLE_MAX_LOAD) {
        size_t newCapacity = grow_capacity(capacity);
        adjustCapacity(newCapacity);
    }
    
    Entry* entry = findEntry(entries, key, this->capacity);
    
    bool isNewKey = ValueOP::is_empty(key);
    if (isNewKey && ValueOP::is_nul(entry->value)) count++;
    
    entry->key = key;
    entry->value = value;
    return isNewKey;
}

Entry* Table::findEntry(Entry* entries, Value key, size_t capacity) {
    uint32_t index = ValueOP::hashValue(key) & (capacity - 1);
    Entry* tombstone = nullptr;
    
    while(true) {
        Entry* entry = &entries[index];
        
        if(ValueOP::is_empty(entry->key)) {
            if(ValueOP::is_nul(entry->value)) {
                return tombstone != nullptr ? tombstone : entry;
            } else {
                if (tombstone == nullptr) tombstone = entry;
            }
        } else if(ValueOP::valuesEqual(key, entry->key)) {
            
            return entry;
        }
        
        index = (index + 1) & (capacity - 1);
    }
}

void Table::adjustCapacity(size_t newCapacity) {
    Entry* newEntries = allocate<Entry>(newCapacity, vm);
    for(int i = 0; i < newCapacity; i++) {
        newEntries[i].key = ValueOP::empty_val();
        newEntries[i].value = ValueOP::nul_val();
    }
    
    count = 0;
    for (int i = 0; i < this->capacity; i++) {
        Entry* entry = &entries[i];
        if(ValueOP::is_empty(entry->key)) continue;
        
        Entry* dest = findEntry(newEntries, entry->key, newCapacity);
        dest->key = entry->key;
        dest->value = entry->value;
        count++;
    }
    
    this->entries = newEntries;
    this->capacity = newCapacity;
    
}

void Table::tableAddAll(Table* from) {
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if(ValueOP::is_empty(entry->key)) {
            tableSet(entry->key, entry->value);
        }
    }
}

bool Table::tableGet(Value key, Value *value) {
    if (count == 0) return false;
    
    Entry* entry = findEntry(entries, key, this->capacity);
    if(ValueOP::is_empty(entry->key)) return false;
    
    *value = entry->value;
    return true;
}

bool Table::tableDelete(Value key) {
    if (count == 0) return false;
    
    Entry* entry = findEntry(entries, key, this->capacity);
    if(ValueOP::is_empty(entry->key)) return false;
    
    entry->key = ValueOP::empty_val();
    entry->value = ValueOP::bool_val(true);
    
    return true;
}

ObjString* Table::tableFindString(const char *chars, size_t length, uint32_t hash) {
    if (count == 0) return nullptr;
    
    uint32_t index = hash & (capacity - 1);
    
    while(true) {
        Entry* entry = &entries[index];
        if (ValueOP::is_empty(entry->key)) {
            if (ValueOP::is_nul(entry->value)) return nullptr;
        } else if(ValueOP::is_obj(entry->key) && ValueOP::is_string(entry->key)){
            
            ObjString* key = ValueOP::as_string(entry->key);
            
            if (key->length == length && key->hash == hash &&
                memcmp(key->chars, chars, length) == 0) {
                return key;
            }
        }
        
        index = (index + 1) & (capacity - 1);
    }
}

void Table::removeWhite(VM* vm) {
    for(int i = 0; i < capacity; i++) {
        Entry* entry = &entries[i];
        if(!ValueOP::is_empty(entry->key) &&
           ValueOP::as_obj(entry->key)->mark != vm->marker) {
            tableDelete(entry->key);
        }
    }
}
