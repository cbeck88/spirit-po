CXXFLAGS := -std=c++11
INCLUDES := -Iinclude/ -I/usr/include

main: src/main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f main

