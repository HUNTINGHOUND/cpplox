#include "table.hpp"
#include "memory.hpp"
#include "object.hpp"


Table::Table() {
    vm = nullptr;
}

Table::Table(VM* vm) {
    this->vm = vm;
    count = 0;
}


bool Table::tableSet(Value key, Value value) {
    if(count + 1 > this->entries.size() * TABLE_MAX_LOAD) {
        size_t newCapacity = grow_capacity(this->entries.capacity());
        adjustCapacity(newCapacity);
    }
    
    Entry* entry = findEntry(entries, key, this->entries.capacity());
    
    bool isNewKey = ValueOP::is_empty(entry->key);
    if (isNewKey && ValueOP::is_nul(entry->value)) count++;
    
    entry->key = key;
    entry->value = value;
    return isNewKey;
}

Entry* Table::findEntry(std::vector<Entry>& entries, Value key, size_t capacity) {
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
    std::vector<Entry> newEntries(newCapacity);
    for(int i = 0; i < newCapacity; i++) {
        newEntries[i].key = ValueOP::empty_val();
        newEntries[i].value = ValueOP::nul_val();
    }
    
    count = 0;
    for (int i = 0; i < this->entries.size(); i++) {
        Entry* entry = &entries[i];
        if(ValueOP::is_empty(entry->key)) continue;
        
        Entry* dest = findEntry(newEntries, entry->key, newCapacity);
        dest->key = entry->key;
        dest->value = entry->value;
        count++;
    }
    
    entries.swap(newEntries);
    
}

void Table::tableAddAll(Table* from) {
    for (int i = 0; i < from->entries.size(); i++) {
        Entry* entry = &from->entries[i];
        if(ValueOP::is_empty(entry->key)) {
            tableSet(entry->key, entry->value);
        }
    }
}

bool Table::tableGet(Value key, Value *value) {
    if (count == 0) return false;
    
    Entry* entry = findEntry(entries, key, entries.size());
    if(ValueOP::is_empty(entry->key)) return false;
    
    *value = entry->value;
    return true;
}

bool Table::tableDelete(Value key) {
    if (count == 0) return false;
    
    Entry* entry = findEntry(entries, key, entries.size());
    if(ValueOP::is_empty(entry->key)) return false;
    
    entry->key = ValueOP::empty_val();
    entry->value = ValueOP::bool_val(true);
    
    return true;
}

ObjString* Table::tableFindString(const char *chars, size_t length, uint32_t hash) {
    if (count == 0) return nullptr;
    
    uint32_t index = hash & (entries.size() - 1);
    
    while(true) {
        Entry* entry = &entries[index];
        if (ValueOP::is_empty(entry->key)) {
            if (ValueOP::is_nul(entry->value)) return nullptr;
        } else if(ValueOP::is_obj(entry->key) && ValueOP::is_string(entry->key)){
            
            ObjString* key = ValueOP::as_string(entry->key);
            
            if (key->chars.length() == length && key->hash == hash &&
                key->chars.compare(chars) == 0) {
                return key;
            }
        }
        
        index = (index + 1) & (entries.size() - 1);
    }
}

void Table::removeWhite(VM* vm) {
    for(int i = 0; i < entries.size(); i++) {
        Entry* entry = &entries[i];
        if(!ValueOP::is_empty(entry->key) &&
           ValueOP::as_obj(entry->key)->mark != vm->marker) {
            tableDelete(entry->key);
        }
    }
}
