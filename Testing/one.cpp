#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <string>

using namespace std;

// access points for wait and signal
enum Semaphore {
    chairSem,
};

enum Mutex {
    barberMut,
    criticalSection
};

// will keep a queue so that first customers will be served first
struct SharedMemory {
    int barberMutex = 1;
    int criticalSection = 1;
    int chairSemaphore = 0;
    int numOfChairs = 0;
    queue<string *> customersInShop;
} sMem;

void wait(SharedMemory *sharedMemory, Semaphore toAccess) {
    switch(toAccess) {
        case chairSem:
            if (sharedMemory->chairSemaphore < sharedMemory->numOfChairs) {
                sharedMemory->chairSemaphore++;
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


// setup the number of chairs that are open to sit in
void setNumChairs(SharedMemory *sharedMemory, int numChairs) {
    sharedMemory->numOfChairs = numChairs;
}

void enterBarberShop(SharedMemory *sharedMemory, string *customer) {
    acquire(sharedMemory, criticalSection);

    // only allow customers to enter if there are open seats
    if (sharedMemory->chairSemaphore >= sharedMemory->numOfChairs) {
        //cout << "Barber shop is full..." + *customer + " did not enter\n";
        //cout.flush();
    }
    else {
        sharedMemory->customersInShop.push(customer);
        //cout << *customer + " has entered the barber shop\n";
        //cout.flush();
        wait(sharedMemory,chairSem);
    }

    release(sharedMemory, criticalSection);
}

void leaveBarberShop(SharedMemory *sharedMemory, string *customer) {
    release(sharedMemory, barberMut);

    //cout << *customer + " left the barber shop\n";
    //cout.flush();
}

void cutHair(SharedMemory *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory*) sharedMemory;

    acquire(sharedMemory, barberMut);

    

    // dequeue customer having their haircut
    string *customer = memory->customersInShop.front();
    //cout << "Started cutting " + *customer + "'s hair\n";
    //cout.flush();

    this_thread::sleep_for(chrono::seconds(2));

    memory->customersInShop.pop();

    // TEST 3 VARIABLE 1
    int numCustomers = memory->chairSemaphore;

    // TEST 3
    if(memory->customersInShop.size() == numCustomers - 1) {
        cout << "TEST 3: SUCCESS - Num of customers remains the same after hair is cut but customer is about to leave\n";
    } else {
        cout << "TEST 3: FAILED\n";
    }

    //cout << "Finished cutting " + *customer + "'s hair\n";
    //cout.flush();

    leaveBarberShop(memory, customer);
}



void* barber(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    // cut hair if there are customers in the shop and not busy
    while (true) {
        if (memory->criticalSection == 1) {
            if (memory->chairSemaphore > 0 && memory->barberMutex == 1) {
                cutHair(memory);
                signal(memory, chairSem);
            }
        }
    }

    return NULL;
}

void* producer(void *sharedMemory) {
    SharedMemory *memory = (SharedMemory *) sharedMemory;

    bool run = true;

    while (true) {
        // only run producer code once
        if (run) {
            // sample names
            string people[26] = {"Alpha", "Bravo", "Charlie", "Delta", "Echo",
                                "Foxtrot", "Golf", "Hotel", "India", "Juliet",
                                "Kilo", "Lima", "Mike", "November", "Oscar",
                                "Papa", "Quebec", "Romeo", "Sierra", "Tango",
                                "Uniform", "Victor", "Whisky", "X-Ray", "Yankee",
                                "Zulu"};
            enterBarberShop(memory, &people[0]);
            enterBarberShop(memory, &people[1]);
            enterBarberShop(memory, &people[2]);

            // TEST 1
            if(memory->chairSemaphore == memory->numOfChairs) {
                cout << "TEST 1: SUCCESS - People successfully entered barber shop\n";
            } else {
                cout << "TEST 1: FAILED\n"; 
            } 

            enterBarberShop(memory, &people[3]);
            enterBarberShop(memory, &people[4]);

            // TEST 2
            if(memory->chairSemaphore == memory->numOfChairs) {
                cout << "TEST 2: SUCCESS - Barber shop only admitted max number of seats\n";
            } else {
                cout << "TEST 2: FAILED\n"; 
            } 

            // wait before entering more people
            this_thread::sleep_for(chrono::seconds(3));
            enterBarberShop(memory, &people[5]);
            enterBarberShop(memory, &people[6]);
            enterBarberShop(memory, &people[7]);
            enterBarberShop(memory, &people[8]);

            // wait before entering more people
            this_thread::sleep_for(chrono::seconds(10));
            enterBarberShop(memory, &people[9]);

            //prevent from running again
            run = false;
        }
    }

    return NULL;
}

int main() {
    SharedMemory *sharedMemory = &sMem;

    setNumChairs(sharedMemory, 3);

    // two threads, consumer=barber, producer=waiting area (for customers)
    pthread_t tidBarber;
    pthread_t tidProducer;
    pthread_attr_t attrBarber;
    pthread_attr_t attrProducer;

    pthread_attr_init(&attrBarber);
    pthread_attr_init(&attrProducer);
    pthread_create(&tidBarber, &attrBarber, barber, sharedMemory);
    pthread_create(&tidProducer, &attrProducer, producer, sharedMemory);


    pthread_join(tidBarber, NULL);
    pthread_join(tidProducer, NULL);

    return 0;
}