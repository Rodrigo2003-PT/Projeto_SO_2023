#ifndef sensor_process   /* Include guard */
#define sensor_process

#include "shared_mem.h"
#include "functions.h"

#define MAX_KEY_LENGTH 32
#define MAX_ID_LENGTH 32
#define MESSAGE_SIZE 64

pid_t sensor_pid;

void handle_sigint(int signal);
void handle_sigtstp(int signal);
void send_message(char* id, char* key, int value);

#endif