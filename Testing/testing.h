#include <string>
#include <iostream>

using namespace std;

int testNum = 1;

void assertInt(int a, int b, string pass, string fail) {
    if (a == b) {
        cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
    } else {
        cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
    }
    testNum++;
}
void assertIntNotEqual(int a, int b, string pass, string fail) {
    if (a != b) {
        cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
    } else {
        cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
    }
}
void assertLessThan(int a, int b, string pass, string fail) {
    if(a < b) {
        cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
    } else {
        cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
    }
    testNum++;
}

void assertLessThanEqualTo(int a, int b, string pass, string fail) {
    if(a<=b) {
        cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
    } else {
        cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
    }
    testNum++;
}