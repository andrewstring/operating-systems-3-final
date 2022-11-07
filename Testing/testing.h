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

void endTesting() {
    cout << "TESTING IS DONE\n";
    cout.flush();
}