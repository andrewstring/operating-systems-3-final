To execute, run either:
    g++ -std=c++11 -o {file}.out {file}.cpp
    or
    clang++ -std=c++11 -o {file}.out {file}.cpp if g++ does not work

In order to change producer additions:
    For 1:
        For setting number of chairs, in main function, change second argument value for
        setNumChairs
        For changing admissions into barber shop, add/remove enterBarberShop calls in
        producer function, with different indices of people array for different people names

    For 2:
        For setting number of rounds, in main function, change seconds argument value for
        setNumRounds

    For 3:
        For setting number of chairs, in main function, change second argument value for
        setNumHallwayChairs
        For changing admissions into hallway, add/remove enterHallway calls in producer function,
        with different indices of students array for different student names

    For 4:
        For changing admissions into database, add/remove addReader/addWriter calls in addPeople
        function, with different indices of students array for different student names

    For 5:
        For changing north and south travelers, add/remove addNorthTraveler/addSouthTraveler in
        northProducer/southProducer functions, with different indices of traveler array for different
        traveler names

    For 6:
        