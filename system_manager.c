#include "system_manager.h"

int main(){

  //Ignora o sinal SIG_INT e SIGTSTP até fazer todas as inicializações
  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGUSR1, SIG_IGN);

  //Save Process Pid to clean it
  main_pid = getpid();

  shm_id = -1;
  msq_id = -1;

  //read config file
  int *configs = NULL;
  configs = read_config_file();
  if (configs == NULL) printf("Error reading file or invalid number of teams\ncheck if your file is config.txt or the number of teams (line 3) is bigger than 3!");
  process_config_file(configs);
  free(configs);

  init_log();

  //generate shared memory and control mechanisms
  init_program();

}

void init_program(){
  //Generate global structure shared memory

  // shared_mem size incomplete
  int shared_mem_size = (sizeof(sensor_struct) * config->max_sensors) + (sizeof(int) * config->num_workers);
  if ((shm_id = shmget(IPC_PRIVATE, shared_mem_size, IPC_CREAT | IPC_EXCL | 0700)) < 1){
    print("Error in shmget with IPC_CREAT\n");
    exit(1);
  }

  if((sensor = (sensor_struct *) shmat(shm_id, NULL, 0)) == (sensor_struct*)-1){
      print("Error attaching shared memory in race_manager process");
      exit(0);
  }

  //Define first of each type for easy consulting
  first_worker = (int*) &sensor[config->max_sensors];
  
  clean_data();
}

//Log management
void init_log(){
  //Delete semaphore if is open
  sem_unlink(LOG_SEM_NAME);
  log_file = fopen("log.txt", "w");
  log_semaphore = sem_open(LOG_SEM_NAME, O_CREAT | O_EXCL, 0777, 1);
}
