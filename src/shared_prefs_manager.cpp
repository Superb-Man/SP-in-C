#include "../include/shared_prefs_manager.hpp"
#include "../include/async.hpp"
#include <sys/stat.h>

pthread_mutex_t SharedPrefsManager::lock = PTHREAD_MUTEX_INITIALIZER;
map<string, SharedPreferences*> SharedPrefsManager::registry;

string SharedPrefsManager::name_to_path(const string& name) {
    string dir = ".shared_prefs";
    mkdir(dir.c_str(), 0755);  
    return dir + "/" + name + ".sp";
}

SharedPreferences* SharedPrefsManager::get(const string& name) {
    pthread_mutex_lock(&lock);

    SharedPreferences* prefs = nullptr;
    auto it = registry.find(name);
    if (it != registry.end()) {
        prefs = it->second;
    } else {
        if (registry.empty()) {
            async_init();
        }
        string path = name_to_path(name);
        prefs = new SharedPreferences(path);
        registry[name] = prefs;
    }

    pthread_mutex_unlock(&lock);
    return prefs;
}

void SharedPrefsManager::remove(const string& name) {
    pthread_mutex_lock(&lock);

    auto it = registry.find(name);
    if (it != registry.end()) {
        delete it->second;
        registry.erase(it);
    }

    pthread_mutex_unlock(&lock);
}

void SharedPrefsManager::cleanup() {
    async_shutdown();

    pthread_mutex_lock(&lock);

    for (auto& pair : registry) {
        delete pair.second;
    }
    registry.clear();

    pthread_mutex_unlock(&lock);

    Storage::cleanup_file_locks();
}
