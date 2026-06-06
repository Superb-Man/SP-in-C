#include "../include/storage.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cstdint>

using namespace std;

#define MAGIC 0xDEADBEEF

Storage::Storage(const string& p) {
    path = p;
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

bool Storage::flush(HashMap* map) {
    FILE* fp = fopen("prefs.tmp", "wb");
    if (!fp)
        return false;
    
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
    fsync(fileno(fp));
    fclose(fp);
    rename("prefs.tmp", path.c_str());
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