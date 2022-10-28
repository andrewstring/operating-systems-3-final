#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <string>
#include <tuple>
#include <map>

using namespace std;

enum Mutex {
    northMut,
    southMut,
    criticalSection
};

enum Direction {
    north,
    south
};
string directionOutput[2] = {"North Bound", "South Bound"};

struct Traveler {
    string *name;
    Direction direction;
};

struct SharedMemory {
    int northMutex = 1;
    int southMutex = 1;
    int criticalSection = 1;
    int rrQuantum = 0;
    queue<tuple<Direction, string *>> northTravelers;
    queue<tuple<Direction, string *>> southTravelers;
    int numOfNorthTravelers = 0;
    int numOfSouthTravelers = 0;
    int numOfTravelersOnBridge = 0;
    string traveler[26] = {"Alpha", "Bravo", "Charlie", "Delta", "Echo",
                           "Foxtrot", "Golf", "Hotel", "India", "Juliet",
                           "Kilo", "Lima", "Mike", "November", "Oscar",
                           "Papa", "Quebec", "Romeo", "Sierra", "Tango",
                           "Uniform", "Victor", "Whisky", "X-Ray", "Yankee",
                           "Zulu"};
} sMem;

void acquire(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case northMut:
            if (sharedMemory->northMutex == 1) {
                sharedMemory->northMutex = 0;
            }
            break;
        case southMut:
            if (sharedMemory->southMutex == 1) {
                sharedMemory->southMutex = 0;
            }
            break;
        case criticalSection:
            if (sharedMemory->criticalSection == 1) {
                sharedMemory->criticalSection = 0;
            }
            break;
    }
}

void release(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case northMut:
            if (sharedMemory->northMutex == 0) {
                sharedMemory->northMutex = 1;
            }
            break;
        case southMut:
            if (sharedMemory->southMutex == 0) {
                sharedMemory->southMutex = 1;
            }
            break;
        case criticalSection:
            if (sharedMemory->criticalSection == 0) {
                sharedMemory->criticalSection = 1;
            }
            break;
    }
}

void addNorthTraveler(SharedMemory *sharedMemory, string *name) {
    tuple<Direction, string*> newNorthTraveler = make_tuple(north, name);
    sharedMemory->numOfNorthTravelers++;
    sharedMemory->northTravelers.push(newNorthTraveler);
    cout << *name + " (Direction: " + directionOutput[get<0>(newNorthTraveler)] + ") has been added\n";
    cout.flush();
}

tuple<Direction, string*> *removeNorthTraveler(SharedMemory *sharedMemory) {
    tuple<Direction, string*> *retrievedNorthTraveler = &(sharedMemory->northTravelers.front());
    sharedMemory->numOfNorthTravelers--;
    sharedMemory->northTravelers.pop();
    return retrievedNorthTraveler;
}

void addSouthTraveler(SharedMemory *sharedMemory, string *name) {
    tuple<Direction, string*> newSouthTraveler = make_tuple(south, name);
    sharedMemory->numOfSouthTravelers++;
    sharedMemory->southTravelers.push(newSouthTraveler);
    cout << *name + " (Direction: " + directionOutput[get<0>(newSouthTraveler)] + ") has been added\n";
    cout.flush();
}

tuple<Direction, string*> *removeSouthTraveler(SharedMemory *sharedMemory) {
    tuple<Direction, string*> *retrievedSouthTraveler = &(sharedMemory->southTravelers.front());
    sharedMemory->numOfSouthTravelers--;
    sharedMemory->southTravelers.pop();
    return retrievedSouthTraveler;
}

void* crossBridge(void *traveler) {
    tuple<Direction, string*> *bridgeTraveler = (tuple<Direction, string*> *) traveler;
    this_thread::sleep_for(chrono::seconds(2));
    cout << *(get<1>(*bridgeTraveler)) + " (Direction: " + directionOutput[get<0>(*bridgeTraveler)] + ") has crossed the bridge\n";
    cout.flush();

    return NULL;
}

pthread_t enterBridge(SharedMemory *sharedMemory, tuple<Direction, string*> *traveler) {
    pthread_t tidTraveler;
    pthread_attr_t attrTraveler;

    pthread_attr_init(&attrTraveler);

    cout << *get<1>(*traveler) + " (Direction: " + directionOutput[get<0>(*traveler)] + ") has started crossing bridge\n";
    cout.flush();

    if(get<0>(*traveler) == north) {
        acquire(sharedMemory, northMut);
    } else if(get<0>(*traveler) == south) {
        acquire(sharedMemory, southMut);
    }
    sharedMemory->numOfTravelersOnBridge++;

    pthread_create(&tidTraveler, &attrTraveler, crossBridge, traveler);



    return tidTraveler;
}

void setRRQuantum(SharedMemory *sharedMemory, int quantum) {
    sharedMemory->rrQuantum = quantum;
}

void* northProducer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

