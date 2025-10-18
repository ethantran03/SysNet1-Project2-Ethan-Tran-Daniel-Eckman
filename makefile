# Build mt-collatz with debugging symbols and warnings enabled
# Usage:
#   make            # build
#   ./mt-collatz N T [-nolock]
#   make clean      # remove binary

CXX      := g++
CXXFLAGS := -Wall -g -O2 -pthread
LDFLAGS  := -pthread

all: mt-collatz

mt-collatz: mt-collatz.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f mt-collatz
