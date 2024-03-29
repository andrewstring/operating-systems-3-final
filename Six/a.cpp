#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <string>

using namespace std;

// access points for acquire and release
enum Mutex {
    eastwardMut,
    westwardMut,
    criticalSection
};

// access points for wait and signal
enum Semaphore {
    monkeysOnRopeSemaphore
};

enum Destination {
    east,
    west
};

string destinationOutput[2] = {"east", "west"};

struct SharedMemory;

struct Monkey {
    int id;
    Destination destination;
    SharedMemory *sharedMemory;
    Monkey(int idPass, Destination destinationPass, SharedMemory *sharedMemoryPass);
};

// constructor function for monkey struct
Monkey::Monkey(int idPass, Destination destinationPass, SharedMemory *sharedMemoryPass) {
    id = idPass;
    destination = destinationPass;
    sharedMemory = sharedMemoryPass;
}

struct SharedMemory {
    int eastwardMutex = 1;
    int westwardMutex = 1;
    int criticalSection = 1;
    int monkeysOnRopeSemaphore = 0;
    queue<Monkey> eastwardMonkeys;
    queue<Monkey> westwardMonkeys;
    queue<Monkey> monkeysOnRope;
    int numOfEastwardMonkeys = 0;
    int numOfWestwardMonkeys = 0;
    int ropeCapacity = 5;
} sMem;

void acquire(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case eastwardMut:
            if(sharedMemory->eastwardMutex == 1) {
                sharedMemory->eastwardMutex = 0;
            }
            break;
        case westwardMut:
            if(sharedMemory->westwardMutex == 1) {
                sharedMemory->westwardMutex = 0;
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
        case eastwardMut:
            if(sharedMemory->eastwardMutex == 0) {
                sharedMemory->eastwardMutex = 1;
            }
            break;
        case westwardMut:
            if(sharedMemory->westwardMutex == 0) {
                sharedMemory->westwardMutex = 1;
            }
            break;
        case criticalSection:
            if(sharedMemory->criticalSection == 0) {
                sharedMemory->criticalSection = 1;
            }
            break;
    }
}

void wait(SharedMemory *sharedMemory, Semaphore toAccess) {
    switch(toAccess) {
        case monkeysOnRopeSemaphore:
            if(sharedMemory->monkeysOnRopeSemaphore < sharedMemory->ropeCapacity) {
                sharedMemory->monkeysOnRopeSemaphore++;
            }
            break;
    }
}

void signal(SharedMemory *sharedMemory, Semaphore toAccess) {
    switch(toAccess) {
        case monkeysOnRopeSemaphore:
            if(sharedMemory->monkeysOnRopeSemaphore > 0) {
                sharedMemory->monkeysOnRopeSemaphore--;
            }
            break;
    }
}


void addEastwardMonkey(SharedMemory *sharedMemory, int id) {
    Monkey eastwardMonkey = Monkey(id, east, sharedMemory);
    sharedMemory->numOfEastwardMonkeys++;

    // create and add an eastward monkey to the eastward monkey queue
    sharedMemory->eastwardMonkeys.push(eastwardMonkey);
    cout << "Monkey with id " + to_string(id) + " headed to the " +
    destinationOutput[eastwardMonkey.destination] + " has been added\n";
    cout.flush();
}

void addWestwardMonkey(SharedMemory *sharedMemory, int id) {
    Monkey westwardMonkey = Monkey(id, west, sharedMemory);
    westwardMonkey.id = id;
    sharedMemory->numOfWestwardMonkeys++;

    // create and add a westward monkey to the westward monkey queue
    sharedMemory->westwardMonkeys.push(westwardMonkey);
    cout << "Monkey with id " + to_string(id) + " headed to the " +
            destinationOutput[westwardMonkey.destination] + " has been added\n";
    cout.flush();
}

Monkey removeEastwardMonkey(SharedMemory *sharedMemory) {
    Monkey retrievedEastwardMonkeyTuple = sharedMemory->eastwardMonkeys.front();

    // remove an eastward monkey from the eastward monkey queue
    sharedMemory->eastwardMonkeys.pop();
    sharedMemory->numOfEastwardMonkeys--;
    return retrievedEastwardMonkeyTuple;
}

Monkey removeWestwardMonkey(SharedMemory *sharedMemory) {
    Monkey retrievedWestwardMonkeyTuple = sharedMemory->westwardMonkeys.front();

    // remove a westward monkey from the westward monkey queue
    sharedMemory->westwardMonkeys.pop();
    sharedMemory->numOfWestwardMonkeys--;
    return retrievedWestwardMonkeyTuple;
}

void* crossRope(void *monkeyRef) {
    Monkey *monkey = (struct Monkey *) monkeyRef;

    // take two seconds for monkey to cross rope
    this_thread::sleep_for(chrono::seconds(2));
    cout << "Monkey with id " + to_string(monkey->id) + " headed to the " + destinationOutput[monkey->destination] +
            " has crossed the rope\n";
    cout.flush();

    // signal the monkey on rope semaphore once the monkey has crossed the rope
    signal(monkey->sharedMemory, monkeysOnRopeSemaphore);
    if(monkey->sharedMemory->monkeysOnRopeSemaphore == 0) {
        if(monkey->destination == east) {
            release(monkey->sharedMemory, eastwardMut);
        } else if(monkey->destination == west) {
            release(monkey->sharedMemory, westwardMut);
        }
    }

    return NULL;
}

