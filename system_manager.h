#ifndef system_manager
#define system_manager  /* Include guard */

//User libraries
#include "alerts_watcher.h"
#include "shared_mem.h"
#include "functions.h"
#include "worker.h"

void create_named_pipe(char *name);
void *dispatcher_reader(void *arg);
void *console_reader(void *arg);
void *sensor_reader(void *arg);
void init_program();
void create_msq();
void init_log();

#endif 