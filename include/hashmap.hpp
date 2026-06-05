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

    entry_t(const string& k,const value_t& v) {
        key = k;
        value = v;
        next = NULL;
    }
};

class HashMap {
private:
    entry_t** buckets;
    int capacity;

    unsigned long hash(const string& key);

public:
    HashMap(int cap);
    ~HashMap();

    void put(const string& key,const value_t& value);
    entry_t* get(const string& key);
    void remove(const string& key);

    int get_capacity();
    entry_t** get_buckets();
    HashMap* clone();
};

#endif
