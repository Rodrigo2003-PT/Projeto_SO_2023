#ifndef sensor_process   /* Include guard */
#define sensor_process

#include "shared_mem.h"
#include "functions.h"

pid_t sensor_pid;

void handle_sigint(int signal);
void handle_sigtstp(int signal);
void create_named_pipe(char *name);
void send_message(char* id, char* key, int value);

#endif