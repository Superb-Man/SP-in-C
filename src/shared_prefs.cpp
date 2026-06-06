#include "../include/shared_prefs.hpp"
#include "../include/async.hpp"
#include <cstdlib>

SharedPreferences::SharedPreferences(const string& path) {
    pthread_mutex_init(&lock, NULL);
    map = (HashMap*)malloc(sizeof(HashMap)); 
    new (map) HashMap(256);
    storage = (Storage*)malloc(sizeof(Storage)); 
    new (storage) Storage(path);
    storage->load(map);
    dirty = false;
    async_init();
}

SharedPreferences::~SharedPreferences() {
    storage->flush(map);
    async_shutdown();
    map->~HashMap(); 
    free(map);
    storage->~Storage(); free(storage);
    pthread_mutex_destroy(&lock);
}

int SharedPreferences::get_int(const string& key, int def) {
    pthread_mutex_lock(&lock);
    entry_t* e = map->get(key);
    int out = def;
    
    if (e && e->value.type == VALUE_INT)
        out = e->value.i;
        
    pthread_mutex_unlock(&lock);
    return out;
}

bool SharedPreferences::get_boolean(const string& key, bool def) {
    pthread_mutex_lock(&lock);
    entry_t* e = map->get(key);
    bool out = def;
    
    if (e && e->value.type == VALUE_BOOL)
        out = e->value.b;
        
    pthread_mutex_unlock(&lock);
    return out;
}

float SharedPreferences::get_float(const string& key, float def) {
    pthread_mutex_lock(&lock);
    entry_t* e = map->get(key);
    float out = def;
    
    if (e && e->value.type == VALUE_FLOAT)
        out = e->value.f;
        
    pthread_mutex_unlock(&lock);
    return out;
}

string SharedPreferences::get_string(const string& key, const string& def) {
    pthread_mutex_lock(&lock);
    entry_t* e = map->get(key);
    string out = def;
    
    if (e && e->value.type == VALUE_STRING)
        out = e->value.s;
        
    pthread_mutex_unlock(&lock);
    return out;
}

Editor* SharedPreferences::edit() {
    Editor* ed = (Editor*)malloc(sizeof(Editor)); 
    new (ed) Editor(this);
    return ed;
}

Snapshot::Snapshot(HashMap* map) {
    copy = map->clone();
}

Snapshot::~Snapshot() {
    copy->~HashMap(); free(copy);
}

Editor::Editor(SharedPreferences* prefs) {
    sp = prefs;
    head = NULL;
    tail = NULL;
}

Editor::~Editor() {
    operation_t* op = head;
    while (op) {
        operation_t* next = op->next;
        op->~operation_t(); free(op);
        op = next;
    }
}

void Editor::append(operation_t* op) {
    if (!head)
        head = op;
    else
        tail->next = op;
        
    tail = op;
}

Editor* Editor::put_int(const string& key, int value) {
    operation_t* op = (operation_t*)malloc(sizeof(operation_t)); 
    new (op) operation_t();
    op->key = key;
    op->value.type = VALUE_INT;
    op->value.i = value;
    op->is_remove = false;
    op->next = NULL;
    
    append(op);
    return this;
}

Editor* Editor::put_boolean(const string& key, bool value) {
    operation_t* op = (operation_t*)malloc(sizeof(operation_t)); 
    new (op) operation_t();
    op->key = key;
    op->value.type = VALUE_BOOL;
    op->value.b = value;
    op->is_remove = false;
    op->next = NULL;
    
    append(op);
    return this;
}

Editor* Editor::put_float(const string& key, float value) {
    operation_t* op = (operation_t*)malloc(sizeof(operation_t)); 
    new (op) operation_t();
    op->key = key;
    op->value.type = VALUE_FLOAT;
    op->value.f = value;
    op->is_remove = false;
    op->next = NULL;
    
    append(op);
    return this;
}

Editor* Editor::put_string(const string& key, const string& value) {
    operation_t* op = (operation_t*)malloc(sizeof(operation_t)); 
    new (op) operation_t();
    op->key = key;
    op->value.type = VALUE_STRING;
    op->value.s = value;
    op->is_remove = false;
    op->next = NULL;
    
    append(op);
    return this;
}

Editor* Editor::remove(const string& key) {
    operation_t* op = (operation_t*)malloc(sizeof(operation_t)); 
    new (op) operation_t();
    op->key = key;
    op->is_remove = true;
    op->next = NULL;
    
    append(op);
    return this;
}

void Editor::apply_common() {
    operation_t* op = head;
    while (op) {
        if (op->is_remove)
            sp->map->remove(op->key);
        else
            sp->map->put(op->key, op->value);
            
        op = op->next;
    }
}   

void Editor::apply() {
    pthread_mutex_lock(&sp->lock);

    apply_common();

    sp->dirty=true;
    pthread_mutex_unlock(&sp->lock);
    
    async_schedule(sp);
}

bool Editor::commit() {
    pthread_mutex_lock(&sp->lock);

    apply_common();

    bool ok = sp->storage->flush(sp->map);
    pthread_mutex_unlock(&sp->lock);
    
    return ok;
}