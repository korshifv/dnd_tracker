CXX = g++
CXXFLAGS = -std=c++17 -fPIC $(shell pkg-config --cflags Qt6Widgets)
LDFLAGS = $(shell pkg-config --libs Qt6Widgets)
MOC = /usr/lib/qt6/moc

# ДОБАВИЛИ CharacterSheet.cpp и moc_CharacterSheet.cpp
SRCS = main.cpp MainWindow.cpp TrackerColumn.cpp CharacterCard.cpp CharacterSheet.cpp
MOCS = moc_MainWindow.cpp moc_TrackerColumn.cpp moc_CharacterCard.cpp moc_CharacterSheet.cpp
OBJS = $(SRCS:.cpp=.o) $(MOCS:.cpp=.o)

TARGET = dnd_tracker

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o moc_*.cpp $(TARGET)

run: all
	./$(TARGET)