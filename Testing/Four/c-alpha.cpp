#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <string>
#include <tuple>
#include "../testing.h"

using namespace std;

enum Mutex {
    readerMut,
    writerMut,
    criticalSection
};

enum Type {
    reader,
    writer
};

string typeOutput[2] = {"reader", "writer"};

struct SharedMemory {
    int readerMutex = 1;
    int writerMutex = 1;
    int criticalSection = 1;
    int testCounter = 0;
    queue<tuple<Type, string *>> people;
    queue<tuple<Type, string *>> peopleInDatabase;
    int numOfPeople = 0;
    int numOfPeopleInDatabase = 0;
    int numOfReadersInDatabase = 0;
    int numOfWritersInDatabase = 0;
} sMem;

void acquire(SharedMemory *sharedMemory, Mutex toAccess) {
    switch(toAccess) {
        case readerMut:
            if (sharedMemory->readerMutex == 1) {
                sharedMemory->readerMutex = 0;
            }
            break;
        case writerMut:
            if (sharedMemory->writerMutex == 1) {
                sharedMemory->writerMutex = 0;
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
        case readerMut:
            if (sharedMemory->readerMutex == 0) {
                sharedMemory->readerMutex = 1;
            }
            break;
        case writerMut:
            if (sharedMemory->writerMutex == 0) {
                sharedMemory->writerMutex = 1;
            }
            break;
        case criticalSection:
            if (sharedMemory->criticalSection == 0) {
                sharedMemory->criticalSection = 1;
            }
            break;
    }
}
void addReader(SharedMemory *sharedMemory, string *name) {
        tuple<Type, string*> newReader = make_tuple(reader, name);
        sharedMemory->numOfPeople++;
        sharedMemory->people.push(newReader);
        assertInt(
            sharedMemory->numOfPeople,
            sharedMemory->people.size(),
            "Number of people and size of people queue are equal when new reader is added",
            "Number of people and size of people queue are not equal when new reader is added"
        );
}

void addWriter(SharedMemory *sharedMemory, string *name) {
    tuple<Type, string*> newWriter = make_tuple(writer, name);
    sharedMemory->numOfPeople++;
    sharedMemory->people.push(newWriter);
    assertInt(
        sharedMemory->numOfPeople,
        sharedMemory->people.size(),
        "Number of people and size of people queue are equal when new writer is added",
        "Number of people and size of people queue are not equal when new writer is added"
    );
}

Type getTypeAtFront(SharedMemory *sharedMemory) {
    return get<0>(sharedMemory->people.front());
}

tuple<Type, string*> removePerson(SharedMemory *sharedMemory) {
    tuple<Type, string*> retrievedPerson = sharedMemory->people.front();
    sharedMemory->numOfPeople--;
    sharedMemory->people.pop();
    assertInt(
        sharedMemory->numOfPeople,
        sharedMemory->people.size(),
        "Number of people and size of people queue are equal when a person is remove",
        "Number of people and size of people queue are not equal when a person is remove"
    );

    return retrievedPerson;
}

void enterDatabase(SharedMemory *sharedMemory, tuple<Type, string*> person) {
    if (get<0>(person) == reader) {
        if(sharedMemory->writerMutex == 1) {
            acquire(sharedMemory, readerMut);
            assertInt(
                sharedMemory->readerMutex,
                0,
                "Reader mutex was successfully acquired",
                "Reader mutex was not successfully acquired"
            );
            sharedMemory->numOfReadersInDatabase++;
            sharedMemory->numOfPeopleInDatabase++;
            sharedMemory->peopleInDatabase.push(person);
            assertInt(
                sharedMemory->numOfPeopleInDatabase,
                sharedMemory->peopleInDatabase.size(),
                "Number of people in database and size of people in database queue are equal when reader enters database",
                "Number of people in database and size of people in database queue are not equal when reader enters database"
            );
        }
    }

    if (get<0>(person) == writer) {
        if(sharedMemory->readerMutex == 1 && sharedMemory->writerMutex == 1) {
            acquire(sharedMemory, writerMut);
            assertInt(
                sharedMemory->writerMutex,
                0,
                "Writer mutex was successfully acquired",
                "Writer mutex was not successfully acquired"
            );
            sharedMemory->numOfWritersInDatabase++;
            sharedMemory->numOfPeopleInDatabase++;
            sharedMemory->peopleInDatabase.push(person);
            assertInt(
                sharedMemory->numOfPeopleInDatabase,
                sharedMemory->peopleInDatabase.size(),
                "Number of people in database and size of people in database queue are equal when writer enters database",
                "Number of people in database and size of people in database queue are not equal when writer enters database"
            );
        }
    }
}

void leaveDatabase(SharedMemory *sharedMemory) {
        tuple<Type, string*> personLeaving = sharedMemory->peopleInDatabase.front();
        if(get<0>(personLeaving) == reader) {
            sharedMemory->peopleInDatabase.pop();
            sharedMemory->numOfReadersInDatabase--;
            sharedMemory->numOfPeopleInDatabase--;
            assertInt(
                sharedMemory->numOfPeopleInDatabase,
                sharedMemory->peopleInDatabase.size(),
                "Number of people in database and size of people in database queue are equal when reader is removed from database",
                "Number of people in database and size of people in database queue are not equal when reader is removed from database"
            );
            if (sharedMemory->numOfReadersInDatabase == 0) {
                release(sharedMemory, readerMut);
                assertInt(
                    sharedMemory->readerMutex,
                    1,
                    "Reader mutex was successfully released",
                    "Reader mutex was not successfully released"
                );
            }
        }

        if(get<0>(personLeaving) == writer) {
            sharedMemory->peopleInDatabase.pop();
            sharedMemory->numOfWritersInDatabase--;
            sharedMemory->numOfPeopleInDatabase--;
            assertInt(
                sharedMemory->numOfPeopleInDatabase,
                sharedMemory->peopleInDatabase.size(),
                "Number of people in database and size of people in database queue are equal when writer is removed from database",
                "Number of people in database and size of people in database queue are not equal when writer is removed from database"
            );
            release(sharedMemory, writerMut);
                assertInt(
                    sharedMemory->writerMutex,
                    1,
                    "Writer mutex was successfully released",
                    "Writer mutex was not successfully released"
                );
        }
    sharedMemory->testCounter++;
}

void* database(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    while(true) {
        this_thread::sleep_for(chrono::seconds(2));
        if(memory->numOfPeopleInDatabase > 0) {
            leaveDatabase(memory);
        }
    }

    return NULL;
}

void* producer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    while(true) {
        if(memory->criticalSection == 1) {
            if(memory->numOfPeople > 0) {
                Type nextUpType = getTypeAtFront(memory);
                if(nextUpType == reader) {
                    if(memory->writerMutex == 1) {
                        acquire(memory, criticalSection);
                        tuple<Type, string*> readerEnteringDatabase = removePerson(memory);
                        enterDatabase(memory, readerEnteringDatabase);
                        release(memory, criticalSection);
                    }
                }
                else if(nextUpType == writer) {
                    if(memory->readerMutex == 1 && memory->writerMutex == 1) {
                        acquire(memory, criticalSection);
                        tuple<Type, string*> writerEnteringDatabase = removePerson(memory);
                        enterDatabase(memory, writerEnteringDatabase);
                        release(memory, criticalSection);
                    }
                }
            }
        }
    }
}

