#include "../include/shared_prefs.hpp"
#include "../include/async.hpp"
#include <cstdlib>

SharedPreferences::SharedPreferences(const string& path) {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&commit_cond, NULL);
    map = new HashMap(256);
    storage = new Storage(path);
    storage->load(map);
    dirty = false;
    pending_commits = 0;
    async_init();
}

SharedPreferences::~SharedPreferences() {
    storage->flush(map);
    async_shutdown();

    delete map;
    delete storage;

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&commit_cond);
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
    Editor* ed = new Editor(this);
    return ed;
}

void SharedPreferences::notify_commit_done() {
    pthread_mutex_lock(&lock);
    pending_commits--;
    pthread_cond_broadcast(&commit_cond);
    pthread_mutex_unlock(&lock);
}

Snapshot::Snapshot(HashMap* map) {
    copy = map->clone();
}

Snapshot::~Snapshot() {
    delete copy;
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
        delete op;
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
    operation_t* op = new operation_t();
    op->key = key;
    op->value.type = VALUE_INT;
    op->value.i = value;
    op->is_remove = false;
    op->next = NULL;
    
    append(op);
    return this;
}

Editor* Editor::put_boolean(const string& key, bool value) {
    operation_t* op = new operation_t();
    op->key = key;
    op->value.type = VALUE_BOOL;
    op->value.b = value;
    op->is_remove = false;
    op->next = NULL;
    
    append(op);
    return this;
}

Editor* Editor::put_float(const string& key, float value) {
    operation_t* op = new operation_t();
    op->key = key;
    op->value.type = VALUE_FLOAT;
    op->value.f = value;
    op->is_remove = false;
    op->next = NULL;
    
    append(op);
    return this;
}

Editor* Editor::put_string(const string& key, const string& value) {
    operation_t* op = new operation_t();
    op->key = key;
    op->value.type = VALUE_STRING;
    op->value.s = value;
    op->is_remove = false;
    op->next = NULL;
    
    append(op);
    return this;
}

Editor* Editor::remove(const string& key) {
    operation_t* op = new operation_t();
    op->key = key;
    op->is_remove = true;
    op->next = NULL;
    
    append(op);
    return this;
}

void Editor::apply_internal(WriteStrategy strategy) {
    bool need_schedule = false;

    pthread_mutex_lock(&sp->lock);

    operation_t* op = head;
    while (op) {
        if (op->is_remove)
            sp->map->remove(op->key);
        else
            sp->map->put(op->key, op->value);

        op = op->next;
    }

    if (strategy == WriteStrategy::COMMIT) {
        sp->dirty = true;
        sp->pending_commits++;
    } else {
        if (!sp->dirty)
            need_schedule = true;

        sp->dirty = true;
    }

    pthread_mutex_unlock(&sp->lock);

    if (strategy == WriteStrategy::COMMIT)
        async_schedule(sp, WriteStrategy::COMMIT);
    else if (need_schedule)
        async_schedule(sp, WriteStrategy::APPLY);
}

void Editor::apply() {
    apply_internal(WriteStrategy::APPLY);
}

bool Editor::commit() {
    apply_internal(WriteStrategy::COMMIT);

    // Wait for flush to complete
    pthread_mutex_lock(&sp->lock);
    while (sp->pending_commits > 0)
        pthread_cond_wait(&sp->commit_cond, &sp->lock);
    pthread_mutex_unlock(&sp->lock);

    return true;
}

bool Editor::block_caller_and_commit() {
    pthread_mutex_lock(&sp->lock);

    operation_t* op = head;
    while (op) {
        if (op->is_remove)
            sp->map->remove(op->key);
        else
            sp->map->put(op->key, op->value);

        op = op->next;
    }

    Snapshot snap(sp->map);

    pthread_mutex_unlock(&sp->lock);

    return sp->storage->flush_atomic(snap.copy);
}