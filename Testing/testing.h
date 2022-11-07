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

void assertIntInArray(int a, int *b, string pass, string fail) {
    int in = 0;
    for (int i = 0; i < sizeof(b)/sizeof(b[0]); ++i) {
        cout << "i\n";
        if (a != b[i]) {
            in++;
        }
    }
    if(in == sizeof(*b)/sizeof(b[0])) {
        cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
    } else {
        cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
    }
    testNum++;
}

void endTesting() {
    cout << "TESTING IS DONE\n";
    cout.flush();
}