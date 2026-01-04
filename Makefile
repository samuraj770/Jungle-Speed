CXX = g++

CXXFLAGS = -std=c++17 -g -Wall -Wextra

# Nazwa pliku wykonywalnego
TARGET = jungle_server

# Lista wszystkich plików źródłowych .cpp
SRCS = src/main.cpp \
       src/Server.cpp \
       src/Player.cpp \
       src/Utils.cpp \
       src/GameRoom.cpp \
       src/GameState.cpp


OBJS = $(SRCS:.cpp=.o)


all: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)