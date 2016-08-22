all: main.o Circuit.o
	g++ -o RADAR main.o Circuit.o
main.o: main.cpp Circuit.h TimeInfo.h clauses.h
	g++ -c main.cpp
circuit.o: Circuit.cpp Circuit.h TimeInfo.h clauses.h
	g++ -c Circuit.cpp
clean:
	rm *.o
