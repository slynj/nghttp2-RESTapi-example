CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

INCLUDES = -I/opt/homebrew/include
LIBS = -L/opt/homebrew/lib -lnghttp2 -ljsoncpp -lssl -lcrypto -lpthread

TARGET = server
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDES) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean