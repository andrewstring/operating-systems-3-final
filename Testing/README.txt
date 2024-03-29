To execute, run either:
    g++ -std=c++11 -o {file}.out {file}.cpp
    or
    clang++ -std=c++11 -o {file}.out {file}.cpp if g++ does not work

NOTE:
    In the main function, there is a loop that checks for completion
    of program via the testCounter sharedMemory variable. If you add
    elements that the producer delivers to the consumer, the testCounter
    will need to be changed to reflect the additional elements.
    With this implementation, if the producer produces an element that
    cannot be admitted at the moment, that element will be forgotten
    about (for example, in 3, if the hallway is full and a student
    tries to enter, that student will be forgotten about. If you were
    to add an extra student when the hallway is full, the testCounter
    would not be incremented by 1, since that student will not enter
    the hallway and be operated on).

Changing tests for 1:
    In producer function, change the enterBarberShop calls, accessing different
    indices of the people array to add different people. You can also add/change
    the thread sleep times to wait between adding people

Changing tests for 2:
    In struct SharedMemory, change the value of numOfRounds

Changing tests for 3:
    In producer function, change the enterHallway calls, accessing different indices
    of the people array to add different people. You can also add/change
    the thread sleep times to wait between adding people

Changing tests for 4:
    In the addPeople function, change the addReader/addWriter calls, accessing different
    indices of the students array. You can also add/change the thread sleep
    times to wait between adding people