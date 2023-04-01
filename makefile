CC = gcc
LIBS = -lm
FLAGS = -Wall -pthread -g
PROGS = system_manager user_console sensor_process
OBJS = system_manager.o functions.o alerts_watcher.o worker.o user_console.o sensor_process.o

all: ${PROGS}

clean:
	rm ${OBJS} *~ ${PROGS}

system_manager: system_manager.o functions.o alerts_watcher.o worker.o
	${CC} ${FLAGS} $^ ${LIBS} -o $@

user_console: user_console.o functions.o
	${CC} ${FLAGS} $^ ${LIBS} -o $@

sensor_process: sensor_process.o functions.o
	${CC} ${FLAGS} $^ ${LIBS} -o $@

%.o: %.c
	${CC} ${FLAGS} -c $< -o $@

##########################

system_manager.o: system_manager.c system_manager.h shared_mem.h functions.h worker.h alerts_watcher.h

alerts_watcher.o: alerts_watcher.c alerts_watcher.h shared_mem.h functions.h

user_console.o: user_console.c user_console.h shared_mem.h functions.h

worker.o: worker.c worker.h shared_mem.h functions.h

sensor_process.o: sensor_process.c sensor_process.h shared_mem.h functions.h

functions.o: functions.c functions.h shared_mem.h