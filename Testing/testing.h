#include <string>
#include <iostream>

using namespace std;

int testNum = 1;

void assertInt(int a, int b, string pass, string fail) {
    if (a == b) {
        cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
        cout.flush();
    } else {
        cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
        cout.flush();
    }
    testNum++;
}
void assertIntNotEqual(int a, int b, string pass, string fail) {
    if (a != b) {
        cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
        cout.flush();
    } else {
        cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
        cout.flush();
    }
}
void assertLessThan(int a, int b, string pass, string fail) {
    if(a < b) {
        cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
        cout.flush();
    } else {
        cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
        cout.flush();
    }
    testNum++;
}

void assertLessThanEqualTo(int a, int b, string pass, string fail) {
    if(a<=b) {
        cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
        cout.flush();
    } else {
        cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
        cout.flush();
    }
    testNum++;
}

void assertIntInArray(int a, int *b, int size, string pass, string fail) {
    int outOfArray = 0;
    for (int i = 0; i < size; ++i) {
        if (a != b[i]) {
            outOfArray++;
        }
    }
    if(outOfArray != size) {
        cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
    } else {
        cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
    }
    testNum++;
}

void checkUnique(Ingredient *ingredients, int size, string pass, string fail) {
    Ingredient first = ingredients[0];
    for(int i = 1; i < size; i++) {
        if()

    }
}

void endTesting() {
    cout << "TESTING IS DONE\n";
    cout.flush();
}