CXX=$(shell sh -c "which g++-6 || which g++")
CXXFLAGS=-std=c++1z -O3 -g3 -ggdb -Wall 
INCLUDES = -Isrc/include
CXXFLAGS += $(INCLUDES)
LDFLAGS =
LDLIBS = -lrt -lpthread
SRC_DIR = src
OUT_DIR = bin
SOURCES := $(shell find $(SRC_DIR) -name *.cpp)
DEPENDS = $(SOURCES:.cpp=.d)
TARGETS = client server inspect

SOURCES := $(filter-out $(patsubst %,examples/%.cpp,$(TARGETS)),$(SOURCES))
OBJECTS = $(SOURCES:.cpp=.o)

$(info $(OBJECTS))

.PHONY: clean directories

all:
	mkdir -p $(OUT_DIR)
	$(MAKE) $(addprefix $(OUT_DIR)/,$(TARGETS))

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

bin/%: example/%.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	find . -type f -name '*.o' -delete
	find . -type f -name '*.d' -delete
	rm -rf ./bin