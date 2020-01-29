output: main.o
	g++ -o output main.o -lwsock32

main.o: main.cpp
	g++ -c main.cpp

.PHONE: clean
clean:
	rm output *.o