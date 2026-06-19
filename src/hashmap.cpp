#include "../include/hashmap.hpp"
#include <cstdlib>

HashMap::HashMap(int cap) {
    capacity = cap;
    size = 0;
    buckets = new entry_t*[capacity];
    for (int i = 0; i < capacity; i++)
        buckets[i] = nullptr;
}

HashMap::~HashMap() {
    for (int i = 0; i < capacity; i++) {
        entry_t* e = buckets[i];
        while (e) {
            entry_t* next = e->next;
            delete e;
            e = next;
        }
    }
    delete[] buckets;
}

unsigned long HashMap::hash(const string& key) {
    unsigned long h = 5381;
    for (char c : key)
        h = ((h << 5) + h) + c;
    return h;
}

void HashMap::put(const string& key, const value_t& value) {
    unsigned long h = hash(key);
    unsigned long idx = h % capacity;
    entry_t* e = buckets[idx];
    
    // Check if key exists
    while (e) {
        if (e->hash == h && e->key == key) {
            e->value = value;
            return;
        }
        e = e->next;
    }
    
    // Insert new entry
    entry_t* node = new entry_t(key, value, h);
    node->next = buckets[idx];
    buckets[idx] = node;
    size++;
    
    if (size > (int)(capacity * LOAD_FACTOR)) {
        resize(capacity * 2);
    }
}

entry_t* HashMap::get(const string& key) {
    unsigned long h = hash(key);
    unsigned long idx = h % capacity;
    entry_t* e = buckets[idx];
    
    while (e) {
        if (e->hash == h && e->key == key)
            return e;
        e = e->next;
    }
    
    return nullptr;
}

void HashMap::remove(const string& key) {
    unsigned long h = hash(key);
    unsigned long idx = h % capacity;
    entry_t* e = buckets[idx];
    entry_t* prev = nullptr;
    
    while (e) {
        if (e->hash == h && e->key == key) {
            if (!prev)
                buckets[idx] = e->next;
            else
                prev->next = e->next;
                
            delete e;
            size--;
            return;
        }
        prev = e;
        e = e->next;
    }
}

int HashMap::get_capacity() {
    return capacity;
}

int HashMap::get_size() {
    return size;
}

entry_t** HashMap::get_buckets() {
    return buckets;
}

void HashMap::resize(int new_capacity) {
    entry_t** new_buckets = new entry_t*[new_capacity];
    for (int i = 0; i < new_capacity; i++)
        new_buckets[i] = nullptr;
    
    for (int i = 0; i < capacity; i++) {
        entry_t* e = buckets[i];
        while (e) {
            entry_t* next = e->next;
            unsigned long new_idx = e->hash % new_capacity;
            e->next = new_buckets[new_idx];
            new_buckets[new_idx] = e;
            e = next;
        }
    }
    
    delete[] buckets;
    buckets = new_buckets;
    capacity = new_capacity;
}

HashMap* HashMap::clone() {
    HashMap* copy = new HashMap(capacity);
    copy->size = size;
    
    // Direct bucket copy without rehashing
    for (int i = 0; i < capacity; i++) {
        entry_t* e = buckets[i];
        entry_t* prev = nullptr;
        
        while (e) {
            entry_t* new_entry = new entry_t(e->key, e->value, e->hash);
            
            if (!prev)
                copy->buckets[i] = new_entry;
            else
                prev->next = new_entry;
            
            prev = new_entry;
            e = e->next;
        }
    }
    
    return copy;
}