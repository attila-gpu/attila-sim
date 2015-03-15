
ATTILA_SOURCE_DIR=../..

INCLUDE_DIRS = -I $(ATTILA_SOURCE_DIR)/support -I $(ATTILA_SOURCE_DIR)/emul \
               -I $(ATTILA_SOURCE_DIR)/sim

EXTRA_OBJECTS=ShaderInstruction.o ShaderOptimization.o OptimizedDynamicMemory.o support.o

OBJECTS= assembler disassembler optimizer

all: $(OBJECTS)

$(OBJECTS): % : %.cpp $(EXTRA_OBJECTS)
	g++ $@.cpp $(INCLUDE_DIRS) $(EXTRA_OBJECTS) $(LIBRARY_DIRS) $(LIBS) -o $@

ShaderInstruction.o: $(ATTILA_SOURCE_DIR)/emul/ShaderInstruction.cpp $(ATTILA_SOURCE_DIR)/emul/ShaderInstruction.h
	g++ -c $(ATTILA_SOURCE_DIR)/emul/ShaderInstruction.cpp $(INCLUDE_DIRS) -o $@

ShaderOptimization.o: $(ATTILA_SOURCE_DIR)/emul/ShaderOptimization.cpp $(ATTILA_SOURCE_DIR)/emul/ShaderOptimization.h
	g++ -c $(ATTILA_SOURCE_DIR)/emul/ShaderOptimization.cpp $(INCLUDE_DIRS) -o $@

OptimizedDynamicMemory.o: $(ATTILA_SOURCE_DIR)/support/OptimizedDynamicMemory.cpp $(ATTILA_SOURCE_DIR)/support/OptimizedDynamicMemory.h
	g++ -c $(ATTILA_SOURCE_DIR)/support/OptimizedDynamicMemory.cpp $(INCLUDE_DIRS) -o $@

support.o: $(ATTILA_SOURCE_DIR)/support/support.cpp $(ATTILA_SOURCE_DIR)/support/support.h
	g++ -c $(ATTILA_SOURCE_DIR)/support/support.cpp $(INCLUDE_DIRS) -o $@
