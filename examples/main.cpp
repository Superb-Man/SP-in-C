#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include "SharedPrefHelper.hpp"

using namespace std;

int main() {
    int iteration = 0;

    while (iteration < 3) {
        cout << "\n================ ITERATION " << iteration << " ================\n";

        SharedPrefHelper helper("prefs.db");

        helper.putString("city", "Dhaka");

        cout << "city = " << helper.getString("city", "none") << endl;

        helper.putInt("age", 21 + iteration);
        helper.putString("name", "Alice");

        cout << "age  = " << helper.getInt("age", 0) << endl;
        cout << "name = " << helper.getString("name", "none") << endl;

        iteration++;
    }

    return 0;
}