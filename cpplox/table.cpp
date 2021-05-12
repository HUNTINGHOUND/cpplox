#include "table.hpp"
#include "memory.hpp"
#include "object.hpp"


Table::Table() {
    count = 0;
    capacity = 0;
    entries = std::vector<Entry>();
}

void Table::freeTable() {
}

bool Table::tableSet(Value key, Value value) {
    if(count + 1 > capacity * TABLE_MAX_LOAD) {
        int newCapacity = grow_capacity(capacity);
        adjustCapacity(newCapacity);
    }
    
    Entry* entry = findEntry(entries, key);
    
    bool isNewKey = entry->key.type == VAL_EMPTY;
    if (isNewKey && Value::is_nul(entry->value)) count++;
    
    entry->key = key;
    entry->value = value;
    return isNewKey;
}

Entry* Table::findEntry(std::vector<Entry>& entries, Value key) {
    uint32_t index = Value::hashValue(key) % entries.capacity();
    Entry* tombstone = nullptr;
    
    while(true) {
        Entry* entry = &entries[index];
        
        if(Value::is_empty(entry->key)) {
            if(Value::is_nul(entry->value)) {
                return tombstone != nullptr ? tombstone : entry;
            } else {
                if (tombstone == nullptr) tombstone = entry;
            }
        } else if(Value::valuesEqual(key, entry->key)) {
            
            return entry;
        }
        
        index = (index + 1) % capacity;
    }
}

void Table::adjustCapacity(int capacity) {
    std::vector<Entry> newEntries(capacity);
    for(int i = 0; i < capacity; i++) {
        newEntries[i].key = Value::empty_val();
        newEntries[i].value = Value::nul_val();
    }
    
    count = 0;
    for (int i = 0; i < this->capacity; i++) {
        Entry* entry = &entries[i];
        if(Value::is_empty(entry->key)) continue;
        
        Entry* dest = findEntry(newEntries, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        count++;
    }
    
    this->entries = newEntries;
    this->capacity = capacity;
    
}

void Table::tableAddAll(Table* from) {
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if(Value::is_empty(entry->key)) {
            tableSet(entry->key, entry->value);
        }
    }
}

bool Table::tableGet(Value key, Value *value) {
    if (count == 0) return false;
    
    Entry* entry = findEntry(entries, key);
    if(Value::is_empty(entry->key)) return false;
    
    *value = entry->value;
    return true;
}

bool Table::tableDelete(Value key) {
    if (count == 0) return false;
    
    Entry* entry = findEntry(entries, key);
    if(Value::is_empty(entry->key)) return false;
    
    entry->key = Value::empty_val();
    entry->value = Value::bool_val(true);
    
    return true;
}

ObjString* Table::tableFindString(const char *chars, int length, uint32_t hash) {
    if (count == 0) return nullptr;
    
    uint32_t index = hash % capacity;
    
    while(true) {
        Entry* entry = &entries[index];
        if (Value::is_empty(entry->key)) {
            if (Value::is_nul(entry->value)) return nullptr;
        } else if(Value::is_obj(entry->key) && Value::is_string(entry->key)){
            
            ObjString* key = Value::as_string(entry->key);
            
            if (key->length == length && key->hash == hash &&
                memcmp(key->chars, chars, length) == 0) {
                return key;
            }
        }
        
        index = (index + 1) % capacity;
    }
}

void Table::removeWhite(VM* vm) {
    for(int i = 0; i < capacity; i++) {
        Entry* entry = &entries[i];
        if(Value::is_empty(entry->key) &&
           Value::as_obj(entry->key)->mark != vm->marker) {
            tableDelete(entry->key);
        }
    }
}
