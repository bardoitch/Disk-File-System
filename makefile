all: ex_final.h main.cpp
	g++ main.cpp -o main

all-GDB: ex_final.h main.cpp
	g++ -g main.cpp -o main

	