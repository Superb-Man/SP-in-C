#ifndef ASYNC_HPP
#define ASYNC_HPP
#include "shared_prefs.hpp"

const int MAX_DELAY_SECS = 1;
struct SharedPreferences;
struct Snapshot;

void async_init();
void async_schedule(SharedPreferences* sp, WriteStrategy strategy);
void async_schedule_sync(SharedPreferences* sp);  // Synchronous flush (commit)
void async_shutdown();

#endif