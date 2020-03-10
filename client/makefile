client: main.o
	g++ -o client main.o -lwsock32 -lncurses 

main.o: main.cpp
	g++ -c main.cpp

.PHONE: clean
clean:
	rm client *.o