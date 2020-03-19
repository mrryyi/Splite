CXX = g++
COMPILE_FLAGS = -c
LIBRARIES =

# https://gist.github.com/gzmask/329661
LDFLAGS = $(LIBRARIES) -lwsock32  -lglfw3 -lglfw3dll -lopengl32 -lgdi32 -lglu32

TARGET = client.exe
cpp_files = main.cpp
objects = $(cpp_files:.cpp=.o)

all: $(TARGET)

$(TARGET): $(objects)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY : clean
clean :
	-rm $(TARGET) $(objects) *.o *.exe