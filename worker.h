//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

#ifndef worker   /* Include guard */
#define worker

#include "shared_mem.h"
#include "functions.h"

typedef struct worker_sensor {
    char* id;
    char* chave;
    int value;
}worker_sensor;

void worker_init(int* pipe_fd);

worker_sensor create_worker_sensor(char* msg);

#endif