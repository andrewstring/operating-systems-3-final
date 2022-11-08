#include <string>
#include <iostream>

using namespace std;

enum Ingredient {
    Tobacco,
    Paper,
    Matches
};

void assertInt(int a, int b, string pass, string fail) {
    if (a == b) {
        cout << "TEST PASSED: " + pass + "\n";
        cout.flush();
    } else {
        cout << "TEST FAILED: " + fail + "\n";
        cout.flush();
    }
}
void assertIntNotEqual(int a, int b, string pass, string fail) {
    if (a != b) {
        cout << "TEST PASSED: " + pass + "\n";
        cout.flush();
    } else {
        cout << "TEST FAILED: " + fail + "\n";
        cout.flush();
    }
}
void assertLessThan(int a, int b, string pass, string fail) {
    if(a < b) {
        cout << "TEST PASSED: " + pass + "\n";
        cout.flush();
    } else {
        cout << "TEST FAILED: " + fail + "\n";
        cout.flush();
    }
}

void assertLessThanEqualTo(int a, int b, string pass, string fail) {
    if(a<=b) {
        cout << "TEST PASSED: " + pass + "\n";
        cout.flush();
    } else {
        cout << "TEST FAILED: " + fail + "\n";
        cout.flush();
    }
}

void assertIntInArray(int a, int *b, int size, string pass, string fail) {
    int outOfArray = 0;
    for (int i = 0; i < size; ++i) {
        if (a != b[i]) {
            outOfArray++;
        }
    }
    if(outOfArray != size) {
        cout << "TEST PASSED: " + pass + "\n";
        cout.flush();
    } else {
        cout << "TEST FAILED: " + fail + "\n";
        cout.flush();
    }
}

int assertUniqueIngredientArray(Ingredient *ingredients, int size, string pass, string fail) {
    Ingredient first = ingredients[0];
    for(int i = 0; i < size; i++) {
        for(int j = i+1; j < size; j++) {
            if (ingredients[i] == ingredients[j]) {
                cout << "TEST FAILED: " + fail + "\n";
                cout.flush();
                return 0;
            }
        }
    }
    cout << "TEST PASSED: " + pass + "\n";
    cout.flush();
    return 1;
}

void assertIngredientNotEqual(Ingredient ingredientOne, Ingredient ingredientTwo, string pass, string fail) {
    if(ingredientOne != ingredientTwo) {
        cout << "TEST PASSED: " + pass + "\n";
        cout.flush();
    }
    else {
        cout << "TEST FAILED: " + fail + "\n";
        cout.flush();
    }
}

int assertIngredientNotInArray(Ingredient ingredient, Ingredient *ingredientArray, string pass, string fail) {
    for(int i = 0; i < sizeof(*ingredientArray)/sizeof(ingredientArray[0]); ++i) {
        if (ingredient == ingredientArray[i]) {
            cout << "TEST FAILED: " + fail + "\n";
            cout.flush();
            return 0;
        }
    }
    cout << "TEST PASSED: " + pass + "\n";
    cout.flush();
    return 1;
}

void endTesting() {
    cout << "TESTING IS DONE\n";
    cout.flush();
}