//    string traveler[26] = {"Alpha", "Bravo", "Charlie", "Delta", "Echo",
//                           "Foxtrot", "Golf", "Hotel", "India", "Juliet",
//                           "Kilo", "Lima", "Mike", "November", "Oscar",
//                           "Papa", "Quebec", "Romeo", "Sierra", "Tango",
//                           "Uniform", "Victor", "Whisky", "X-Ray", "Yankee",
//                           "Zulu"};

    bool run1 = true;
    while(run1) {
        if(memory->criticalSection == 1) {
            acquire(memory, criticalSection);
            addNorthTraveler(memory, &memory->traveler[0]);
            addNorthTraveler(memory, &memory->traveler[1]);
            addNorthTraveler(memory, &memory->traveler[2]);
            addNorthTraveler(memory, &memory->traveler[3]);
            addNorthTraveler(memory, &memory->traveler[4]);
            addNorthTraveler(memory, &memory->traveler[5]);
            run1 = false;
        }
    }
    release(memory, criticalSection);

    this_thread::sleep_for(chrono::seconds(5));

    bool run2 = true;
    while(run2) {
        if(memory->criticalSection == 1) {
            acquire(memory, criticalSection);
            addNorthTraveler(memory, &memory->traveler[5]);
            addNorthTraveler(memory, &memory->traveler[6]);
            addNorthTraveler(memory, &memory->traveler[7]);
            addNorthTraveler(memory, &memory->traveler[8]);
            addNorthTraveler(memory, &memory->traveler[9]);
            run2 = false;
        }
    }
    release(memory, criticalSection);

    return NULL;
}

void* southProducer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

//    string traveler[26] = {"Alpha", "Bravo", "Charlie", "Delta", "Echo",
//                           "Foxtrot", "Golf", "Hotel", "India", "Juliet",
//                           "Kilo", "Lima", "Mike", "November", "Oscar",
//                           "Papa", "Quebec", "Romeo", "Sierra", "Tango",
//                           "Uniform", "Victor", "Whisky", "X-Ray", "Yankee",
//                           "Zulu"};

    bool run1 = true;
    while(run1) {
        if(memory->criticalSection == 1) {
            acquire(memory, criticalSection);
            addSouthTraveler(memory, &memory->traveler[10]);
            addSouthTraveler(memory, &memory->traveler[11]);
            addSouthTraveler(memory, &memory->traveler[12]);
            addSouthTraveler(memory, &memory->traveler[13]);
            addSouthTraveler(memory, &memory->traveler[14]);
            addSouthTraveler(memory, &memory->traveler[24]);
            run1 = false;
        }
    }
    release(memory, criticalSection);

    this_thread::sleep_for(chrono::seconds(4));

    bool run2 = true;
    while(run2) {
        if(memory->criticalSection == 1) {
            acquire(memory, criticalSection);
            addSouthTraveler(memory, &memory->traveler[15]);
            addSouthTraveler(memory, &memory->traveler[16]);
            addSouthTraveler(memory, &memory->traveler[17]);
            addSouthTraveler(memory, &memory->traveler[18]);
            addSouthTraveler(memory, &memory->traveler[19]);
            run2 = false;
        }
    }
    release(memory, criticalSection);

    return NULL;
}

void* travelerConsumer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    Direction rrCurrentSide = north;

    while(true) {
        if (memory->criticalSection == 1 && rrCurrentSide == north) {
            if (memory->numOfNorthTravelers > 0 && memory->southMutex == 1) {
                acquire(memory, criticalSection);
                int counter = 0;
                pthread_t tidTravelers[memory->rrQuantum];
                while (counter < memory->rrQuantum) {
                    if (memory->numOfNorthTravelers == 0) {
                        break;
                    } else {
                        tidTravelers[counter] = enterBridge(memory, removeNorthTraveler(memory));
                        counter++;
                    }
                }
                for (int i = 0; i < counter; ++i) {
                    pthread_join(tidTravelers[i], NULL);
                    memory->numOfTravelersOnBridge--;
                }
                if(memory->numOfTravelersOnBridge == 0) {
                    release(memory, northMut);
                }
            }
            rrCurrentSide = south;
            release(memory, criticalSection);
        } else if (memory->criticalSection == 1 && rrCurrentSide == south) {
            if (memory->numOfSouthTravelers > 0 && memory->northMutex == 1) {
                acquire(memory, criticalSection);
                int counter = 0;
                pthread_t tidTravelers[memory->rrQuantum];
                while (counter < memory->rrQuantum) {
                    if (memory->numOfSouthTravelers == 0) {
                        break;
                    } else {
                        tidTravelers[counter] = enterBridge(memory, removeSouthTraveler(memory));
                        counter++;
                    }
                }
                for (int i = 0; i < counter; ++i) {
                    pthread_join(tidTravelers[i], NULL);
                    memory->numOfTravelersOnBridge--;
                }
                if(memory->numOfTravelersOnBridge == 0) {
                    release(memory, southMut);
                }
            }
            rrCurrentSide = north;
            release(memory, criticalSection);
        }

    }

    return NULL;
}

int main() {
    SharedMemory *sharedMemory = &sMem;

    setRRQuantum(sharedMemory, 5);

    pthread_t tidTravelerConsumer;
    pthread_t tidNorthProducer;
    pthread_t tidSouthProducer;
    pthread_attr_t attrTravelerConsumer;
    pthread_attr_t attrNorthProducer;
    pthread_attr_t attrSouthProducer;



    pthread_attr_init(&attrNorthProducer);
    pthread_attr_init(&attrSouthProducer);
    pthread_attr_init(&attrTravelerConsumer);

    pthread_create(&tidNorthProducer, &attrNorthProducer, northProducer, sharedMemory);
    pthread_create(&tidSouthProducer, &attrSouthProducer, southProducer, sharedMemory);

    // Prevents error with tidTravelerConsumer starting before producers
    this_thread::sleep_for(chrono::seconds(2));
    pthread_create(&tidTravelerConsumer, &attrTravelerConsumer, travelerConsumer, sharedMemory);


    pthread_join(tidNorthProducer, NULL);
    pthread_join(tidSouthProducer, NULL);
    pthread_join(tidTravelerConsumer, NULL);

    return 0;
}