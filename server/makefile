server: main.o
	g++ -o server main.o -lwsock32 -std=c++0x

main.o: main.cpp
	g++ -c main.cpp

.PHONE: clean
clean:
	rm server *.o