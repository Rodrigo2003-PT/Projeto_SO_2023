#ifndef system_manager
#define system_manager  /* Include guard */

//User libraries
#include "shared_mem.h"
#include "functions.h"

void init_program();
void init_log();
void create_named_pipe(char *name);
void create_msq();

#endif 