#ifndef system_manager
#define system_manager  /* Include guard */

//User libraries
#include "alerts_watcher.h"
#include "shared_mem.h"
#include "functions.h"
#include "worker.h"

void init_program();
void init_log();
void create_named_pipe(char *name);
void *sensor_reader(void *arg);
void *console_reader(void *arg);
void create_msq();

#endif 