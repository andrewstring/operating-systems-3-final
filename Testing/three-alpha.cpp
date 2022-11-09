#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <string>
#include "testing.h"

using namespace std;

// access points for wait and signal
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
    // we will use a queue for students in hallway so that first student will be served first
    queue<string *> studentsInHallway;
    // TA queue will only ever be max length 1, but this makes it easy to push student into the
    // TAs office and pop them out once they are done
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
    // only allow students to enter hallway if it is not full
    if (sharedMemory->chairHallwaySemaphore >= sharedMemory->numOfChairsHallway) {
        assertLessThanEqualTo(
            sharedMemory->numOfChairsHallway,
            sharedMemory->chairHallwaySemaphore,
            "Full semaphore prevented admission of student",
            "Full semaphore did not prevent admission of student"
        );
        //cout << "Hallway is full..." + *student + " did not enter\n";
        //cout.flush();
    }
    else {
        assertLessThan(
            sharedMemory->chairHallwaySemaphore,
            sharedMemory->numOfChairsHallway,
            "Non-full semaphore allowed admission of student",
            "Non-full semaphore did not allow admission of student"
        );
        //cout << *student + " has sat down in the hallway\n";
        wait(sharedMemory, chairHallwaySem);
        sharedMemory->studentsInHallway.push(student);
        //cout.flush();
    }

    release(sharedMemory, criticalSection);
}

string* enterTaOffice(SharedMemory *sharedMemory) {
    // only enter TA office if the TA is not busy
    if (sharedMemory->taMutex == 1) {
        string *studentFromHallway = sharedMemory->studentsInHallway.front();
        //cout << *studentFromHallway + " has entered TA's office\n";
        //cout.flush();
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
    //cout << "TA started helping " + *student + "\n";
    //cout.flush();

    // this sleep will be the amount of time that the TA is spending with student
    this_thread::sleep_for(chrono::seconds(2));
    //cout << "TA finished helping " + *student + "\n";
    //cout.flush();
    //cout << *student + " has left the TA's office\n";
    //cout.flush();
    sharedMemory->studentWithTa.pop();
    release(sharedMemory, taMut);
    assertInt(
        sharedMemory->studentWithTa.size(),
        0,
        "Released TA Mutex when student with TA queue is empty",
        "Released TA Mutex when student with TA queue was not empty"
    );
}

void* ta(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;
    while(true) {
        if(memory->criticalSection == 1) {
            // only help student when the hallway is not empty and TA is not already
            // busy with a student
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
        // only run when we have not finished (mac will give an error if this thread stops)
        if (run) {
            // array of student names
            string students[26] = {"Alpha", "Bravo", "Charlie", "Delta", "Echo",
                                "Foxtrot", "Golf", "Hotel", "India", "Juliet",
                                "Kilo", "Lima", "Mike", "November", "Oscar",
                                "Papa", "Quebec", "Romeo", "Sierra", "Tango",
                                "Uniform", "Victor", "Whisky", "X-Ray", "Yankee",
                                "Zulu"};

            // keep waiting to add students until critical section is free
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

            // wait some time until adding more students
            this_thread::sleep_for(chrono::seconds(6));

            // keep waiting to add students until critical section is free
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
    
            //prevent from running again
            run = false;
        }
    }

    return NULL;
}

int main() {

    SharedMemory *sharedMemory = &sMem;

    // we will have two threads - producer=students entering hallway, consumer=TA
    pthread_t tidTa;
    pthread_t tidProducer;
    pthread_attr_t attrTa;
    pthread_attr_t attrProducer;

    pthread_attr_init(&attrTa);
    pthread_attr_init(&attrProducer);
    pthread_create(&tidTa, &attrTa, ta, sharedMemory);
    pthread_create(&tidProducer, &attrProducer, producer, sharedMemory);

    pthread_join(tidTa, NULL);
    pthread_join(tidProducer, NULL);

    return 0;
}