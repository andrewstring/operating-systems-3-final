#include <string>
#include <iostream>

using namespace std;

int testNum = 1;


enum Ingredient {
    Tobacco,
    Paper,
    Matches
};

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

int assertUniqueIngredientArray(Ingredient *ingredients, int size, string pass, string fail) {
    Ingredient first = ingredients[0];
    for(int i = 0; i < size; i++) {
        for(int j = i+1; j < size; j++) {
            if (ingredients[i] == ingredients[j]) {
                cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
                testNum++;
                return 0;
            }
        }
    }
    cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
    testNum++;
    return 1;
}

void assertIngredientNotEqual(Ingredient ingredientOne, Ingredient ingredientTwo, string pass, string fail) {
    if(ingredientOne != ingredientTwo) {
        cout << "TEST " + to_string(testNum) + " PASSED: " + pass + "\n";
    }
    else {
        cout << "TEST " + to_string(testNum) + " FAILED: " + fail + "\n";
    }
    testNum++;
}

void endTesting() {
    cout << "TESTING IS DONE\n";
    cout.flush();
}