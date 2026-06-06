#ifndef SHARED_PREF_HELPER_HPP
#define SHARED_PREF_HELPER_HPP

#include "../include/shared_prefs.hpp"

enum class WriteStrategy {
    APPLY,
    COMMIT
};

struct SharedPrefHelper {
    SharedPreferences* sp;
    Editor* editor;

    
    SharedPrefHelper(const string& path);
    ~SharedPrefHelper();

    void setStrategy(WriteStrategy strategy = WriteStrategy::APPLY);
    void putInt(const string& key, int value);
    void putBoolean(const string& key, bool value);
    void putFloat(const string& key, float value);
    void putString(const string& key, const string& value);
    void remove(const string& key);
    int getInt(const string& key, int def);
    bool getBoolean(const string& key, bool def);
    float getFloat(const string& key, float def);
    string getString(const string& key, const string& def);

    // setDelay(int secs)
    void setDelay(int secs);

private :
    WriteStrategy strategy;
    void applyOrCommit();
};

#endif