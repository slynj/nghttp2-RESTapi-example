CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

INCLUDES = -I/opt/homebrew/include -I/$(VCPKG_ROOT)/installed/arm64-osx/include
LIBS = -L/opt/homebrew/lib -L/$(VCPKG_ROOT)/installed/arm64-osx/lib -lnghttp2 -lnghttp2_asio -lboost_system -ljsoncpp -lssl -lcrypto -lpthread

TARGET = server
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET) clean_obj

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LIBS) $(INCLUDES)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	
clean_obj:
	rm -f $(OBJS)

.PHONY: all clean