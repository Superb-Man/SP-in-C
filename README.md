# SP-in-C

A lightweight C++17 SharedPreferences-style key-value storage library with thread-safe in-memory access and persistent on-disk storage.

## Overview

This project implements a local preferences system inspired by Android SharedPreferences semantics:

- Typed key-value operations (`int`, `bool`, `float`, `string`)
- Batched writes through an `Editor`
- Multiple write strategies:
  - `APPLY` (asynchronous, non-blocking)
  - `COMMIT` (caller blocks until persistence completes, flush is performed by worker thread)
  - `MAIN_THREAD_COMMIT` (caller thread performs the flush immediately)
- File-backed persistence in `.shared_prefs/<name>.sp`
- Atomic persistence workflow with temp file + backup + transaction log
- Basic recovery support after interrupted writes

## Project Structure

```
include/
  hashmap.hpp
  storage.hpp
  shared_prefs.hpp
  shared_prefs_manager.hpp
  async.hpp

src/
  hashmap.cpp
  storage.cpp
  shared_prefs.cpp
  shared_prefs_manager.cpp
  async.cpp

examples/
  main.cpp
  main2.cpp
  SharedPrefHelper.hpp
  SharedPrefHelper.cpp
```

## Build

From the repository root:

```bash
make
```

This builds the sample binary named `test`.

## Run

```bash
make run
```

You can also run:

```bash
./test
```

## Additional Targets (from current `Makefile`)

```bash
make leak-asan      # Build with AddressSanitizer and run
make leak-valgrind  # Run valgrind leak check
make clean          # Remove built binary
```

## Quick Usage

The examples use `SharedPrefHelper`, which wraps the core API:

```cpp
SharedPrefHelper helper("default");
helper.putString("city", "Dhaka");
helper.setStrategy(WriteStrategy::MAIN_THREAD_COMMIT);
helper.putInt("age", 21);
int age = helper.getInt("age", 0);
```

Core API classes:

- `SharedPrefsManager`: obtains and manages named preference instances
- `SharedPreferences`: typed `get_*` methods + `edit()`
- `Editor`: staged write operations via `put_*`, `remove`, `apply`, `commit`

## Notes

- If your program calls `SharedPrefsManager::get(...)`, call `SharedPrefsManager::cleanup()` once before exit to stop the async worker and release managed instances cleanly.
- Preference files are created under `.shared_prefs/` relative to the working directory.
