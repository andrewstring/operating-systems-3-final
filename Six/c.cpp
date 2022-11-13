#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <string>

using namespace std;

// access points for acquire and release
enum Mutex {
    criticalSection
};

// access points for wait and signal
enum Semaphore {
    studentsOnRopeSemaphore,
    eastwardStudentsOnRopeSemaphore,
    westwardStudentsOnRopeSemaphore
};

enum Destination {
    east,
    west
};

string destinationOutput[2] = {"east", "west"};

struct SharedMemory;

struct Student {
    int id;
    Destination destination;
    SharedMemory *sharedMemory;
    Student(int idPass, Destination destinationPass, SharedMemory *sharedMemoryPass);
};

// constructor function for student struct
Student::Student(int idPass, Destination destinationPass, SharedMemory *sharedMemoryPass) {
    id = idPass;
    destination = destinationPass;
    sharedMemory = sharedMemoryPass;
}

struct SharedMemory {
    int criticalSection = 1;
    int studentsOnRopeSemaphore = 0;
    int eastwardStudentsOnRopeSemaphore = 0;
    int westwardStudentsOnRopeSemaphore = 0;
    queue<Student> eastwardStudents;
    queue<Student> westwardStudents;
    queue<Student> studentsOnRope;
    int numOfEastwardStudents;
    int numOfWestwardStudents;
    int ropeCapacity;
    int eastwardRopeCapacity;
    int westwardRopeCapacity;
} sMem;

void acquire(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case criticalSection:
            if(sharedMemory->criticalSection == 1) {
                sharedMemory->criticalSection = 0;
            }
            break;
    }
}

void release(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case criticalSection:
            if(sharedMemory->criticalSection == 0) {
                sharedMemory->criticalSection = 1;
            }
            break;
    }
}

void wait(SharedMemory *sharedMemory, Semaphore toAccess) {
    switch(toAccess) {
        case studentsOnRopeSemaphore:
            if(sharedMemory->studentsOnRopeSemaphore < sharedMemory->ropeCapacity) {
                sharedMemory->studentsOnRopeSemaphore++;
            }
            break;
        case eastwardStudentsOnRopeSemaphore:
            if(sharedMemory->eastwardStudentsOnRopeSemaphore < sharedMemory->eastwardRopeCapacity) {
                sharedMemory->eastwardStudentsOnRopeSemaphore++;
            }
            break;
        case westwardStudentsOnRopeSemaphore:
            if(sharedMemory->westwardStudentsOnRopeSemaphore < sharedMemory->westwardRopeCapacity) {
                sharedMemory->westwardStudentsOnRopeSemaphore++;
            }
            break;
    }
}

void signal(SharedMemory *sharedMemory, Semaphore toAccess) {
    switch(toAccess) {
        case studentsOnRopeSemaphore:
            if(sharedMemory->studentsOnRopeSemaphore > 0) {
                sharedMemory->studentsOnRopeSemaphore--;
            }
            break;
        case eastwardStudentsOnRopeSemaphore:
            if(sharedMemory->eastwardStudentsOnRopeSemaphore > 0) {
                sharedMemory->eastwardStudentsOnRopeSemaphore--;
            }
            break;
        case westwardStudentsOnRopeSemaphore:
            if(sharedMemory->westwardStudentsOnRopeSemaphore > 0) {
                sharedMemory->westwardStudentsOnRopeSemaphore--;
            }
            break;
    }
}

// enables main function to set the rope capacity
void setRopeCapacity(SharedMemory *sharedMemory, int capacity) {
    sharedMemory->ropeCapacity = capacity;
}

// enables main function to set the number of eastward students that can cross rope at once
void setEastwardRopeCapacity(SharedMemory *sharedMemory, int capacity) {
    sharedMemory->eastwardRopeCapacity = capacity;
}

// enables main function to set the number of westward students that can cross the rope at once
void setWestwardRopeCapacity(SharedMemory *sharedMemory, int capacity) {
    sharedMemory->westwardRopeCapacity = capacity;
}

void addEastwardStudent(SharedMemory *sharedMemory, int id) {
    Student eastwardStudent = Student(id, east, sharedMemory);
    sharedMemory->numOfEastwardStudents++;

    // create and add an eastward student to the eastward student queue
    sharedMemory->eastwardStudents.push(eastwardStudent);
    cout << "Student with id " + to_string(id) + " headed to the " +
    destinationOutput[eastwardStudent.destination] + " has been added\n";
    cout.flush();
}

void addWestwardStudent(SharedMemory *sharedMemory, int id) {
    Student westwardStudent = Student(id, west, sharedMemory);
    sharedMemory->numOfWestwardStudents++;

    // create and add a westward student to the westward student queue
    sharedMemory->westwardStudents.push(westwardStudent);
    cout << "Student with id " + to_string(id) + " headed to the " +
    destinationOutput[westwardStudent.destination] + " has been added\n";
    cout.flush();
}

Student removeEastwardStudent(SharedMemory *sharedMemory) {
    Student retrievedEastwardStudent = sharedMemory->eastwardStudents.front();
    
    // remove an eastward student from the eastward student queue
    sharedMemory->eastwardStudents.pop();
    sharedMemory->numOfEastwardStudents--;
    return retrievedEastwardStudent;
}

