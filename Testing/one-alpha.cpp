#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <string>
#include "testing.h"

using namespace std;

enum Semaphore {
    chairSem,
};

enum Mutex {
    barberMut,
    criticalSection
};

struct SharedMemory {
    int barberMutex = 1;
    int criticalSection = 1;
    int chairSemaphore = 0;
    int numOfChairs = 0;
    int testCounter = 0;
    queue<string *> customersInShop;
} sMem;

void wait(SharedMemory *sharedMemory, Semaphore toAccess) {
    switch(toAccess) {
        case chairSem:
            if (sharedMemory->chairSemaphore < sharedMemory->numOfChairs) {
                sharedMemory->chairSemaphore++;
                assertLessThanEqualTo(
                    sharedMemory->chairSemaphore,
                    sharedMemory->numOfChairs,
                    "Chair semaphore less than or equal to the number of chairs",
                    "Chair semaphore went out of bounds - larger than number of chairs"
                );
            }
            break;
    }
}

void acquire(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case barberMut:
            if(sharedMemory->barberMutex == 1) {
                sharedMemory->barberMutex = 0;
            }
            break;
        case criticalSection:
            if(sharedMemory->criticalSection == 1) {
                sharedMemory->criticalSection = 0;
            }
            break;
    }
}

void signal(SharedMemory *sharedMemory, Semaphore toAccess) {
    switch(toAccess) {
        case chairSem:
            if (sharedMemory->chairSemaphore > 0) {
                sharedMemory->chairSemaphore--;
            }
            break;
    }
}

void release(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case barberMut:
            if(sharedMemory->barberMutex == 0) {
                sharedMemory->barberMutex = 1;
            }
            break;
        case criticalSection:
            if(sharedMemory->criticalSection == 0) {
                sharedMemory->criticalSection = 1;
            }
            break;
    }

}


void setNumChairs(SharedMemory *sharedMemory, int numChairs) {
    sharedMemory->numOfChairs = numChairs;
    assertInt(
        sharedMemory->numOfChairs,
        numChairs,
        "Chair semaphore limit was set correctly",
        "Chair semaphore limit was set incorrectly"
    );
}

void enterBarberShop(SharedMemory *sharedMemory, string *customer) {
    acquire(sharedMemory, criticalSection);

    if (sharedMemory->chairSemaphore >= sharedMemory->numOfChairs) {
        assertLessThanEqualTo(
            sharedMemory->numOfChairs,
            sharedMemory->chairSemaphore,
            "Full semaphore prevented admission of customer",
            "Full semaphore did not prevent admission of customer");
    }
    else {
        assertLessThan(
            sharedMemory->chairSemaphore,
            sharedMemory->numOfChairs,
            "Non-full semaphore allowed admission of customer",
            "Non-full semaphore did not allow admission of customer");
        sharedMemory->customersInShop.push(customer);
        wait(sharedMemory,chairSem);
    }

    release(sharedMemory, criticalSection);
}

void leaveBarberShop(SharedMemory *sharedMemory, string *customer) {
    release(sharedMemory, barberMut);
    assertInt(
        sharedMemory->barberMutex,
        1,
        "Barber Mutex successful release",
        "Barber Mutex unsuccessful release");
    sharedMemory->testCounter++;
}

void cutHair(SharedMemory *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory*) sharedMemory;

    acquire(sharedMemory, barberMut);
    assertInt(
        sharedMemory->barberMutex,
        0,
        "Barber Mutex successful acquire",
        "Barber Mutex unsuccessful acquire"
    );

    string *customer = memory->customersInShop.front();

    this_thread::sleep_for(chrono::seconds(2));

    memory->customersInShop.pop();
    leaveBarberShop(memory, customer);
}



void* barber(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    while (true) {
        if (memory->criticalSection == 1) {
            if (memory->chairSemaphore > 0 && memory->barberMutex == 1) {
                cutHair(memory);
                signal(memory, chairSem);
                assertInt(
                    memory->chairSemaphore,
                    memory->customersInShop.size(),
                    "Customer has left - customer queue is same size value as chairSemaphore",
                    "Customer has left - customer queue is not same size value as chairSemaphore"
                );
            }
        }
    }

    return NULL;
}

void* producer(void *sharedMemory) {
    SharedMemory *memory = (SharedMemory *) sharedMemory;

    bool run = true;

    while (true) {
        if (run) {
            string people[26] = {"Alpha", "Bravo", "Charlie", "Delta", "Echo",
                                "Foxtrot", "Golf", "Hotel", "India", "Juliet",
                                "Kilo", "Lima", "Mike", "November", "Oscar",
                                "Papa", "Quebec", "Romeo", "Sierra", "Tango",
                                "Uniform", "Victor", "Whisky", "X-Ray", "Yankee",
                                "Zulu"};
            enterBarberShop(memory, &people[0]);
            enterBarberShop(memory, &people[1]);
            enterBarberShop(memory, &people[2]);
            enterBarberShop(memory, &people[3]);
            enterBarberShop(memory, &people[4]);

            this_thread::sleep_for(chrono::seconds(3));
            enterBarberShop(memory, &people[5]);
            enterBarberShop(memory, &people[6]);
            enterBarberShop(memory, &people[7]);
            enterBarberShop(memory, &people[8]);

            this_thread::sleep_for(chrono::seconds(10));
            enterBarberShop(memory, &people[9]);
            enterBarberShop(memory, &people[10]);
            enterBarberShop(memory, &people[11]);

            this_thread::sleep_for(chrono::seconds(5));
            enterBarberShop(memory, &people[12]);

            assertInt(
                memory->customersInShop.size(),
                memory->chairSemaphore,
                "All customers have left: size of customer queue and chairSemaphore are zero",
                "Customer queue or chairSemaphore ERROR: both should equal zero when all customers have left"
            );

            run = false;
        }
    }

    return NULL;
}

int main() {
    SharedMemory *sharedMemory = &sMem;

    setNumChairs(sharedMemory, 3);

    pthread_t tidBarber;
    pthread_t tidProducer;
    pthread_attr_t attrBarber;
    pthread_attr_t attrProducer;

    pthread_attr_init(&attrBarber);
    pthread_attr_init(&attrProducer);
    pthread_create(&tidBarber, &attrBarber, barber, sharedMemory);
    pthread_create(&tidProducer, &attrProducer, producer, sharedMemory);
    

    bool testInProgress = true;

    while (testInProgress) {
        // we use 5 since there will be 5 customer in barber shop
        if(sharedMemory->testCounter >= 5) {
            this_thread::sleep_for(chrono::seconds(5));
            endTesting();
            testInProgress = false;
        }
    }


    pthread_join(tidBarber, NULL);
    pthread_join(tidProducer, NULL);

    return 0;
}
