#include "../include/storage.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define MAGIC 0xDEADBEEF
#define TX_MAGIC 0xCAFEBABE

pthread_mutex_t Storage::file_locks_lock = PTHREAD_MUTEX_INITIALIZER;
map<string, pthread_mutex_t*> Storage::file_locks;

pthread_mutex_t* Storage::get_file_lock(const string& filepath) {
    pthread_mutex_lock(&file_locks_lock);
    
    if (file_locks.find(filepath) == file_locks.end()) {
        pthread_mutex_t* new_lock = new pthread_mutex_t;
        pthread_mutex_init(new_lock, nullptr);
        file_locks[filepath] = new_lock;
    }
    
    pthread_mutex_t* lock = file_locks[filepath];
    pthread_mutex_unlock(&file_locks_lock);
    
    return lock;
}

void Storage::cleanup_file_locks() {
    pthread_mutex_lock(&file_locks_lock);
    
    for (auto& pair : file_locks) {
        pthread_mutex_destroy(pair.second);
        delete pair.second;
    }
    file_locks.clear();
    
    pthread_mutex_unlock(&file_locks_lock);
}

Storage::Storage(const string& p) {
    path = p;
    log_path = p + ".log";
    // Pre-create file lock in registry
    get_file_lock(path);
}

Storage::~Storage() {
}

static void write_string(FILE* fp, const string& s) {
    int len = s.size();
    fwrite(&len, sizeof(int), 1, fp);
    fwrite(s.c_str(), 1, len, fp);
}

static string read_string(FILE* fp) {
    int len;
    fread(&len, sizeof(int), 1, fp);
    string s(len, '\0');
    fread(&s[0], 1, len, fp);
    return s;
}

bool Storage::write_transaction_log() {
    FILE* fp = fopen(log_path.c_str(), "wb");
    if (!fp)
        return false;
    
    uint32_t tx_magic = TX_MAGIC;
    fwrite(&tx_magic, sizeof(uint32_t), 1, fp);
    fclose(fp);
    return true;
}

bool Storage::clear_transaction_log() {
    return unlink(log_path.c_str()) == 0 || access(log_path.c_str(), F_OK) != 0;
}

bool Storage::recover_from_log() {
    // If log exists, previous write was interrupted
    // Restore from backup if available
    if (access(log_path.c_str(), F_OK) == 0) {
        string backup = path + ".bak";
        if (access(backup.c_str(), F_OK) == 0) {
            rename(backup.c_str(), path.c_str());
        }
        unlink(log_path.c_str());
        return true;
    }
    return true;
}

bool Storage::flush(HashMap* map) {
    return flush_atomic(map);
}

bool Storage::flush_atomic(HashMap* map) {
    // Get or create global lock for this file path
    pthread_mutex_t* file_lock = get_file_lock(path);
    pthread_mutex_lock(file_lock);

    string tmp = path + ".tmp";
    string bak = path + ".bak";
    
    // Write transaction log first (signals write in progress)
    if (!write_transaction_log()) {
        pthread_mutex_unlock(file_lock);
        return false;
    }
    
    // Create backup of old file
    if (access(path.c_str(), F_OK) == 0) {
        unlink(bak.c_str());
        if (rename(path.c_str(), bak.c_str()) != 0) {
            clear_transaction_log();
            pthread_mutex_unlock(file_lock);
            return false;
        }
    }
    
    FILE* fp = fopen(tmp.c_str(), "wb");
    if (!fp) {
        if (access(bak.c_str(), F_OK) == 0)
            rename(bak.c_str(), path.c_str());
        pthread_mutex_unlock(file_lock);
        clear_transaction_log();
        return false;
    }
    
    sleep(delay_secs); // Simulate delay for commit

    uint32_t magic = MAGIC;
    fwrite(&magic, sizeof(uint32_t), 1, fp);

    for (int i = 0; i < map->get_capacity(); i++) {
        entry_t* e = map->get_buckets()[i];
        while (e) {
            uint32_t type = e->value.type;
            fwrite(&type, sizeof(uint32_t), 1, fp);
            write_string(fp, e->key);

            switch (type) {
            case VALUE_INT:
                fwrite(&e->value.i, sizeof(int), 1, fp);
                break;
            case VALUE_BOOL:
                fwrite(&e->value.b, sizeof(bool), 1, fp);
                break;
            case VALUE_FLOAT:
                fwrite(&e->value.f, sizeof(float), 1, fp);
                break;
            case VALUE_STRING:
                write_string(fp, e->value.s);
                break;
            }
            e = e->next;
        }
    }

    uint32_t end = (uint32_t)-1;
    fwrite(&end, sizeof(uint32_t), 1, fp);
    // fsync(fileno(fp));
    fclose(fp);
    
    if (rename(tmp.c_str(), path.c_str()) != 0) {
        unlink(tmp.c_str());
        if (access(bak.c_str(), F_OK) == 0)
            rename(bak.c_str(), path.c_str());
        clear_transaction_log();
        pthread_mutex_unlock(file_lock);
        return false;
    }
    
    unlink(bak.c_str());
    clear_transaction_log();
    pthread_mutex_unlock(file_lock);
    return true;
}

bool Storage::load(HashMap* map) {
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp)
        return false;

    uint32_t magic;
    fread(&magic, sizeof(uint32_t), 1, fp);
    if (magic != (uint32_t)MAGIC) {
        fclose(fp);
        return false;
    }

    while (true) {
        uint32_t type;
        fread(&type, sizeof(uint32_t), 1, fp);
        if (type == (uint32_t)-1)
            break;

        string key = read_string(fp);
        value_t value;
        value.type = (value_type_t)type;

        switch (type) {
        case VALUE_INT:
            fread(&value.i, sizeof(int), 1, fp);
            break;
        case VALUE_BOOL:
            fread(&value.b, sizeof(bool), 1, fp);
            break;
        case VALUE_FLOAT:
            fread(&value.f, sizeof(float), 1, fp);
            break;
        case VALUE_STRING:
            value.s = read_string(fp);
            break;
        }
        map->put(key, value);
    }

    fclose(fp);
    return true;
}

void Storage::setDelay(int secs) {
    delay_secs = secs;
}