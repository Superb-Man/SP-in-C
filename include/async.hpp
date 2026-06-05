#ifndef ASYNC_HPP
#define ASYNC_HPP

class SharedPreferences;
class Snapshot;

void async_init();
void async_schedule(SharedPreferences* sp, Snapshot* snap);
void async_shutdown();

#endif