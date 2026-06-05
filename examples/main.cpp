#include <iostream>
#include <unistd.h>
#include <cstdlib> // Required for free()
#include "../include/shared_prefs.hpp"

using namespace std;

int main() {
    int iteration = 0;

    while (iteration < 3) {
        cout << "\n================ ITERATION " << iteration << " ================\n";

        SharedPreferences* sp = new SharedPreferences("prefs.db");

        Editor* ed1 = sp->edit()->put_string("city", "Dhaka");
        ed1->apply();
        
        ed1->~Editor(); 
        free(ed1);

        cout << "city = " << sp->get_string("city", "none") << endl;

        Editor* ed2 = sp->edit()->put_int("age", 21 + iteration)->put_string("name", "Alice");
        ed2->commit();
        
        ed2->~Editor(); 
        free(ed2);

        cout << "age  = " << sp->get_int("age", 0) << endl;
        cout << "name = " << sp->get_string("name", "none") << endl;

        delete sp;


        iteration++;
    }

    return 0;
}