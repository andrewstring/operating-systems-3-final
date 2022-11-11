#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "testing.h"

using namespace std;

enum Mutex {
        smokerMutex,
        agentMutex,
        criticalSection,
};

Ingredient ingredientMapping[3] = {Tobacco, Paper, Matches};

string ingredientOutput[3] = {"Tobacco", "Paper", "Matches"};

struct SharedMemory {
    int smokerMutex = 1;
    int agentMutex = 1;
    int criticalSection = 1;
    int numOfRounds = 5;
    int roundCounter = 0;
    Ingredient smokerOne;
    Ingredient smokerTwo;
    Ingredient smokerThree;
    Ingredient agentIngredients[2];
} sMem;

void acquire(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case smokerMutex:
            if(sharedMemory->smokerMutex == 1) {
                sharedMemory->smokerMutex = 0;
            }
            break;
        case agentMutex:
            if(sharedMemory->agentMutex == 1) {
                sharedMemory->agentMutex = 0;
            }
            break;
        case criticalSection:
            if(sharedMemory->criticalSection == 1) {
                sharedMemory->criticalSection = 0;
            }
            break;
    }
}

void release(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case smokerMutex:
            if(sharedMemory->smokerMutex == 0) {
                sharedMemory->smokerMutex = 1;
            }
            break;
        case agentMutex:
            if(sharedMemory->agentMutex == 0) {
                sharedMemory->agentMutex = 1;
            }
            break;
        case criticalSection:
            if(sharedMemory->criticalSection == 0) {
                sharedMemory->criticalSection = 1;
            }
            break;

    }
}

int getRandomIngredientIndex(int min, int max) {
    int val = rand() % (max+1) + min;
    int inArrayCheck[max+1];
    for (int i = 0; i <= max; ++i) {
        inArrayCheck[i] = i;
    }
    assertIntInArray(
        val,
        inArrayCheck,
        max + 1,
        "The randomly generated index is in the range",
        "The randomly generated index is not in the range"
    );

    return val;
}

void setSmokerIngredients(SharedMemory *sharedMemory) {
    Ingredient ingredient[3];

    int ingredientOneIndex = getRandomIngredientIndex(0,2);
    ingredient[0] = ingredientMapping[ingredientOneIndex];

    Ingredient remainingIngredients[2];
    int index = 0;
    for (int i = 0; i < 3; ++i) {
        if (i != ingredientOneIndex) {
            remainingIngredients[index] = ingredientMapping[i];
            index++;
        }
    }
    int ingredientTwoIndex = getRandomIngredientIndex(0,1);
    ingredient[1] = remainingIngredients[ingredientTwoIndex];

    Ingredient reminingIngredient;
    for(int i = 0; i < 3; ++i) {
        if (ingredientMapping[i] != ingredient[0] && ingredientMapping[i] != ingredient[1]) {
            ingredient[2] = ingredientMapping[i];
        }
    }

    assertUniqueIngredientArray(
        ingredient,
        3,
        "All agent ingredients are unique",
        "Not all agent ingredients are unique"
    );

    sharedMemory->smokerOne = ingredient[0];
    sharedMemory->smokerTwo = ingredient[1];
    sharedMemory->smokerThree = ingredient[2];
}

void setTwoIngredients(SharedMemory *sharedMemory) {
    int ingredientOneIndex = getRandomIngredientIndex(0,2);
    sharedMemory->agentIngredients[0] = ingredientMapping[ingredientOneIndex];

    Ingredient remainingIngredients[2];
    int index = 0;
    for (int i = 0; i < 3; ++i) {
        if (i != ingredientOneIndex) {
            remainingIngredients[index] = ingredientMapping[i];
            index++;
        }
    }

    int ingredientTwoIndex = getRandomIngredientIndex(0,1);
    sharedMemory->agentIngredients[1] = remainingIngredients[ingredientTwoIndex];

    assertIngredientNotEqual(
        sharedMemory->agentIngredients[0],
        sharedMemory->agentIngredients[1],
        "Two agent ingredients are unique",
        "Two agent ingredients are not unique"
    );
}

void* smokerOne(void *sharedMemory) {
    SharedMemory *memory = (SharedMemory *) sharedMemory;

    while(true) {
        if (memory->smokerMutex == 1 && memory->criticalSection == 1) {
            if (memory->criticalSection == 1) {
                acquire(memory, criticalSection);
                if (memory->smokerOne != memory->agentIngredients[0] and
                    memory->smokerOne != memory->agentIngredients[1]) {
                        assertIngredientNotInArray(
                            memory->smokerOne,
                            memory->agentIngredients,
                            "Smoker one did not have the two agent ingredients",
                            "Smoker one had one of the two agent ingredients"
                        );
                    this_thread::sleep_for(chrono::seconds(2));
                    acquire(memory, smokerMutex);
                    assertInt(
                        memory->smokerMutex,
                        0,
                        "Smoker Mutex was successfully acquired",
                        "Smoker Mutex was not successfully acquired"
                    );
                    release(memory, agentMutex);
                    memory->roundCounter++;
                }
                release(memory, criticalSection);
            }
        }
    }

    return NULL;
}

