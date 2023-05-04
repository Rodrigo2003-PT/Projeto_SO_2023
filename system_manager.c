//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

//TO-DO
//Finish dispatcher
//Implement read_from_pipe -> worker reading from unnamed_pipe
//Clean resources

#include "system_manager.h"

int fd_1;
int fd_2;
int *configs = NULL;

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
  configs = read_config_file();
  if (configs == NULL){
    printf("Error reading config file");
    print("Error reading config file");
  } 
  process_config_file(configs);

  init_log();

  //generate shared memory and control mechanisms
  init_program();

  int pipes[config->num_workers][2];
  create_unnamed_pipes(pipes);
  create_named_pipe(PIPENAME_1);
  create_named_pipe(PIPENAME_2);
  create_msq();

  struct Queue* queue = createQueue();
  struct DispatcherArgs dispatcher_args = { .pipes = pipes, .queue = queue };


  signal(SIGINT, cleanup);

  alerts_watcher_process = fork();
  if(alerts_watcher_process == 0) alerts_watcher_init();

  worker_pid = (pid_t*) malloc(config->num_workers * sizeof(pid_t));
  for(int i = 0; i < config->num_workers; i++){
    worker_pid[i] = fork();
    if(worker_pid[i] == 0){
      worker_init(pipes[i]);
      exit(0);
    }
  }

  // Create threads
  if (pthread_create(&console_reader_thread, NULL, console_reader, (void*) queue) != 0) {
    perror("Cannot create console thread: ");
    exit(1);
  }
  
  if (pthread_create(&dispatcher_thread, NULL, dispatcher_reader, (void*) &dispatcher_args) != 0) {
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

  wait_workers();
  wait_alerts_watcher();

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

  // Initialize all workers to 1
  for (int i = 0; i < config->num_workers; i++) {
    *(first_worker + i) = 1;
  }

  sem_unlink(ARRAY_SEM_NAME);
  array_sem = sem_open(ARRAY_SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
  if (array_sem == SEM_FAILED) {
      perror("sem_open");
      exit(1);
  }

  sem_unlink(WORKER_SEM_NAME);
  worker_sem = sem_open(WORKER_SEM_NAME, O_CREAT | O_EXCL, 0666, 0);
  if (worker_sem == SEM_FAILED) {
      perror("sem_open");
      exit(1);
  }
}

//Log management
void init_log(){
  //Delete semaphore if is open
  sem_unlink(LOG_SEM_NAME);
  log_file = fopen("log.txt", "w");
  log_semaphore = sem_open(LOG_SEM_NAME, O_CREAT | O_EXCL, 0777, 1);
  if (log_semaphore == SEM_FAILED) {
      perror("sem_open");
      exit(1);
  }
}

void wait_workers(){
  for (int i = 0; i < config->num_workers; i++) {
    int status;
    if (waitpid(worker_pid[i], &status, 0) == -1) {
        perror("waitpid");
    }
    printf("Worker process %d terminated with status %d\n", worker_pid[i], status);
  }
  free(worker_pid);
  print("Worker processes terminated");
}

void wait_alerts_watcher(){
  int status;
  if (waitpid(alerts_watcher_process, &status, 0) == -1) {
    perror("waitpid");
  }
  printf("Alerts watcher process %d terminated with status %d\n", alerts_watcher_process, status);
  print("Alerts_Watcher process terminated");
}

void create_unnamed_pipes(int pipes[][2]){
  for (int i = 0; i < config->num_workers; i++) {
        if (pipe(pipes[i]) == -1) {
          printf("CANNOT CREATE UNNAMED PIPE -> EXITING\n");
          print("CANNOT CREATE UNNAMED PIPE -> EXITING\n");
          exit(1);
        }
    }
}

void create_named_pipe(char *name){
  unlink(name);
  if ((mkfifo(name, O_CREAT|O_EXCL|0600)<0) && (errno != EEXIST)){
    printf("CANNOT CREATE NAMED PIPE -> EXITING\n");
    print("CANNOT CREATE NAMED PIPE -> EXITING\n");
    exit(1);
  }
}

void create_msq(){
  if((msq_id = msgget(IPC_PRIVATE, IPC_CREAT|0777)) == -1){
    printf("Error creating message queue");
    print("Error creating message queue");
    exit(1);
  }
}

void cleanup(int sig) {

  print("System terminating\n");

  // // Send SIGTERM signal to worker processes
  // for (int i = 0; i < config->num_workers; i++) {
  //   kill(worker_pid[i], SIGKILL);
  // }

  // while (wait(NULL) != -1);

  //Detach shared memory
  if (shmdt(sensor) == -1) {
    perror("Error detaching shared memory segment");
  }

  //Delete message queue
  if (msgctl(msq_id, IPC_RMID, NULL) == -1) {
    perror("Error deleting message queue");
  }

  //Close and unlink named pipes
  if (close(fd_1) == -1) {
    perror("Error closing pipe fd1");
  }
  if (unlink(PIPENAME_1) == -1) {
    perror("Error unlinking pipe PIPENAME_1");
  }
  if (close(fd_2) == -1) {
    perror("Error closing pipe fd2");
  }
  if (unlink(PIPENAME_2) == -1) {
    perror("Error unlinking pipe PIPENAME_2");
  }

  //Close log file and destroy semaphore
  fclose(log_file);
  sem_close(log_semaphore);
  sem_unlink(LOG_SEM_NAME);
  sem_close(array_sem);
  sem_unlink(ARRAY_SEM_NAME);
  sem_close(worker_sem);
  sem_unlink(WORKER_SEM_NAME);

  free(configs);

  exit(0);
}

// This thread needs to be synchronized
void *sensor_reader(void *arg){

  // Opens the pipe for reading
  if ((fd_1 = open(PIPENAME_1, O_RDONLY)) < 0) {
    perror("Cannot open pipe for reading: ");
    exit(0);
  }

  // Do some work
  char buffer[MESSAGE_SIZE];
  struct Queue* queue = (struct Queue*) arg;

  while (1) {

      memset(buffer, 0, MESSAGE_SIZE);

      // Lê a mensagem do named pipe
      ssize_t bytes_read = read(fd_1, buffer, MESSAGE_SIZE);
      printf("The number of bytes read is: %zd\n", bytes_read);
      printf("MSG recebida sensor_reader: %s",buffer);

      if (bytes_read > 0) {
          // Tenta inserir a mensagem na fila interna
          if (queue_size(queue) < config->queue_slot_number){
            pthread_mutex_lock(&queue_mutex);
            enqueue(queue,buffer);
            pthread_cond_signal(&queue_cond);
            pthread_mutex_unlock(&queue_mutex);
          }

          else{
            printf("Internal queue is full! Discarding message!");
          }
      } 
      else {
          perror("read");
          // TODO: escrever no arquivo de log
      }
  }
  return NULL;
};

// This thread needs to be synchronized
void *console_reader(void *arg){
  
  // Opens the pipe for reading
  if ((fd_2 = open(PIPENAME_2, O_RDONLY)) < 0) {
    perror("Cannot open pipe for reading: ");
    exit(0);
  }

  // Do some work
  char buffer[MESSAGE_SIZE];
  struct Queue* queue = (struct Queue*) arg;

  while (1) {

     memset(buffer, 0, MESSAGE_SIZE);

      // Lê a mensagem do named pipe
      ssize_t bytes_read = read(fd_2, buffer, MESSAGE_SIZE);

      if (bytes_read > 0) {
          // Tenta inserir a mensagem na fila interna
          if (queue_size(queue) < config->queue_slot_number){
            pthread_mutex_lock(&queue_mutex);
            enqueue(queue,buffer);
            pthread_cond_signal(&queue_cond);
            pthread_mutex_unlock(&queue_mutex);
          }

          else
            printf("Internal queue is full! Discarding message!");
      } 
      else {
          perror("read");
          // TODO: escrever no arquivo de log
      }
  }
  return NULL;
};

void *dispatcher_reader(void *arg){

  struct DispatcherArgs *args = (struct DispatcherArgs*) arg;
  int (*pipes)[2] = args->pipes;
  struct Queue* queue = args->queue;

  while (1) {

    // Check if there are messages in the queue
    pthread_mutex_lock(&queue_mutex);
    while (isEmpty(queue)) {
      pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    char *msg = dequeue(queue);
    if (msg != NULL) printf("Message rcv by dispatcher: %s", msg);
    pthread_mutex_unlock(&queue_mutex);

    // Find a free worker
    int free_worker = -1;

    sem_wait(array_sem);
      for (int i = 0; i < config->num_workers; i++) {
        int worker_state = *(first_worker + i);
        if (worker_state == 1) {
          free_worker = i;
          worker_state = 0;
          sem_post(array_sem);
          break;
        }
      }

    // Wait for one to become available
    if (free_worker == -1){
      sem_wait(worker_sem);
      sem_wait(array_sem);
      for (int i = 0; i < config->num_workers; i++) {
        int worker_state = *(first_worker + i);
        if (worker_state == 1) {
          free_worker = i;
          worker_state = 0;
          sem_post(array_sem);
          break;
        }
      }
    }

    // Send the message to the worker
    int bytes_written = write(pipes[free_worker][1], msg, strlen(msg));
    if (bytes_written < 0) {
      perror("Error writing to pipe");
      exit(1);
    }
  }
  
  return NULL;
};
