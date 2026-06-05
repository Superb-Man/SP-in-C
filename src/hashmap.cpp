#include "../include/hashmap.hpp"
#include <cstdlib>

HashMap::HashMap(int cap) {
    capacity = cap;
    buckets = (entry_t**)malloc(sizeof(entry_t*) * capacity);
    for (int i = 0; i < capacity; i++)
        buckets[i] = NULL;
}

HashMap::~HashMap() {
    for (int i = 0; i < capacity; i++) {
        entry_t* e = buckets[i];
        while (e) {
            entry_t* next = e->next;
            e->~entry_t();
            free(e);
            e = next;
        }
    }
    free(buckets);
}

unsigned long HashMap::hash(const string& key) {
    unsigned long h = 5381;
    for (char c : key)
        h = ((h << 5) + h) + c;
    return h;
}

void HashMap::put(const string& key, const value_t& value) {
    unsigned long idx = hash(key) % capacity;
    entry_t* e = buckets[idx];
    
    while (e) {
        if (e->key == key) {
            e->value = value;
            return;
        }
        e = e->next;
    }
    
    entry_t* node = new (malloc(sizeof(entry_t))) entry_t(key, value);
    node->next = buckets[idx];
    buckets[idx] = node;
}

entry_t* HashMap::get(const string& key) {
    unsigned long idx = hash(key) % capacity;
    entry_t* e = buckets[idx];
    
    while (e) {
        if (e->key == key)
            return e;
        e = e->next;
    }
    
    return NULL;
}

void HashMap::remove(const string& key) {
    unsigned long idx = hash(key) % capacity;
    entry_t* e = buckets[idx];
    entry_t* prev = NULL;
    
    while (e) {
        if (e->key == key) {
            if (!prev)
                buckets[idx] = e->next;
            else
                prev->next = e->next;
                
            e->~entry_t();
            free(e);
            return;
        }
        prev = e;
        e = e->next;
    }
}

int HashMap::get_capacity() {
    return capacity;
}

entry_t** HashMap::get_buckets() {
    return buckets;
}

HashMap* HashMap::clone() {
    HashMap* copy = new (malloc(sizeof(HashMap))) HashMap(capacity);
    
    for (int i = 0; i < capacity; i++) {
        entry_t* e = buckets[i];
        while (e) {
            copy->put(e->key, e->value);
            e = e->next;
        }
    }
    
    return copy;
}