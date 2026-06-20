#ifndef SHARED_PREFS_MANAGER_HPP
#define SHARED_PREFS_MANAGER_HPP

#include <string>
#include <map>
#include <pthread.h>
#include "shared_prefs.hpp"

using namespace std;

class SharedPrefsManager {
private:
    static pthread_mutex_t lock;
    static map<string, SharedPreferences*> registry;

    SharedPrefsManager() = delete;
    static string name_to_path(const string& name);

public:

    static SharedPreferences* get(const string& name);
    static void remove(const string& name);
    static void cleanup();
};

#endif