//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

#ifndef user_console   /* Include guard */
#define user_console

#include "shared_mem.h"
#include "functions.h"

#define MAX_MSG_SIZE 256

pid_t console_pid;

void start_user_console();
void sigint_handler(int sig);
void send_command(char *command);
void *console_function(void *arg);
void *receive_function(void *arg);
void process_command(char *command);

#endif