#include "SharedPrefHelper.hpp"

SharedPrefHelper::SharedPrefHelper(const string& path) {
    sp = new SharedPreferences(path);
    editor = sp->edit();
    strategy = WriteStrategy::APPLY;                
}

SharedPrefHelper::~SharedPrefHelper() {
    delete editor;
    delete sp;
}

void SharedPrefHelper::setStrategy(WriteStrategy strategy) {
    this->strategy = strategy;
}

void SharedPrefHelper::putInt(const string& key, int value) {
    editor->put_int(key, value);
    applyOrCommit();
}

void SharedPrefHelper::putBoolean(const string& key, bool value) {
    editor->put_boolean(key, value);
    applyOrCommit();
}

void SharedPrefHelper::putFloat(const string& key, float value) {
    editor->put_float(key, value);
    applyOrCommit();
}

void SharedPrefHelper::putString(const string& key, const string& value) {
    editor->put_string(key, value);
    applyOrCommit();
}

void SharedPrefHelper::remove(const string& key) {
    editor->remove(key);
}

int SharedPrefHelper::getInt(const string& key, int def) {
    return sp->get_int(key, def);
}

bool SharedPrefHelper::getBoolean(const string& key, bool def) {
    return sp->get_boolean(key, def);
}

float SharedPrefHelper::getFloat(const string& key, float def) {
    return sp->get_float(key, def);
}

string SharedPrefHelper::getString(const string& key, const string& def) {
    return sp->get_string(key, def);
}

void SharedPrefHelper::applyOrCommit() {
    switch (strategy) {
        case WriteStrategy::APPLY:
            editor->apply();
            break;
        case WriteStrategy::COMMIT:
            editor->commit();
            break;
        case WriteStrategy::MAIN_THREAD_COMMIT:
            editor->block_caller_and_commit();
            break;
    }
}

void SharedPrefHelper::setDelay(int secs) {
    sp->storage->setDelay(secs);
}