void* smokerTwo(void *sharedMemory) {
    SharedMemory *memory = (SharedMemory *) sharedMemory;

    while (true) {
        if (memory->smokerMutex == 1 && memory->criticalSection == 1) {
            if(memory->criticalSection == 1) {
                acquire(memory, criticalSection);
                // only run if smoker has the third ingredient
                if (memory->smokerTwo != memory->agentIngredients[0] and
                    memory->smokerTwo != memory->agentIngredients[1]) {
                        assertIngredientNotInArray(
                            memory->smokerTwo,
                            memory->agentIngredients,
                            "Smoker two did not have the two agent ingredients",
                            "Smoker two had one of the two agent ingredients"
                        );
                    this_thread::sleep_for(chrono::seconds(2));
                    acquire(memory, smokerMutex);
                    assertInt(
                        memory->smokerMutex,
                        0,
                        "Smoker Mutex was successfully acquired",
                        "Smoker Mutex was not successfully acquired"
                    );
                    release(memory, agentMutex);
                    memory->roundCounter++;
                }
                release(memory, criticalSection);
            }
        }
    }

    return NULL;
}

void* smokerThree(void *sharedMemory) {
    SharedMemory *memory = (SharedMemory *) sharedMemory;

    while(true) {
        if (memory->smokerMutex == 1 && memory->criticalSection == 1) {
            if(memory->criticalSection == 1) {
                acquire(memory, criticalSection);
                // only run if smoker has the third ingredient
                if (memory->smokerThree != memory->agentIngredients[0] and
                    memory->smokerThree != memory->agentIngredients[1]) {
                        assertIngredientNotInArray(
                            memory->smokerThree,
                            memory->agentIngredients,
                            "Smoker three did not have the two agent ingredients",
                            "Smoker three had one of the two agent ingredients"
                        );
                    this_thread::sleep_for(chrono::seconds(2));
                    acquire(memory, smokerMutex);
                    assertInt(
                        memory->smokerMutex,
                        0,
                        "Smoker Mutex was successfully acquired",
                        "Smoker Mutex was not successfully acquired"
                    );
                    release(memory, agentMutex);
                    memory->roundCounter++;
                }
                release(memory, criticalSection);
            }
        }
    }

    return NULL;
}

void* agent(void *sharedMemory) {
    SharedMemory *memory = (SharedMemory *) sharedMemory;
    acquire(memory, smokerMutex);

    while (true) {
        if (memory->criticalSection == 1 && memory->roundCounter < memory->numOfRounds) {
            acquire(memory, criticalSection);
            if (memory->agentMutex == 1) {
                setTwoIngredients(memory);
                acquire(memory, agentMutex);
                assertInt(
                    memory->agentMutex,
                    0,
                    "Agent Mutex was successfully acquired",
                    "Agent Mutex was not successfully acquired"
                );

                this_thread::sleep_for(chrono::seconds(2));
                release(memory, smokerMutex);
            }
            release(memory, criticalSection);
        }
    }

    return NULL;
}

int main() {


    SharedMemory *sharedMemory = &sMem;

    srand(time(NULL));
    setSmokerIngredients(sharedMemory);
    setTwoIngredients(sharedMemory);

    pthread_t tidSmokerOne;
    pthread_t tidSmokerTwo;
    pthread_t tidSmokerThree;
    pthread_t tidAgent;
    pthread_attr_t attrSmokerOne;
    pthread_attr_t attrSmokerTwo;
    pthread_attr_t attrSmokerThree;
    pthread_attr_t attrAgent;

    pthread_attr_init(&attrAgent);
    pthread_attr_init(&attrSmokerOne);
    pthread_attr_init(&attrSmokerTwo);
    pthread_attr_init(&attrSmokerThree);
    pthread_create(&tidAgent, &attrAgent, agent, sharedMemory);
    this_thread::sleep_for(chrono::seconds(2));
    pthread_create(&tidSmokerOne, &attrSmokerOne, smokerOne, sharedMemory);
    pthread_create(&tidSmokerTwo, &attrSmokerTwo, smokerTwo, sharedMemory);
    pthread_create(&tidSmokerThree, &attrSmokerThree, smokerThree, sharedMemory);

    bool testInProgress = true;
    while(testInProgress) {
        if(sharedMemory->roundCounter >= sharedMemory->numOfRounds) {
            this_thread::sleep_for(chrono::seconds(5));
            endTesting();
            testInProgress = false;
        }
    }

    pthread_join(tidAgent, NULL);
    pthread_join(tidSmokerOne, NULL);
    pthread_join(tidSmokerTwo, NULL);
    pthread_join(tidSmokerThree, NULL);

    return 0;
}