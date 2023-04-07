#ifndef user_console   /* Include guard */
#define user_console

#include "shared_mem.h"
#include "functions.h"

#define MAX_MSG_SIZE 256

void start_user_console();
void sigint_handler(int sig);
void send_command(char *command);
void *console_function(void *arg);
void *receive_function(void *arg);
void process_command(char *command);

#endif