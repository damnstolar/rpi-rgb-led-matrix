CXX = g++
CXXFLAGS = -O3 -Wall -std=c++14
INCLUDES = -I./include -I/usr/local/include -I/usr/include/GraphicsMagick
LIBS = -L./lib -L/usr/local/lib -lrgbmatrix -lpthread -lMagick++ -lMagickCore

TARGET = matrix-server

SRCS = main.cpp command_queue.cpp handlers.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
