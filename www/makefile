TARGET = progfippi runstats cgistats.cgi clockprog monitordaq
LIBS = -lm 
CFLAGS = -std=c99 -Wall
CXXFLAGS = -Wall -O3 -DNDEBUG   -pthread -std=c++11
INCDIRS = -I/usr  -I/usr/include -I/usr/local/include
LINKFLAGS =  -static -static-libstdc++
BOOSTLIBS = -L/usr/local/lib -lboost_date_time -lboost_chrono -lboost_atomic -lboost_program_options -lboost_system -lboost_thread -lrt -pthread

.PHONY: default all clean

default: $(TARGET)
all: default

%.o: %.c 
	gcc  $(CFLAGS) -c $< -o $@

%.o: %.cpp 
	g++  $(CXXFLAGS) $(INCDIRS) -c $< -o $@

progfippi: progfippi.o MZTIOCommon.o MZTIOConfig.o MZTIODefs.h
	g++ progfippi.o MZTIOCommon.o MZTIOConfig.o $(LIBS) -o progfippi

runstats: runstats.o MZTIOCommon.o MZTIODefs.h
	gcc runstats.o MZTIOCommon.o $(LIBS) -o runstats

monitordaq: monitordaq.o MZTIOCommon.o MZTIOConfig.o MZTIODefs.h
	g++ monitordaq.o MZTIOCommon.o MZTIOConfig.o $(LIBS) -o monitordaq

cgistats.cgi: cgistats.o MZTIOCommon.o MZTIODefs.h
	gcc cgistats.o MZTIOCommon.o $(LIBS) -o cgistats.cgi

clockprog: clockprog.o MZTIOCommon.o MZTIODefs.h
	gcc clockprog.o MZTIOCommon.o $(LIBS) -o clockprog


clean:
	-rm -f *.o
	-rm -f $(TARGET)
