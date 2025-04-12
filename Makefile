CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude

SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)
BIN = tetris

all: $(BIN)

$(BIN): main.cpp $(SRC)
	$(CXX) $(CXXFLAGS) -o $(BIN) main.cpp $(SRC)

clean:
	rm -f $(BIN) src/*.o
