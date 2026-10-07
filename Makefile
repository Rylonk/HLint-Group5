# Build:  make          Run tests:  make test          Clean:  make clean
# No make?  Compile by hand:   g++ -std=c++17 -O2 -o HLInt src/*.cpp
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
TARGET   := HLInt
SOURCES  := $(wildcard src/*.cpp)

$(TARGET): $(SOURCES) $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES)

test: $(TARGET)
	bash tests/run_tests.sh

clean:
	rm -f $(TARGET) $(TARGET).exe NOSPACES.TXT RES_SYM.TXT

.PHONY: test clean
