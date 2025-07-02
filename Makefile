CC=g++ -std=c++20
CFLAGS=-g -c 

TARGET2=dispatcherCore/libdispatch.a
TARGET3=dispatcherCore/dispatcher.exe 

TARGET:${TARGET2} ${TARGET3}

STLIBS=-lpthread

DISPATCHER_OBJS=dispatcherCore/dispatcher_start.o \
				dispatcherCore/dispatchDB.o 	  \
				common/dmsgOp.o 

${TARGET2}:${DISPATCHER_OBJS}
	@echo "Building Dispatcher Library"
	ar rcs ${TARGET2} ${DISPATCHER_OBJS}

${TARGET3}:dispatcherCore/dispatch_main.cpp ${DISPATCHER_OBJS}
	@ECHO "Building Dispatcher Executables"
	${CC} -g dispatcherCore/dispatch_main.o ${DISPATCHER_OBJS} -o ${TARGET3} ${STLIBS}

dispatcherCore/dispatch_main.o:dispatcherCore/dispatch_main.cpp
	${CC} ${CFLAGS} dispatcherCore/dispatch_main.cpp -o dispatcherCore/dispatch_main.o 

dispatcherCore/dispatch_start.o:dispatcherCore/dispatch_start.cpp
	${CC} ${CFLAGS} dispatcherCore/dispatch_start.cpp -o dispatcherCore/dispatch_start.o

dispatcherCore/dispatchDB.o:dispatcherCore/dispatchDB.cpp
	${CC} ${CFLAGS} dispatcherCore/dispatchDB.cpp -o dispatcherCore/dispatchDB.o

common/dmsgOp.o:common/dmsgOp.cpp
	${CC} ${CFLAGS} common/dmsgOp.cpp -o common/dmsgOp.o
clean:
	rm -f *.a
	rm -f *.exe
	rm -f *.o
	rm -f dispatcherCore/*.o
	rm -f dispatcherCore/*.a
	rm -f dispatcherCore/*.exe