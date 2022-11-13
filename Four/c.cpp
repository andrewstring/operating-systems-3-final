#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <string>
#include <tuple>

using namespace std;

// access points for acquire and release
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

        // add new reader to the reader queue
        sharedMemory->people.push(newReader);
        cout << *name + " (Type: " + typeOutput[get<0>(newReader)] + ") has been added\n";
        cout.flush();
}

void addWriter(SharedMemory *sharedMemory, string *name) {
    tuple<Type, string*> newWriter = make_tuple(writer, name);
    sharedMemory->numOfPeople++;

    // add new writer to the writer queue
    sharedMemory->people.push(newWriter);
    cout << *name + " (Type: " + typeOutput[get<0>(newWriter)] + ") has been added\n";
    cout.flush();
}

// returns whether the person at the front of the queue is a reader or a writer
Type getTypeAtFront(SharedMemory *sharedMemory) {
    return get<0>(sharedMemory->people.front());
}


tuple<Type, string*> removePerson(SharedMemory *sharedMemory) {
    tuple<Type, string*> retrievedPerson = sharedMemory->people.front();
    sharedMemory->numOfPeople--;

    // remove reader/writer from the people queue
    sharedMemory->people.pop();
    return retrievedPerson;
}

void enterDatabase(SharedMemory *sharedMemory, tuple<Type, string*> person) {
    if (get<0>(person) == reader) {
        if(sharedMemory->writerMutex == 1) {
            acquire(sharedMemory, readerMut);
            sharedMemory->numOfReadersInDatabase++;
            cout << *(get<1>(person)) + " (Type: " + typeOutput[get<0>(person)] + ") HAS ENTERED THE DATABASE\n";
            cout.flush();
            sharedMemory->numOfPeopleInDatabase++;

            // add reader if there is no writer in the database
            sharedMemory->peopleInDatabase.push(person);
        }
    }

    if (get<0>(person) == writer) {
        if(sharedMemory->readerMutex == 1 && sharedMemory->writerMutex == 1) {
            acquire(sharedMemory, writerMut);
            sharedMemory->numOfWritersInDatabase++;
            cout << *(get<1>(person)) + "(Type: " + typeOutput[get<0>(person)] + ") HAS ENTERED THE DATABASE\n";
            cout.flush();
            sharedMemory->numOfPeopleInDatabase++;

            // add writer if there are no readers or writers in the database
            sharedMemory->peopleInDatabase.push(person);
        }
    }
}

void leaveDatabase(SharedMemory *sharedMemory) {
        tuple<Type, string*> personLeaving = sharedMemory->peopleInDatabase.front();
        if(get<0>(personLeaving) == reader) {

            // remove reader from the database
            sharedMemory->peopleInDatabase.pop();
            sharedMemory->numOfReadersInDatabase--;
            cout << *(get<1>(personLeaving)) + " (Type: " + typeOutput[get<0>(personLeaving)] + ") HAS LEFT THE DATABASE\n";
            cout.flush();
            sharedMemory->numOfPeopleInDatabase--;
            if (sharedMemory->numOfReadersInDatabase == 0) {
                release(sharedMemory, readerMut);
            }
        }

        if(get<0>(personLeaving) == writer) {

            // remove writer from the database
            sharedMemory->peopleInDatabase.pop();
            sharedMemory->numOfWritersInDatabase--;
            cout << *(get<1>(personLeaving)) + " (Type: " + typeOutput[get<0>(personLeaving)] + ") HAS LEFT THE DATABASE\n";
            cout.flush();
            sharedMemory->numOfPeopleInDatabase--;
            release(sharedMemory, writerMut);
        }
}

void* database(void *sharedMemory) {
    SharedMemory *memory = (struct SharedMemory *) sharedMemory;

    while(true) {
        // check for readers/writers in the database every two seconds
        this_thread::sleep_for(chrono::seconds(2));

        // if there is someone in the database, pop them out
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

                        // reader can enter the database if there is no writer in the database
                        enterDatabase(memory, readerEnteringDatabase);
                        release(memory, criticalSection);
                    }
                }
                else if(nextUpType == writer) {
                    if(memory->readerMutex == 1 && memory->writerMutex == 1) {
                        acquire(memory, criticalSection);
                        tuple<Type, string*> writerEnteringDatabase = removePerson(memory);

                        // writer can enter the database if there are no readers or writers in the database
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


    // wait before entering more people
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



    // two threads, consumer=database, producer=producer
    pthread_t tidDatabase;
    pthread_t tidProducer;
    pthread_attr_t attrDatabase;
    pthread_attr_t attrProducer;

    pthread_attr_init(&attrDatabase);
    pthread_attr_init(&attrProducer);
    pthread_create(&tidDatabase, &attrDatabase, database, sharedMemory);
    pthread_create(&tidProducer, &attrProducer, producer, sharedMemory);

    addPeople(sharedMemory, people);

    pthread_join(tidDatabase, NULL);
    pthread_join(tidProducer, NULL);

    return 0;
}