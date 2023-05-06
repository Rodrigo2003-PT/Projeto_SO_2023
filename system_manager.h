//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

#ifndef system_manager
#define system_manager  /* Include guard */

//User libraries
#include "alerts_watcher.h"
#include "shared_mem.h"
#include "functions.h"
#include "worker.h"

#define WORKER_SEM_NAME "worker_semaphore"
#define ARRAY_SEM_NAME "array_semaphore"
#define ALERTS_SEM_NAME "alerts_semaphore"

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

struct DispatcherArgs {
  int (*pipes)[2];
  struct Queue* queue;
};

void create_unnamed_pipes(int pipes[][2]);
void create_named_pipe(char *name);
void *dispatcher_reader(void *arg);
void *console_reader(void *arg);
void *sensor_reader(void *arg);
void wait_alerts_watcher();
void cleanup(int sig);
void wait_workers();
void init_program();
void create_msq();
void init_log();

#endif 