Student removeWestwardStudent(SharedMemory *sharedMemory) {
    Student retrievedWestwardStudent = sharedMemory->westwardStudents.front();

    // remove a westward student from the westward student queue
    sharedMemory->westwardStudents.pop();
    sharedMemory->numOfWestwardStudents--;
    return retrievedWestwardStudent;
}

void* crossRope(void *studentRef) {
    Student *student = (struct Student *) studentRef;

    // take two seconds for student to cross rope
    this_thread::sleep_for(chrono::seconds(2));
    cout << "Student with id " + to_string(student->id) + " headed to the " + destinationOutput[student->destination] +
    " has crossed the rope\n";
    cout.flush();

    // signal the student on rope semaphore once the student has crossed the rope
    signal(student->sharedMemory, studentsOnRopeSemaphore);
    if(student->destination == east) {
        signal(student->sharedMemory, eastwardStudentsOnRopeSemaphore);
    } else if(student->destination == west) {
        signal(student->sharedMemory, westwardStudentsOnRopeSemaphore);
    }

    return NULL;
}

void enterRope(int id, Destination destination, SharedMemory *sharedMemory) {
    if(destination == east) {
        // call wait on the eastward student semaphore if an eastward student enters the rope
        wait(sharedMemory, eastwardStudentsOnRopeSemaphore);
    } else if (destination == west) {
        // call wait on the westward student semaphore if a westward student enters the rope
        wait(sharedMemory, westwardStudentsOnRopeSemaphore);
    }

    wait(sharedMemory, studentsOnRopeSemaphore);

    pthread_t tidStudent;
    pthread_attr_t attrStudent;
    pthread_attr_init(&attrStudent);

    cout << "Student with id " + to_string(id) + " headed to the " + destinationOutput[destination] +
    " has got on the rope\n";
    cout.flush();

    // create student thread
    pthread_create(&tidStudent, &attrStudent, crossRope, new Student(id, destination, sharedMemory));
}

void* eastwardProducer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    bool run1 = true;
    while(run1) {
        if(memory->criticalSection == 1) {
            acquire(memory, criticalSection);

            addEastwardStudent(memory, 1);
            addEastwardStudent(memory, 2);
            addEastwardStudent(memory, 3);
            addEastwardStudent(memory, 4);
            addEastwardStudent(memory, 5);
            addEastwardStudent(memory, 6);
            run1 = false;
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
            addWestwardStudent(memory, 13);
            addWestwardStudent(memory, 14);
            addWestwardStudent(memory, 15);
            addWestwardStudent(memory, 16);
            addWestwardStudent(memory, 17);
            addWestwardStudent(memory, 18);
            run1 = false;
        }
    }
    release(memory, criticalSection);

    return NULL;
}

void* studentConsumer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    while(true) {
        if(memory->criticalSection == 1) {
            if(memory->numOfEastwardStudents > 0
            && memory->eastwardStudentsOnRopeSemaphore < memory->eastwardRopeCapacity
            && memory->studentsOnRopeSemaphore < memory->ropeCapacity) {
                acquire(memory, criticalSection);
                Student eastwardStudent = removeEastwardStudent(memory);

                // if the number of total students and eastward students on rope are not full add the eastward student to the rope
                memory->studentsOnRope.push(eastwardStudent);
                enterRope(eastwardStudent.id, eastwardStudent.destination, memory);
                release(memory, criticalSection);
            }
            else if(memory->numOfWestwardStudents > 0
            && memory->westwardStudentsOnRopeSemaphore < memory->westwardRopeCapacity
            && memory->studentsOnRopeSemaphore < memory->ropeCapacity) {
                acquire(memory, criticalSection);
                Student westwardStudent = removeWestwardStudent(memory);

                // if the number of total students and westward students on rope are not full add the westward student to the rope
                memory->studentsOnRope.push(westwardStudent);
                enterRope(westwardStudent.id, westwardStudent.destination, memory);
                release(memory, criticalSection);
            }
        }
    }

    return NULL;
}

int main() {
    SharedMemory *sharedMemory = &sMem;

    // set the different capacities for the rope
    setRopeCapacity(sharedMemory, 5);
    setEastwardRopeCapacity(sharedMemory, 3);
    setWestwardRopeCapacity(sharedMemory, 3);

    // three threads, consumer=studentConsumer, producer1=eastward, producer2=westward
    pthread_t tidStudentConsumer;
    pthread_t tidEastwardProducer;
    pthread_t tidWestwardProducer;
    pthread_attr_t attrStudentConsumer;
    pthread_attr_t attrEastwardProducer;
    pthread_attr_t attrWestwardProducer;

    pthread_attr_init(&attrStudentConsumer);
    pthread_attr_init(&attrEastwardProducer);
    pthread_attr_init(&attrWestwardProducer);

    pthread_create(&tidEastwardProducer, &attrEastwardProducer, eastwardProducer, sharedMemory);
    pthread_create(&tidWestwardProducer, &attrWestwardProducer, westwardProducer, sharedMemory);

    // wait some time before starting the consumer thread
    this_thread::sleep_for(chrono::seconds(3));
    pthread_create(&tidStudentConsumer, &attrStudentConsumer, studentConsumer, sharedMemory);

    pthread_join(tidEastwardProducer, NULL);
    pthread_join(tidWestwardProducer, NULL);
    pthread_join(tidStudentConsumer, NULL);

    return 0;
}