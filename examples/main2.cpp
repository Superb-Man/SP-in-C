#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include "SharedPrefHelper.hpp"
#include <chrono>

using namespace std;
const int NUM_ENTRIES = 163;

int main() {
    // write a lot of data total avg time for apply, commit and get
    {
        SharedPrefHelper helper("prefs_test.db");
        helper.setStrategy(WriteStrategy::APPLY);
        
        auto var_track_start = std::chrono::high_resolution_clock::now();

        float apply_time_put = 0;
        for (int i = 0; i < NUM_ENTRIES; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            helper.putInt("key" + to_string(i), i);
            auto end = std::chrono::high_resolution_clock::now();
            apply_time_put += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        }
        auto var_track_end = std::chrono::high_resolution_clock::now();

        apply_time_put /= NUM_ENTRIES;

        printf("Average time for apply put: %.2f microseconds\n", apply_time_put);

        auto us = std::chrono::duration_cast<std::chrono::microseconds>(var_track_end-var_track_start).count();

        printf(
            "Time taken to track var: %.2f microseconds\n",
            (double)us
        );


        float apply_time_get = 0;
        for (int i = 0; i < NUM_ENTRIES; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            helper.getInt("key" + to_string(i), -1);
            auto end = std::chrono::high_resolution_clock::now();
            apply_time_get += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        }
        apply_time_get /= NUM_ENTRIES;


        printf("Average time for apply get: %.2f microseconds\n", apply_time_get);
    }

    {
        SharedPrefHelper helper2("prefs_test2.db");
        helper2.setStrategy(WriteStrategy::MAIN_THREAD_COMMIT);

        auto var_track_start = std::chrono::high_resolution_clock::now();

        float commit_time_put = 0;
        for (int i = 0; i < NUM_ENTRIES; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            helper2.putInt("key" + to_string(i), i);
            auto end = std::chrono::high_resolution_clock::now();
            commit_time_put += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        }
        auto var_track_end = std::chrono::high_resolution_clock::now();
        commit_time_put /= NUM_ENTRIES;

        printf("Average time for commit put: %.2f microseconds\n", commit_time_put);

        auto us = std::chrono::duration_cast<std::chrono::microseconds>(var_track_end-var_track_start).count();

        printf(
            "Time taken to track var: %.2f microseconds\n",
            (double)us
        );
    }

    // delete the test files , but not deleted!
    remove("prefs_test.db");
    remove("prefs_test2.db");

    return 0;
}