#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "testing.h"

using namespace std;

// access points for wait and signal
enum Mutex {
        smokerMutex,
        agentMutex,
        criticalSection,
};

// enum Ingredient {
//     Tobacco,
//     Paper,
//     Matches
// };

// mapping that will be used for accessing Ingredients by index
Ingredient ingredientMapping[3] = {Tobacco, Paper, Matches};

// output mapping that will be used to print the Ingredient via cout
string ingredientOutput[3] = {"Tobacco", "Paper", "Matches"};

struct SharedMemory {
    int smokerMutex = 1;
    int agentMutex = 1;
    int criticalSection = 1;
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

// random number generator for ingredient index
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
    //ingredient array
    Ingredient ingredient[3];

    //set first ingredient
    int ingredientOneIndex = getRandomIngredientIndex(0,2);
    ingredient[0] = ingredientMapping[ingredientOneIndex];

    //set second ingredient
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

    //set third ingredient
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

    //set smoker ingredients
    sharedMemory->smokerOne = ingredient[0];
    sharedMemory->smokerTwo = ingredient[1];
    sharedMemory->smokerThree = ingredient[2];
}

void setTwoIngredients(SharedMemory *sharedMemory) {
    //set first ingredient
    int ingredientOneIndex = getRandomIngredientIndex(0,2);
    sharedMemory->agentIngredients[0] = ingredientMapping[ingredientOneIndex];

    //set second ingredient
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
                // only run if smoker has the third ingredient
                if (memory->smokerOne != memory->agentIngredients[0] and
                    memory->smokerOne != memory->agentIngredients[1]) {
                        assertIngredientNotInArray(
                            memory->smokerOne,
                            memory->agentIngredients,
                            "Smoker one did not have the two agent ingredients",
                            "Smoker one had one of the two agent ingredients"
                        );
                    //cout << "Smoker one started smoking\n";
                    //cout.flush();
                    this_thread::sleep_for(chrono::seconds(2));
                    //cout << "Smoker one finished smoking\n";
                    //cout.flush();
                    acquire(memory, smokerMutex);
                    assertInt(
                        memory->smokerMutex,
                        0,
                        "Smoker Mutex was successfully acquired",
                        "Smoker Mutex was not successfully acquired"
                    );
                    release(memory, agentMutex);
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
                    //cout << "Smoker two started smoking\n";
                    //cout.flush();
                    this_thread::sleep_for(chrono::seconds(2));
                    //cout << "Smoker two finished smoking\n";
                    //cout.flush();
                    acquire(memory, smokerMutex);
                    assertInt(
                        memory->smokerMutex,
                        0,
                        "Smoker Mutex was successfully acquired",
                        "Smoker Mutex was not successfully acquired"
                    );
                    release(memory, agentMutex);
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
                    //cout << "Smoker three started smoking\n";
                    //cout.flush();
                    this_thread::sleep_for(chrono::seconds(2));
                    //cout << "Smoker three finished smoking\n";
                    //cout.flush();
                    acquire(memory, smokerMutex);
                    assertInt(
                        memory->smokerMutex,
                        0,
                        "Smoker Mutex was successfully acquired",
                        "Smoker Mutex was not successfully acquired"
                    );
                    release(memory, agentMutex);
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
        if (memory->criticalSection == 1) {
            acquire(memory, criticalSection);
            if (memory->agentMutex == 1) {
                // set two ingredients for each agent round
                setTwoIngredients(memory);
                //cout << "Agent has placed: " + ingredientOutput[memory->agentIngredients[0]]
                //        + " and " + ingredientOutput[memory->agentIngredients[1]] + "\n";
                //cout.flush();
                acquire(memory, agentMutex);
                assertInt(
                    memory->agentMutex,
                    0,
                    "Agent Mutex was successfully acquired",
                    "Agent Mutex was not successfully acquired"
                );

                // this wait is the time it takes for agent to place ingredients on table
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

    //get random ingredient indices
    srand(time(NULL));
    setSmokerIngredients(sharedMemory);
    setTwoIngredients(sharedMemory);

    //display what each smoker has
    //cout << "Smoker one has " + ingredientOutput[sharedMemory->smokerOne] + "\n";
    //cout.flush();
    //cout << "Smoker two has " + ingredientOutput[sharedMemory->smokerTwo] + "\n";
    //cout.flush();
    //cout << "Smoker three has " + ingredientOutput[sharedMemory->smokerThree] + "\n";
    //cout.flush();

    //setTwoIngredients(sharedMemory);

    // four threads - one for the agent and three for the smokers - one thread for each smoker
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

    pthread_join(tidAgent, NULL);
    pthread_join(tidSmokerOne, NULL);
    pthread_join(tidSmokerTwo, NULL);
    pthread_join(tidSmokerThree, NULL);

    return 0;
}