void enterRope(int id, Destination destination, SharedMemory *sharedMemory) {

    if(destination == east) {
        // acquire the eastward mutex if the monkey is from the east
        acquire(sharedMemory, eastwardMut);
    } else if(destination == west) {
        // acquire the westward mutex if the monkey is from the west
        acquire(sharedMemory, westwardMut);
    }

    wait(sharedMemory, monkeysOnRopeSemaphore);

    pthread_t tidMonkey;
    pthread_attr_t attrMonkey;
    pthread_attr_init(&attrMonkey);

    cout << "Monkey with id " + to_string(id) + " headed to the " + destinationOutput[destination] +
    " has got on the rope\n";
    cout.flush();

    // create monkey thread
    pthread_create(&tidMonkey, &attrMonkey, crossRope, new Monkey(id, destination, sharedMemory));
}

void* eastwardProducer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    bool run1 = true;
    while(run1) {
        if(memory->criticalSection == 1) {
            acquire(memory, criticalSection);

            addEastwardMonkey(memory, 1);
            addEastwardMonkey(memory, 2);
            addEastwardMonkey(memory, 3);
            addEastwardMonkey(memory, 4);
            addEastwardMonkey(memory, 5);
            addEastwardMonkey(memory, 6);
            run1 = false;
        }
    }
    release(memory, criticalSection);

    // wait 5 seconds before adding more eastward monkeys
    this_thread::sleep_for(chrono::seconds(5));

    bool run2 = true;
    while(run2) {
        if(memory->criticalSection == 1) {
            acquire(memory, criticalSection);
            addEastwardMonkey(memory, 7);
            addEastwardMonkey(memory, 8);
            addEastwardMonkey(memory, 9);
            addEastwardMonkey(memory, 10);
            addEastwardMonkey(memory, 11);
            addEastwardMonkey(memory, 12);
            run2 = false;
        }
    }
    release(memory, criticalSection);

    return NULL;
}

void* westwardProducer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    bool run1 = true;
    while(run1) {
        if(memory->criticalSection == 1) {
            acquire(memory, criticalSection);
            addWestwardMonkey(memory, 13);
            addWestwardMonkey(memory, 14);
            addWestwardMonkey(memory, 15);
            addWestwardMonkey(memory, 16);
            addWestwardMonkey(memory, 17);
            addWestwardMonkey(memory, 18);
            run1 = false;
        }
    }
    release(memory, criticalSection);

    // wait four seconds before adding more westward monkeys
    this_thread::sleep_for(chrono::seconds(4));

    bool run2 = true;
    while(run2) {
        if(memory->criticalSection == 1) {
            acquire(memory, criticalSection);
            addWestwardMonkey(memory, 19);
            addWestwardMonkey(memory, 20);
            addWestwardMonkey(memory, 21);
            addWestwardMonkey(memory, 22);
            addWestwardMonkey(memory, 23);
            addWestwardMonkey(memory, 24);
            run2 = false;
        }
    }
    release(memory, criticalSection);

    return NULL;
}

void* monkeyConsumer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    while(true) {
        if(memory->criticalSection == 1) {
            if(memory->numOfEastwardMonkeys > 0 && memory->westwardMutex == 1 && memory->monkeysOnRopeSemaphore < 5) {
                acquire(memory, criticalSection);
                Monkey eastwardMonkey = removeEastwardMonkey((memory));

                // if the rope is not full and the westward mutex is not locked, add an eastward monkey to the rope
                memory->monkeysOnRope.push(eastwardMonkey);
                enterRope(eastwardMonkey.id, eastwardMonkey.destination, memory);
                release(memory, criticalSection);
            }
            else if(memory->numOfWestwardMonkeys > 0 && memory->eastwardMutex == 1 && memory->monkeysOnRopeSemaphore < 5) {
                acquire(memory, criticalSection);
                Monkey westwardMonkey = removeWestwardMonkey(memory);

                // if the rope is not full and the eastward mutex is not locked, add a westward monkey to the rope
                memory->monkeysOnRope.push(westwardMonkey);
                enterRope(westwardMonkey.id, westwardMonkey.destination, memory);
                release(memory, criticalSection);
            }

        }
    }

    return NULL;
}

int main() {
    SharedMemory *sharedMemory = &sMem;

    // three threads, consumer=monkeyConsumer, producer1=eastward, producer2=westward
    pthread_t tidMonkeyConsumer;
    pthread_t tidEastwardProducer;
    pthread_t tidWestwardProducer;
    pthread_attr_t attrMonkeyConsumer;
    pthread_attr_t attrEastwardProducer;
    pthread_attr_t attrWestwardProducer;

    pthread_attr_init(&attrEastwardProducer);
    pthread_attr_init(&attrWestwardProducer);
    pthread_attr_init(&attrMonkeyConsumer);

    pthread_create(&tidEastwardProducer, &attrEastwardProducer, eastwardProducer, sharedMemory);
    pthread_create(&tidWestwardProducer, &attrWestwardProducer, westwardProducer, sharedMemory);

    // wait some time before starting consumer thread
    this_thread::sleep_for(chrono::seconds(3));
    pthread_create(&tidMonkeyConsumer, &attrMonkeyConsumer, monkeyConsumer, sharedMemory);

    pthread_join(tidEastwardProducer, NULL);
    pthread_join(tidWestwardProducer, NULL);
    pthread_join(tidMonkeyConsumer, NULL);

    return 0;
}