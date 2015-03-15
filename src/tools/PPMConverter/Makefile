CXFLAGS = -g -Wall
OBJS_SHARED = PPMFile.o PPMInspector.o
OBJS_CONV  = main.o FragmentMapConverter.o LinearRemap.o GroupRemap.o $(OBJS_SHARED)
OBJS_INSP = PPMInspect.o $(OBJS_SHARED)
CX = g++
LD = g++
LDFLAGS = 
PROGS = ppmconv ppminsp

all: $(PROGS)

ppmconv: $(OBJS_CONV)
	$(LD) $(OBJS_CONV) -o ppmconv
ppminsp: $(OBJS_INSP)
	$(LD) $(OBJS_INSP) -o ppminsp

PPMInspect.o: PPMInspect.cpp PPMFile.h PPMInspector.h
	$(CX) $(CXFLAGS) -c PPMInspect.cpp
main.o: main.cpp FragmentMapConverter.h PPMFile.h PPMInspector.h
	$(CX) $(CXFLAGS) -c main.cpp
PPMFile.o: PPMFile.cpp PPMFile.h
	$(CX) $(CXFLAGS) -c PPMFile.cpp
PPMInspector.o: PPMInspector.h PPMInspector.cpp PPMFile.h
	$(CX) $(CXFLAGS) -c PPMInspector.cpp
FragmentMapConverter.o: FragmentMapConverter.h FragmentMapConverter.cpp PPMFile.h
	$(CX) $(CXFLAGS) -c FragmentMapConverter.cpp
LinearRemap.o: LinearRemap.h LinearRemap.cpp FragmentMapConverter.h PPMInspector.h
	$(CX) $(CXFLAGS) -c LinearRemap.cpp
GroupRemap.o: GroupRemap.h GroupRemap.cpp FragmentMapConverter.h PPMInspector.h
	$(CX) $(CXFLAGS) -c GroupRemap.cpp
clean:
	rm *.o
