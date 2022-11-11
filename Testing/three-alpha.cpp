#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <string>
#include "testing.h"

using namespace std;

enum Semaphore {
    chairHallwaySem,
};

enum Mutex {
    taMut,
    criticalSection
};

struct SharedMemory {
    int taMutex = 1;
    int criticalSection = 1;
    int chairHallwaySemaphore = 0;
    int numOfChairsHallway = 3;
    int testCounter = 0;
    queue<string *> studentsInHallway;
    queue<string *> studentWithTa;
} sMem;

void wait(SharedMemory *sharedMemory, Semaphore toAccess) {
    switch (toAccess) {
        case chairHallwaySem:
            if (sharedMemory->chairHallwaySemaphore < sharedMemory->numOfChairsHallway) {
                sharedMemory->chairHallwaySemaphore++;
                assertLessThanEqualTo(
                    sharedMemory->chairHallwaySemaphore,
                    sharedMemory->numOfChairsHallway,
                    "Hallway chair semaphore less than or equal to the number of hallway chairs",
                    "Hallway chair semaphore not less than or equal to the number of hallway chairs"
                );
            }
            break;
    }
}

void acquire(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case taMut:
            if(sharedMemory->taMutex == 1) {
                sharedMemory->taMutex = 0;
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
        case chairHallwaySem:
            if (sharedMemory->chairHallwaySemaphore > 0) {
                sharedMemory->chairHallwaySemaphore--;
            }
            break;
    }
}

void release(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case taMut:
            if(sharedMemory->taMutex == 0) {
                sharedMemory->taMutex = 1;
            }
            break;
        case criticalSection:
            if(sharedMemory->criticalSection == 0) {
                sharedMemory->criticalSection = 1;
            }
            break;
    }
}

void enterHallway(SharedMemory *sharedMemory, string *student) {
    acquire(sharedMemory, criticalSection);
    if (sharedMemory->chairHallwaySemaphore >= sharedMemory->numOfChairsHallway) {
        assertLessThanEqualTo(
            sharedMemory->numOfChairsHallway,
            sharedMemory->chairHallwaySemaphore,
            "Full semaphore prevented admission of student",
            "Full semaphore did not prevent admission of student"
        );
    }
    else {
        assertLessThan(
            sharedMemory->chairHallwaySemaphore,
            sharedMemory->numOfChairsHallway,
            "Non-full semaphore allowed admission of student",
            "Non-full semaphore did not allow admission of student"
        );
        wait(sharedMemory, chairHallwaySem);
        sharedMemory->studentsInHallway.push(student);
    }

    release(sharedMemory, criticalSection);
}

string* enterTaOffice(SharedMemory *sharedMemory) {
    if (sharedMemory->taMutex == 1) {
        string *studentFromHallway = sharedMemory->studentsInHallway.front();
        sharedMemory->studentsInHallway.pop();
        sharedMemory->studentWithTa.push(studentFromHallway);
        signal(sharedMemory, chairHallwaySem);
        assertInt(
            sharedMemory->chairHallwaySemaphore,
            sharedMemory->studentsInHallway.size(),
            "Hallway semaphore and size of students in hallway queue are the same",
            "Hallway semaphore and size of students in hallway queuea are not the same"
        );

        return studentFromHallway;
    }
    else {
        return NULL;
    }

}

void taHelpStudent(SharedMemory *sharedMemory) {
    string *student = enterTaOffice(sharedMemory);

    this_thread::sleep_for(chrono::seconds(2));
    sharedMemory->studentWithTa.pop();
    release(sharedMemory, taMut);
    assertInt(
        sharedMemory->studentWithTa.size(),
        0,
        "Released TA Mutex when student with TA queue is empty",
        "Released TA Mutex when student with TA queue was not empty"
    );
    sharedMemory->testCounter++;
}

void* ta(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;
    while(true) {
        if(memory->criticalSection == 1) {
            if (memory->chairHallwaySemaphore > 0 && memory->taMutex == 1) {
                assertLessThan(
                    0,
                    memory->chairHallwaySemaphore,
                    "Only admitting student into TAs office when hallway is not empty",
                    "Tying to admit students into TAs office when hallway is empty"
                );

                taHelpStudent(memory);
            }
        }
    }

    return NULL;
}

void* producer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;
    bool run = true;

    while (true) {
        if (run) {
            string students[26] = {"Alpha", "Bravo", "Charlie", "Delta", "Echo",
                                "Foxtrot", "Golf", "Hotel", "India", "Juliet",
                                "Kilo", "Lima", "Mike", "November", "Oscar",
                                "Papa", "Quebec", "Romeo", "Sierra", "Tango",
                                "Uniform", "Victor", "Whisky", "X-Ray", "Yankee",
                                "Zulu"};

            bool run1 = true;
            while(run1) {
                if (memory->criticalSection == 1) {
                    acquire(memory, criticalSection);
                    enterHallway(memory, &students[0]);
                    enterHallway(memory, &students[1]);
                    enterHallway(memory, &students[2]);
                    enterHallway(memory, &students[3]);
                    enterHallway(memory, &students[4]);
                    release(memory, criticalSection);
                    run1 = false;
                }
            }

            this_thread::sleep_for(chrono::seconds(6));

            bool run2 = true;
            while(run2) {
                if(memory->criticalSection == 1) {
                    acquire(memory, criticalSection);
                    enterHallway(memory, &students[5]);
                    enterHallway(memory, &students[6]);
                    enterHallway(memory, &students[7]);
                    enterHallway(memory, &students[8]);
                    enterHallway(memory, &students[9]);
                    release(memory, criticalSection);
                    run2 = false;
                    }
            }

            this_thread::sleep_for(chrono::seconds(8));

            assertInt(
                memory->studentsInHallway.size(),
                0,
                "Students in hallway queue was empty when all students have left",
                "Studens in hallway queue was not empty when all students have left"
            );
            assertInt(
                memory->studentWithTa.size(),
                0,
                "Student with TA queue was empty when all students have left",
                "Studens with TA queue was not empty when all students have left"
            );
    
            run = false;
        }
    }

    return NULL;
}

int main() {

    SharedMemory *sharedMemory = &sMem;

    pthread_t tidTa;
    pthread_t tidProducer;
    pthread_attr_t attrTa;
    pthread_attr_t attrProducer;

    pthread_attr_init(&attrTa);
    pthread_attr_init(&attrProducer);
    pthread_create(&tidTa, &attrTa, ta, sharedMemory);
    pthread_create(&tidProducer, &attrProducer, producer, sharedMemory);

    bool testInProgress = true;
    while(testInProgress) {
        // we use 6 since 6 students will enter the hallway successfully
        if(sharedMemory->testCounter >= 6) {
            this_thread::sleep_for(chrono::seconds(4));
            endTesting();
            testInProgress = false;
        }
    }


    pthread_join(tidTa, NULL);
    pthread_join(tidProducer, NULL);

    return 0;
}