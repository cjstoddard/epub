CXX ?= g++
CXXFLAGS := -O2 -std=c++17 -Wall -Wextra -Wpedantic
LIBS := -lzip -ltinyxml2 -lncursesw

SRC := src/main.cpp src/epub.cpp src/html_text.cpp src/ui.cpp
HDR := src/epub.h src/html_text.h src/ui.h
OBJ := $(SRC:.cpp=.o)

epub: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LIBS)

%.o: %.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) saraswati

