// implement of SharedPrefHelper.hpp

#include "../helper/SharedPrefHelper.hpp"

SharedPrefHelper::SharedPrefHelper(const string& path) {
    sp = malloc(sizeof(SharedPreferences));
    new (sp) SharedPreferences(path);
    editor = malloc(sizeof(Editor));
    new (editor) Editor(sp);
}

SharedPrefHelper::~SharedPrefHelper() {
    editor->~Editor();
    free(editor);
    sp->~SharedPreferences();
    free(sp);
}

void SharedPrefHelper::setStrategy(WriteStrategy strategy) {
    this->strategy = strategy;
}

void SharedPrefHelper::putInt(const string& key, int value) {
    editor->put_int(key, value);
}

void SharedPrefHelper::putBoolean(const string& key, bool value) {
    editor->put_boolean(key, value);
}

void SharedPrefHelper::putFloat(const string& key, float value) {
    editor->put_float(key, value);
}

void SharedPrefHelper::putString(const string& key, const string& value) {
    editor->put_string(key, value);
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