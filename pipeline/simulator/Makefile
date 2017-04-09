single_cycle: simulator.o instruction.o
	g++ -g -o pipeline simulator.o instruction.o
simulator.o: simulator.cpp
	g++ -g -c simulator.cpp
instruction.o: instruction.cpp instruction.h
	g++ -g -c instruction.cpp
clean:
	rm -f pipeline simulator.o instruction.o
