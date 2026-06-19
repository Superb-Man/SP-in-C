#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <string>
#include <map>
#include <pthread.h>
#include "hashmap.hpp"

using namespace std;

class Storage {
private:
    string path;
    string log_path;
    int delay_secs = 0; // Simulate delay for commit

    bool write_transaction_log();
    bool clear_transaction_log();
    bool recover_from_log();
    
    static pthread_mutex_t file_locks_lock;
    static map<string, pthread_mutex_t*> file_locks;
    
    static pthread_mutex_t* get_file_lock(const string& filepath);

public:
    Storage(const string& p);

    ~Storage();

    bool flush(HashMap* map);
    bool flush_atomic(HashMap* map);  // Atomic write with fsync
    bool load(HashMap* map);

    void setDelay(int secs);
    
    static void cleanup_file_locks();
};

#endif
