server: main.o message.o
	g++ -o server main.o message.o -lwsock32

main.o: main.cpp ../include/message.h
	g++ -c main.cpp 

message.o: ../include/message.cpp
	g++ -c ../include/message.cpp

.PHONE: clean
clean:
	rm server *.o
