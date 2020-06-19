TARGET = progfippi.cgi runstats status.cgi register.cgi clockprog.cgi monitordaq cgireadoscilloscope.cgi cgiwriteoscilloscope.cgi cgireadregister.cgi cgiwriteregister.cgi cgireadtimediffch.cgi cgiwritetimediffch.cgi cgicleartimediff.cgi cgiupdatetimediff.cgi shutdown.cgi updatefirmware.cgi 
LIBS = -lm 
CFLAGS = -std=c99 -Wall
CXXFLAGS = -Wall -O3 -DNDEBUG   -pthread -std=c++11
INCDIRS = -I/usr  -I/usr/include -I/usr/local/include
LINKFLAGS =  -static -static-libstdc++
BOOSTLIBS = -L/usr/local/lib -lboost_date_time -lboost_chrono -lboost_atomic -lboost_program_options -lboost_system -lboost_thread -lrt -pthread

.PHONY: default all clean

default: $(TARGET) cgisudo
all: default

cgisudo: shutdown.cgi updatefirmware.cgi
	$(shell sh cgisudo.sh)

%.o: %.c 
	gcc  $(CFLAGS) -c $< -o $@

%.o: %.cc 
	g++  $(CXXFLAGS) $(INCDIRS) -c $< -o $@

%.o: %.cpp 
	g++  $(CXXFLAGS) $(INCDIRS) -c $< -o $@

shutdown.cgi: shutdown.o
	g++ shutdown.o $(LIBS) -o shutdown.cgi

updatefirmware.cgi: updatefirmware.o
	g++ updatefirmware.o $(LIBS) -o updatefirmware.cgi

progfippi.cgi: progfippi.o MZTIOCommon.o  MZTIODefs.h
	g++ progfippi.o MZTIOCommon.o  $(LIBS) -o progfippi.cgi

status.cgi: status.o MZTIOCommon.o MZTIODefs.h
	gcc status.o MZTIOCommon.o $(LIBS) -o status.cgi

register.cgi: register.o MZTIOCommon.o MZTIODefs.h
	g++ register.o MZTIOCommon.o $(LIBS) -o register.cgi

cgiwriteoscilloscope.cgi: cgiwriteoscilloscope.o
	g++ cgiwriteoscilloscope.o  $(LIBS) -o cgiwriteoscilloscope.cgi

cgireadoscilloscope.cgi: cgireadoscilloscope.o
	g++ cgireadoscilloscope.o $(LIBS) -o cgireadoscilloscope.cgi

cgireadregister.cgi: cgireadregister.o
	g++ cgireadregister.o $(LIBS) -o cgireadregister.cgi

cgiwriteregister.cgi: cgiwriteregister.o
	g++ cgiwriteregister.o $(LIBS) -o cgiwriteregister.cgi

runstats: runstats.o MZTIOCommon.o MZTIODefs.h
	gcc runstats.o MZTIOCommon.o $(LIBS) -o runstats

monitordaq: monitordaq.o MZTIOCommon.o MZTIODefs.h
	g++ monitordaq.o MZTIOCommon.o $(LIBS) -o monitordaq

clockprog.cgi: clockprog.o MZTIOCommon.o MZTIODefs.h
	gcc clockprog.o MZTIOCommon.o $(LIBS) -o clockprog.cgi

cgireadtimediffch.cgi: cgireadtimediffch.o
	g++ cgireadtimediffch.o $(LIBS) -o cgireadtimediffch.cgi

cgiwritetimediffch.cgi: cgiwritetimediffch.o
	g++ cgiwritetimediffch.o $(LIBS) -o cgiwritetimediffch.cgi

cgicleartimediff.cgi: cgicleartimediff.o
	g++ cgicleartimediff.o $(LIBS) -o cgicleartimediff.cgi

cgiupdatetimediff.cgi: cgiupdatetimediff.o
	g++ cgiupdatetimediff.o $(LIBS) -o cgiupdatetimediff.cgi




clean:
	-rm -f *.o
	-rm -f $(TARGET)