void addPeople(SharedMemory* sharedMemory, string *students) {
    bool run1 = true;
    while(run1) {
        if(sharedMemory->criticalSection == 1) {
            acquire(sharedMemory, criticalSection);
            addReader(sharedMemory, &students[24]);
            addWriter(sharedMemory, &students[18]);
            addWriter(sharedMemory, &students[19]);
            addReader(sharedMemory, &students[0]);
            addReader(sharedMemory, &students[1]);
            addReader(sharedMemory, &students[2]);
            addWriter(sharedMemory, &students[3]);
            addWriter(sharedMemory, &students[4]);
            addWriter(sharedMemory, &students[5]);
            run1 = false;
        }
    }
    release(sharedMemory, criticalSection);

    this_thread::sleep_for(chrono::seconds(5));

    bool run2 = true;
    while(run2) {
        if(sharedMemory->criticalSection == 1) {
            acquire(sharedMemory, criticalSection);
            addReader(sharedMemory, &students[10]);
            addWriter(sharedMemory, &students[11]);
            addReader(sharedMemory, &students[12]);
            addWriter(sharedMemory, &students[13]);
            addReader(sharedMemory, &students[14]);
            addWriter(sharedMemory, &students[15]);
            run2 = false;
        }
    }
    release(sharedMemory, criticalSection);
}



int main() {
    SharedMemory *sharedMemory = &sMem;

    string people[26] = {"Alpha", "Bravo", "Charlie", "Delta", "Echo",
                           "Foxtrot", "Golf", "Hotel", "India", "Juliet",
                           "Kilo", "Lima", "Mike", "November", "Oscar",
                           "Papa", "Quebec", "Romeo", "Sierra", "Tango",
                           "Uniform", "Victor", "Whisky", "X-Ray", "Yankee",
                           "Zulu"};

    pthread_t tidDatabase;
    pthread_t tidProducer;
    pthread_attr_t attrDatabase;
    pthread_attr_t attrProducer;

    pthread_attr_init(&attrDatabase);
    pthread_attr_init(&attrProducer);
    pthread_create(&tidDatabase, &attrDatabase, database, sharedMemory);
    pthread_create(&tidProducer, &attrProducer, producer, sharedMemory);

    addPeople(sharedMemory, people);

    bool testInProgress = true;
    while(testInProgress) {
        // we use 15 since there will be 15 readers/writers in database
        if(sharedMemory->testCounter >= 15) {
            this_thread::sleep_for(chrono::seconds(3));
            endTesting();
            testInProgress = false;
        }
    }

    pthread_join(tidDatabase, NULL);
    pthread_join(tidProducer, NULL);

    return 0;
}