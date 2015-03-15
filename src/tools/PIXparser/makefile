
ATTILA_SOURCE_DIR=../../

INCLUDE_DIRS = -I $(ATTILA_SOURCE_DIR)/support

EXTRA_OBJECTS=

OBJECTS= PIXParser

all: $(OBJECTS)

$(OBJECTS): % : %.cpp $(EXTRA_OBJECTS)
	g++ $@.cpp $(INCLUDE_DIRS) $(EXTRA_OBJECTS) $(LIBRARY_DIRS) $(LIBS) -o $@

#support.o: $(ATTILA_SOURCE_DIR)/support/support.cpp $(ATTILA_SOURCE_DIR)/support/support.h
#	g++ -c $(ATTILA_SOURCE_DIR)/support/support.cpp $(INCLUDE_DIRS) -o $@
