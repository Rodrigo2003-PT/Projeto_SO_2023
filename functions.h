#ifndef functions   /* Include guard */
#define functions

//C libraries
#include "shared_mem.h"

//User define variables
#define line_lenght 20
#define number_of_configs 5

#define LOG_SEM_NAME "log_semaphore"

//Config file gesture
int *read_config_file();
void print_config_file();
void process_config_file(int *configs);

//Log_writing
void print(char *result);


#endif 
