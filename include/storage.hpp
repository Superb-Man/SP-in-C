#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <string>
#include "hashmap.hpp"

using namespace std;

class Storage {
private:
    string path;
    int delay_secs = 0; // Simulate delay for commit

public:
    Storage(const string& p);

    bool flush(HashMap* map);
    bool load(HashMap* map);

    void setDelay(int secs);
};

#endif
