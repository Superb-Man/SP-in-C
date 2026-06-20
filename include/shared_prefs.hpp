#ifndef SHARED_PREFS_HPP
#define SHARED_PREFS_HPP

#include <pthread.h>
#include <string>

#include "hashmap.hpp"
#include "storage.hpp"

using namespace std;

class SharedPrefsManager;

enum class WriteStrategy {
    APPLY,
    COMMIT,
    MAIN_THREAD_COMMIT
};

struct SharedPreferences;

struct operation_t {
    string key;
    bool is_remove;
    value_t value;
    operation_t* next;
};

struct Editor {
private:
    SharedPreferences* sp;
    operation_t* head;
    operation_t* tail;
    void append(operation_t* op);
    void apply_internal(WriteStrategy strategy);

public:
    Editor(SharedPreferences* prefs);
    ~Editor();

    Editor* put_int(const string& key, int value);
    Editor* put_boolean(const string& key, bool value);
    Editor* put_float(const string& key, float value);
    Editor* put_string(const string& key, const string& value);
    Editor* remove(const string& key);
    void apply();
    bool commit();
    bool block_caller_and_commit();
};

struct Snapshot {
public:
    HashMap* copy;
    Snapshot(HashMap* map);
    ~Snapshot();
};

struct SharedPreferences {
    friend class SharedPrefsManager;  // Only SharedPrefsManager can create instances

private:
    SharedPreferences(const string& path);

public:
    pthread_mutex_t lock;
    pthread_cond_t commit_cond;      // Notify when commit completes
    HashMap* map;
    Storage* storage;
    int pending_commits;              // Track in-flight commits

    bool dirty;

    ~SharedPreferences();

    int get_int(const string& key, int def);
    bool get_boolean(const string& key, bool def);
    float get_float(const string& key, float def);
    string get_string(const string& key, const string& def);
    Editor* edit();
    
    // Notify async worker that commit completed
    void notify_commit_done();
};

#endif