
ATTILA_SOURCE_DIR=../..

INCLUDE_DIRS = -I $(ATTILA_SOURCE_DIR)/support -I $(ATTILA_SOURCE_DIR)/sim

EXTRA_OBJECTS=support.o QuadFloat.o

OBJECTS= dumpRegisters

all: $(OBJECTS)

$(OBJECTS): % : %.cpp $(EXTRA_OBJECTS)
	g++ $@.cpp $(INCLUDE_DIRS) $(EXTRA_OBJECTS) $(LIBRARY_DIRS) $(LIBS) -o $@

support.o: $(ATTILA_SOURCE_DIR)/support/support.cpp $(ATTILA_SOURCE_DIR)/support/support.h
	g++ -c $(ATTILA_SOURCE_DIR)/support/support.cpp $(INCLUDE_DIRS) -o $@

QuadFloat.o: $(ATTILA_SOURCE_DIR)/support/QuadFloat.cpp $(ATTILA_SOURCE_DIR)/support/QuadFloat.h
	g++ -c $(ATTILA_SOURCE_DIR)/support/QuadFloat.cpp $(INCLUDE_DIRS) -o $@

