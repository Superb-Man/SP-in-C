#ifndef HASHMAP_HPP
#define HASHMAP_HPP

#include <string>

using namespace std;

enum value_type_t {
    VALUE_INT,
    VALUE_BOOL,
    VALUE_FLOAT,
    VALUE_STRING
};

struct value_t {
    value_type_t type;
    int i;
    bool b;
    float f;
    string s;
};

struct entry_t {
    string key;
    value_t value;
    entry_t* next;
    unsigned long hash;

    entry_t(const string& k, const value_t& v, unsigned long h) {
        key = k;
        value = v;
        hash = h;
        next = nullptr;
    }
};

class HashMap {
private:
    entry_t** buckets;
    int capacity;
    int size;
    static constexpr float LOAD_FACTOR = 0.75f;

    unsigned long hash(const string& key);
    void resize(int new_capacity);

public:
    HashMap(int cap);
    ~HashMap();

    void put(const string& key, const value_t& value);
    entry_t* get(const string& key);
    void remove(const string& key);

    int get_capacity();
    int get_size();
    entry_t** get_buckets();
    HashMap* clone();
};

#endif
