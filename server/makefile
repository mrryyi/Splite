server: main.o
	g++ -o server main.o -lwsock32

main.o: main.cpp
	g++ -c main.cpp

.PHONE: clean
clean:
	rm server *.o