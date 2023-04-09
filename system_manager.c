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

  create_named_pipe(PIPENAME_1);
  create_named_pipe(PIPENAME_2);

  create_msq();
  
  struct Queue* queue = createQueue();

  // Create threads
  if (pthread_create(&console_reader_thread, NULL, console_reader, (void*) queue) != 0) {
      perror("Cannot create console thread: ");
      exit(1);
  }
  
  if (pthread_create(&dispatcher_thread, NULL, dispatcher_reader, (void*) queue) != 0) {
    perror("Cannot create console thread: ");
    exit(1);
  }

  if (pthread_create(&sensor_reader_thread, NULL, sensor_reader,(void*) queue) != 0) {
      perror("Cannot create sensor thread: ");
      exit(1);
  }
  
  pthread_join(console_reader_thread, NULL);
  pthread_join(dispatcher_thread, NULL);
  pthread_join(sensor_reader_thread, NULL);

  for(int i = 0; i < config->num_workers; i++){
    worker_process = fork();
    if(worker_process == 0){
      worker_init(read_from_pipe());
      exit(0);
    }
  }

  alerts_watcher_process = fork();
  if(alerts_watcher_process == 0) alerts_watcher_init();

  for(int i = 0; i < 2; i++)wait(NULL);
}

void init_program(){
  //Generate global structure shared memory

  // shared_mem size incomplete
  int shared_mem_size = (sizeof(sensor_struct) * config->max_sensors) + (sizeof(int) * config->num_workers) + sizeof(int);
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
  count_key = *(first_worker + config->num_workers);
  
  // clean_data();
}

//Log management
void init_log(){
  //Delete semaphore if is open
  sem_unlink(LOG_SEM_NAME);
  log_file = fopen("log.txt", "w");
  log_semaphore = sem_open(LOG_SEM_NAME, O_CREAT | O_EXCL, 0777, 1);
}

void create_named_pipe(char *name){
  unlink(name);
  if ((mkfifo(name, O_CREAT|O_EXCL|0600)<0) && (errno != EEXIST)){
    print("CANNOT CREATE NAMED PIPE -> EXITING\n");
    exit(1);
  }
}

void create_msq(){
  if((msq_id = msgget(IPC_PRIVATE, IPC_CREAT|0777)) == -1){
    print("Error creating message queue");
    exit(0);
  }
}

// This thread needs to be synchronized
void *sensor_reader(void *arg){

  // Opens the pipe for reading
  int fd;
  if ((fd = open(PIPENAME_1, O_RDONLY)) < 0) {
    perror("Cannot open pipe for reading: ");
    exit(0);
  }

  // Do some work
  char buffer[MESSAGE_SIZE];
  struct Queue* queue = (struct Queue*) arg;

  while (1) {

      memset(buffer, 0, MESSAGE_SIZE);

      // Lê a mensagem do named pipe
      ssize_t bytes_read = read(fd, buffer, MESSAGE_SIZE);

      if (bytes_read > 0) {
          // Tenta inserir a mensagem na fila interna
          if (queue_size(queue) < config->queue_slot_number){
            enqueue(queue,buffer);
          }

          else{
            printf("Internal queue is full! Discarding message!");
          }
      } 
      else {
          perror("read");
          // TODO: escrever no arquivo de log
          break;
      }
  }
  return NULL;
};

// This thread needs to be synchronized
void *console_reader(void *arg){
  
  // Opens the pipe for reading
  int fd;
  if ((fd = open(PIPENAME_2, O_RDONLY)) < 0) {
    perror("Cannot open pipe for reading: ");
    exit(0);
  }

  // Do some work
  char buffer[MESSAGE_SIZE];
  struct Queue* queue = (struct Queue*) arg;

  while (1) {

     memset(buffer, 0, MESSAGE_SIZE);

      // Lê a mensagem do named pipe
      ssize_t bytes_read = read(fd, buffer, MESSAGE_SIZE);

      if (bytes_read > 0) {
          // Tenta inserir a mensagem na fila interna
          if (queue_size(queue) < config->queue_slot_number)
              enqueue(queue,buffer);

          else
            printf("Internal queue is full! Discarding message!");
      } 
      else {
          perror("read");
          // TODO: escrever no arquivo de log
          break;
      }
  }
  return NULL;
};

void *dispatcher_reader(void *arg){

  struct Queue* queue = (struct Queue*) arg;

  while (1) {

    // Check if there are messages in the queue
    char *msg = dequeue(queue);

    if (msg == NULL) {
      usleep(100000); // sleep for 100ms
      continue;
    }

    // Find a free worker

    // No free workers, wait for one to become available

    // Send the message to the worker

    // Make worker busy

    free(msg); // free memory allocated by queue_pop
  }
  
  return NULL;
};
