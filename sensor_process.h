//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

#ifndef sensor_process   /* Include guard */
#define sensor_process

#include "shared_mem.h"
#include "functions.h"

void handle_sigint(int signal);
void handle_sigtstp(int signal);
void send_message(char* id, char* key, int value);

#endif