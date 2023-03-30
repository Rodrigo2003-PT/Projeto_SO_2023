CC = gcc
PROG = prog
FLAGS = -Wall -pthread -g
OBJS = system_manager.o functions.o alerts_watcher.o worker.o

all: ${PROG}

clean:
	rm ${OBJS} *~ ${PROG}

${PROG}: ${OBJS}
	${CC} ${FLAGS} ${OBJS} -lm -o $@

.c.o:
	${CC} ${FLAGS} $< -c -o $@

##########################

system_manager.o: system_manager.c system_manager.h shared_mem.h functions.h

alerts_watcher.o: alerts_watcher.c alerts_watcher.h shared_mem.h functions.h

functions.o: functions.c functions.h shared_mem.h

worker.o: worker.c worker.h shared_mem.h functions.h