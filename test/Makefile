BOOST_DIR ?= /usr/include

CXXFLAGS := $(CXXFLAGS) -std=c++11 
INCLUDES := -I../include/ -isystem$(BOOST_DIR)

main: main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f main

check:
	./main
