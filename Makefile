# Makefile

CXX      = g++
CXXFLAGS = -Wall -std=c++20 -ggdb

default: demo

demo: main.o ast.o eval.o optimizer.o
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f demo

.PHONY: clean
