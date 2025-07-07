CC=g++ -std=c++20
CFLAGS=-g -c 

TARGET1=clientLib/libclient.a
TARGET2=dispatcherCore/libdispatch.a
TARGET3=dispatcherCore/dispatcher.exe
TARGET4=examples/pub_skt_main.exe

TARGET:${TARGET1} ${TARGET2} ${TARGET3} ${TARGET4}

STLIBS=-lpthread

DISPATCHER_OBJS=dispatcherCore/dispatcher_start.o \
				dispatcherCore/dispatchDB.o 	  \
				dispatcherCore/dispatch_msg.o     \
				common/dmsgOp.o 				  

CLIIENTLIBS=-LclientLib -lclient

${TARGET1}:clientLib/client.o common/dmsgOp.o
	@echo "Building client library"
	ar rcs ${TARGET1} clientLib/client.o common/dmsgOp.o

${TARGET2}:${DISPATCHER_OBJS}
	@echo "Building Dispatcher Library"
	ar rcs ${TARGET2} ${DISPATCHER_OBJS}

${TARGET3}:dispatcherCore/dispatch_main.o ${DISPATCHER_OBJS}
	@ECHO "Building Dispatcher Executables"
	${CC} -g dispatcherCore/dispatch_main.o ${DISPATCHER_OBJS} -o ${TARGET3} ${STLIBS}

${TARGET4}:examples/pub_skt_main.o examples/pub_skt_example.o ${TARGET1}
	@echo "Building publisher socket executable"
	${CC} -g examples/pub_skt_main.o examples/pub_skt_example.o -o ${TARGET4} ${CLIIENTLIBS}

######### dispatcherCore directory #########
dispatcherCore/dispatch_main.o:dispatcherCore/dispatch_main.cpp
	${CC} ${CFLAGS} dispatcherCore/dispatch_main.cpp -o dispatcherCore/dispatch_main.o 

dispatcherCore/dispatch_start.o:dispatcherCore/dispatch_start.cpp
	${CC} ${CFLAGS} dispatcherCore/dispatch_start.cpp -o dispatcherCore/dispatch_start.o

dispatcherCore/dispatchDB.o:dispatcherCore/dispatchDB.cpp
	${CC} ${CFLAGS} dispatcherCore/dispatchDB.cpp -o dispatcherCore/dispatchDB.o

dispatcherCore/dispatch_msg.o:dispatcherCore/dispatch_msg.cpp
	${CC} ${CFLAGS} dispatcherCore/dispatch_msg.cpp -o dispatcherCore/dispatch_msg.o

######### common directory #########
common/dmsgOp.o:common/dmsgOp.cpp
	${CC} ${CFLAGS} common/dmsgOp.cpp -o common/dmsgOp.o

clean:
	rm -f *.a
	rm -f *.exe
	rm -f *.o
	rm -f dispatcherCore/*.o
	rm -f dispatcherCore/*.a
	rm -f dispatcherCore/*.exe
	rm -f common/*.o
	rm -f common/*.a
	rm -f common/*.exe
	rm -rf clientLib/*.o
	rm -rf clientLib/*.a
	rm -rf clientLib/*.exe
	rm -f examples/*.o 
	rm -f examples/*.exe