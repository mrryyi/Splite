

CXX = g++ -lstdc++ -std=c++17
CC = gcc
COMPILE_FLAGS = -c
LIBRARIES = 

# https://gist.github.com/gzmask/329661
LDFLAGS = $(LIBRARIES) -lwsock32  -lglfw3 -lopengl32 -lgdi32 -lglu32 

TARGET = client.exe
cpp_files = main.cpp stb_image.cpp
c_files = glad.c
objects = $(cpp_files:.cpp=.o) $(c_files:.cpp=.o)

all: $(TARGET)

# https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/Makefile
%.o:%.c
	$(CC) -c -o $@ $<

$(TARGET): $(objects)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY : clean
clean :
	-rm $(TARGET) $(objects) *.o *.exe