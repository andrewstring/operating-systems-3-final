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
    queue<tuple<Type, string *>> readers;
    queue<tuple<Type, string *>> writers;
    queue<tuple<Type, string *>> peopleInDatabase;
    int numOfReaders = 0;
    int numOfWriters = 0;
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
        sharedMemory->numOfReaders++;
        sharedMemory->readers.push(newReader);
        assertInt(
            sharedMemory->numOfReaders,
            sharedMemory->readers.size(),
            "Number of readers and size of readers queue are equal when reader is added",
            "Number of readers and size of readers queue are not equal when reader is added"
        );
        //cout << *name + " (Type: " + typeOutput[get<0>(newReader)] + ") has been added\n";
        //cout.flush();
}

tuple<Type, string*> removeReader(SharedMemory *sharedMemory) {
    tuple<Type, string*> retrievedReader = sharedMemory->readers.front();
    sharedMemory->numOfReaders--;
    sharedMemory->readers.pop();

    assertInt(
        sharedMemory->numOfReaders,
        sharedMemory->readers.size(),
        "Number of readers and size of readers queue are equal when reader is removed",
        "Number of readers and size of readers queue are not equal when reader is removed"
    );

    return retrievedReader;
}

void addWriter(SharedMemory *sharedMemory, string *name) {
    tuple<Type, string*> newWriter = make_tuple(writer, name);
    sharedMemory->numOfWriters++;
    sharedMemory->writers.push(newWriter);
    assertInt(
        sharedMemory->numOfWriters,
        sharedMemory->writers.size(),
        "Number of writers and size of writers queue are equal when writer is added",
        "Number of writers and size of writers queue are not equal when writer is added"
    );
    //cout << *name + " (Type: " + typeOutput[get<0>(newWriter)] + ") has been added\n";
    //cout.flush();
}

tuple<Type, string*> removeWriter(SharedMemory *sharedMemory) {
    tuple<Type, string*> retrievedWriter = sharedMemory->writers.front();
    sharedMemory->numOfWriters--;
    sharedMemory->writers.pop();
    assertInt(
        sharedMemory->numOfWriters,
        sharedMemory->writers.size(),
        "Number of writers and size of writers queue are equal when writer is removed",
        "Nubmer of writers and size of writers queue are note equal when writer is removed"
    );
    return retrievedWriter;

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
            //cout << *(get<1>(person)) + " (Type: " + typeOutput[get<0>(person)] + ") HAS ENTERED THE DATABASE\n";
            //cout.flush();
            sharedMemory->numOfPeopleInDatabase++;
            sharedMemory->peopleInDatabase.push(person);
            assertInt(
                sharedMemory->numOfPeopleInDatabase,
                sharedMemory->peopleInDatabase.size(),
                "Number of people in database and people in database queue are equal when someone enters database",
                "Number of people in database and people in database queue are not equal when someone enters database"
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
            //cout << *(get<1>(person)) + "(Type: " + typeOutput[get<0>(person)] + ") HAS ENTERED THE DATABASE\n";
            //cout.flush();
            sharedMemory->numOfPeopleInDatabase++;
            sharedMemory->peopleInDatabase.push(person);
            assertInt(
                sharedMemory->numOfPeopleInDatabase,
                sharedMemory->peopleInDatabase.size(),
                "Number of people in database and people in database queue are equal when someone enters database",
                "Number of people in database and people in database queue are not equal when someone enters database"
            );
        }
    }
}

void leaveDatabase(SharedMemory *sharedMemory) {
        tuple<Type, string*> personLeaving = sharedMemory->peopleInDatabase.front();
        if(get<0>(personLeaving) == reader) {
            sharedMemory->peopleInDatabase.pop();
            sharedMemory->numOfReadersInDatabase--;
            //cout << *(get<1>(personLeaving)) + " (Type: " + typeOutput[get<0>(personLeaving)] + ") HAS LEFT THE DATABASE\n";
            //cout.flush();
            sharedMemory->numOfPeopleInDatabase--;
            assertInt(
                sharedMemory->numOfPeopleInDatabase,
                sharedMemory->peopleInDatabase.size(),
                "Number of people in database and size of people in database queue are equal when person is removed from database",
                "Number of people in database and size of people in database queue are not equal when person is removed from database"
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
            //cout << *(get<1>(personLeaving)) + " (Type: " + typeOutput[get<0>(personLeaving)] + ") HAS LEFT THE DATABASE\n";
            //cout.flush();
            sharedMemory->numOfPeopleInDatabase--;
            assertInt(
                sharedMemory->numOfPeopleInDatabase,
                sharedMemory->peopleInDatabase.size(),
                "Number of people in database and size of people in database queue are equal when person is removed from database",
                "Number of people in database and size of people in database queue are not equal when person is removed from database"
            );
            release(sharedMemory, writerMut);
                assertInt(
                    sharedMemory->readerMutex,
                    1,
                    "Reader mutex was successfully released",
                    "Reader mutex was not successfully released"
                );
        }
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
            if(memory->writerMutex == 1 && memory->numOfReaders > 0) {
                acquire(memory, criticalSection);
                tuple<Type, string*> readerEnteringDatabase = removeReader(memory);
                enterDatabase(memory, readerEnteringDatabase);
                release(memory, criticalSection);
            }
            else if(memory->readerMutex == 1 && memory->writerMutex == 1 && memory->numOfWriters > 0) {
                acquire(memory, criticalSection);
                tuple<Type, string*> writerEnteringDatabase = removeWriter(memory);
                enterDatabase(memory, writerEnteringDatabase);
                release(memory, criticalSection);
            }
        }

    }
}


void* readerProducer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    while(true) {
            if (memory->writerMutex == 1 && memory->numOfReaders > 0) {
                tuple<Type, string*> readerEnteringDatabase = removeReader(memory);
                enterDatabase(memory, readerEnteringDatabase);
            }
    }

    return NULL;
}

void* writerProducer(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    while(true) {
            if(memory->readerMutex == 1 && memory->writerMutex == 1 && memory->numOfWriters > 0) {
                tuple<Type, string*> writerEnteringDatabase = removeWriter(memory);
                enterDatabase(memory, writerEnteringDatabase);

            }
    }

    return NULL;
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
    // pthread_t tidReader;
    // pthread_t tidWriter;
    pthread_attr_t attrDatabase;
    pthread_attr_t attrProducer;
    //pthread_attr_t attrReader;
    //pthread_attr_t attrWriter;

    pthread_attr_init(&attrDatabase);
    pthread_attr_init(&attrProducer);
    //pthread_attr_init(&attrReader);
    //pthread_attr_init(&attrWriter);
    pthread_create(&tidDatabase, &attrDatabase, database, sharedMemory);
    pthread_create(&tidProducer, &attrProducer, producer, sharedMemory);
    //pthread_create(&tidReader, &attrReader, readerProducer, sharedMemory);
    //pthread_create(&tidWriter, &attrWriter, writerProducer, sharedMemory);

    addPeople(sharedMemory, people);

    pthread_join(tidDatabase, NULL);
    pthread_join(tidProducer, NULL);
    //pthread_join(tidReader, NULL);
    //pthread_join(tidWriter, NULL);

    return 0;
}