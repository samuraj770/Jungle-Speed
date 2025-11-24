# Kompilator, którego chcemy użyć
CXX = g++

# Flagi kompilatora:
# -std=c++17 : Używaj standardu C++17 (ważne dla std::shared_ptr itp.)
# -g         : Dołącz informacje debugowania (dla debuggera gdb)
# -Wall      : Włącz wszystkie standardowe ostrzeżenia
# -Wextra    : Włącz dodatkowe ostrzeżenia (dobra praktyka)
CXXFLAGS = -std=c++17 -g -Wall -Wextra

# Nazwa pliku wykonywalnego, który chcemy stworzyć
TARGET = jungle_server

# Lista wszystkich plików źródłowych .cpp
# Plik Protocol.h nie jest tu potrzebny, bo to tylko nagłówek
SRCS = src/main.cpp \
       src/Server.cpp \
       src/Player.cpp \
       src/Utils.cpp \
       src/GameRoom.cpp \
       #src/GameState.cpp
       # Dodaj tu inne swoje pliki .cpp, jeśli je masz

# Automatycznie zamień listę .cpp na listę .o (pliki obiektowe)
OBJS = $(SRCS:.cpp=.o)

# --- Reguły Budowania ---

# Reguła domyślna ('make' lub 'make all'):
# Zależy od naszego pliku wykonywalnego
all: $(TARGET)

# Reguła linkowania (tworzenia finalnego pliku):
# Aby stworzyć $(TARGET), potrzebujemy wszystkich plików $(OBJS)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Reguła kompilacji (tworzenia plików .o):
# Mówi, jak zrobić plik .o z pliku .cpp
# $< to plik źródłowy (.cpp), $@ to plik docelowy (.o)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Reguła "clean":
# Służy do sprzątania skompilowanych plików
clean:
	rm -f $(TARGET) $(OBJS)