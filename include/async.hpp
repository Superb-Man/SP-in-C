#ifndef ASYNC_HPP
#define ASYNC_HPP

const int MAX_DELAY_SECS = 1;
struct SharedPreferences;
struct Snapshot;

void async_init();
void async_schedule(SharedPreferences* sp);
void async_shutdown();

#endif