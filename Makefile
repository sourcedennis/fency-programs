.PHONY: clean all

CXXFLAGS=-std=c++0x -pthread -Wall

SRCS := $(wildcard *.cc)
BINS := $(SRCS:%.cc=bin/%)

all: ${BINS}

bin/%: %.cc
	@mkdir -p $(@D)
	$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f ${BINS}
