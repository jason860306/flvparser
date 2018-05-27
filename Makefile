CXX=g++
CFLAGS=-W -Wall -O2

TARGET=flvparser
all: $(TARGET)

flvparser: flvparser.cpp
	$(CXX) $(CFLAGS) $^ -o $@

.PHONY: all clean
clean:
	@rm -f $(TARGET) *.o *~ *flymake*
