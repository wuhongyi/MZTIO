TARGET = progfippi runstats cgistats.cgi clockprog monitordaq
LIBS = -lm 
CFLAGS = -std=c99 -Wall
CXXFLAGS = -Wall -O3 -DNDEBUG   -pthread -std=gnu++98
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
	

progfippi: progfippi.o PixieNetCommon.o PixieNetConfig.o PixieNetDefs.h
	g++ progfippi.o PixieNetCommon.o PixieNetConfig.o $(LIBS) -o progfippi

runstats: runstats.o PixieNetCommon.o PixieNetDefs.h
	gcc runstats.o PixieNetCommon.o $(LIBS) -o runstats

monitordaq: monitordaq.o PixieNetCommon.o PixieNetConfig.o PixieNetDefs.h
	g++ monitordaq.o PixieNetCommon.o PixieNetConfig.o $(LIBS) -o monitordaq

cgistats.cgi: cgistats.o PixieNetCommon.o PixieNetDefs.h
	gcc cgistats.o PixieNetCommon.o $(LIBS) -o cgistats.cgi

clockprog: clockprog.o PixieNetCommon.o PixieNetDefs.h
	gcc clockprog.o PixieNetCommon.o $(LIBS) -o clockprog


clean:
	-rm -f *.o
	-rm -f $(TARGET)
