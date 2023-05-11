//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

//TO-DO
//FINISHED

#include "system_manager.h"

int fd_1;
int fd_2;
int *configs = NULL;

int main(){
  running = 1;
  count_alerts = 0;

  //Ignora o sinal SIG_INT e SIGTSTP até fazer todas as inicializações
  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGUSR1, SIG_IGN);

  //read config file
  configs = read_config_file();
  if (configs == NULL){
    printf("ERROR READING CONFIG FILE\n");
  } 
  process_config_file(configs);

  init_log();

  init_program();

  print("SYSTEM STARTING\n");

  int pipes[config->num_workers][2];
  create_unnamed_pipes(pipes);
  create_named_pipe(PIPENAME_1);
  create_named_pipe(PIPENAME_2);
  create_msq();

  queue_sensor = createQueue();
  queue_console = createQueue();
  struct DispatcherArgs dispatcher_args = { .pipes = pipes, .queue_sensor = queue_sensor, .queue_console = queue_console };

  alerts_watcher_process = fork();
  if(alerts_watcher_process == 0){
    alerts_watcher_init();
    exit(0);
  }

  print("ALERTS_WATCHER CREATED\n");

  worker_pid = (pid_t*) malloc(config->num_workers * sizeof(pid_t));
  for(int i = 0; i < config->num_workers; i++){
    worker_pid[i] = fork();
    if(worker_pid[i] == 0){
      worker_init(pipes[i]);
      exit(0);
    }
  }

  print("WORKERS CREATED\n");

  // Create threads
  if (pthread_create(&console_reader_thread, NULL, console_reader, (void*) queue_console) != 0) {
    print("CANNOT CREATE CONSOLE_THREAD\n");
    exit(1);
  }
  
  if (pthread_create(&dispatcher_thread, NULL, dispatcher_reader, (void*) &dispatcher_args) != 0) {
    print("CANNOT CREATE DISPATCHER_THREAD\n");
    exit(1);
  }

  if (pthread_create(&sensor_reader_thread, NULL, sensor_reader,(void*) queue_sensor) != 0) {
    print("CANNOT CREATE SENSOR_THREAD\n");
    exit(1);
  }

  signal(SIGINT, cleanup);

  wait(NULL);
}

void init_program(){
  //Generate global structure shared memory

  // shared_mem size incomplete
  int shared_mem_size = (sizeof(sensor_id) * config->max_sensors) + (sizeof(int) * config->num_workers) + sizeof(int) + (sizeof(sensor_chave) * config->max_keys);
  if ((shm_id = shmget(IPC_PRIVATE, shared_mem_size, IPC_CREAT | IPC_EXCL | 0700)) < 1){
    print("ERROR IN SHMGET WITH IPC_CREAT\n");
    exit(1);
  }

  if((sensor = (sensor_id*) shmat(shm_id, NULL, 0)) == (sensor_id*)-1){
      print("ERROR ATTACHING SHARED MEMORY\n");
      exit(0);
  }

  //Define first of each type for easy consulting
  first_worker = (int*) &sensor[config->max_sensors];
  count_key = *(first_worker + config->num_workers);
  chave = (sensor_chave*)(&first_worker[config->num_workers + 1]);;

  //Initialize sensor id
  memset(sensor, 0, sizeof(sensor_id) * config->max_sensors);

  // Initialize all workers to 1
  for (int i = 0; i < config->num_workers; i++) {
    *(first_worker + i) = 1;
  }

  count_key = 0;

  //Initialize all chaves
  for (int i = 0; i < config->max_keys; i++) {
    chave[i] = (sensor_chave) {
      .chave = "",
      .sensor = "",
      .last_value = -999,
      .min_value = -999,
      .max_value = 999,
      .count = 0,
      .avg = -999,
      .last_update = 0,
      .alerts = {{0}}
    };
    for (int j = 0; j < ALERTS_PER_SENSOR; j++) {
      chave[i].alerts[j] = (sensor_alerts) {
          .pid = -1, 
          .alert_min = -999,
          .alert_max = 999,
          .alert_flag = 0,
          .alert_id = ""
      };
    }
  };

  sem_unlink(ARRAY_SEM_NAME);
  array_sem = sem_open(ARRAY_SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
  if (array_sem == SEM_FAILED) {
      perror("sem_open");
      exit(1);
  }

  sem_unlink(WORKER_SEM_NAME);
  worker_sem = sem_open(WORKER_SEM_NAME, O_CREAT | O_EXCL, 0666, config->num_workers);
  if (worker_sem == SEM_FAILED) {
      perror("sem_open");
      exit(1);
  }

  sem_unlink(ALERTS_SEM_NAME);
  alerts_sem = sem_open(ALERTS_SEM_NAME, O_CREAT | O_EXCL, 0666, 0);
  if (alerts_sem == SEM_FAILED) {
      perror("sem_open");
      exit(1);
  }

  sem_unlink(CONTROL_SEM_NAME);
  control_sem = sem_open(CONTROL_SEM_NAME, O_CREAT | O_EXCL, 0666, 0);
  if (control_sem == SEM_FAILED) {
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
  }
  free(worker_pid);
  print("WORKERS TERMINATED\n");
}

void wait_alerts_watcher(){
  int status;
  if (waitpid(alerts_watcher_process, &status, 0) == -1) {
    perror("waitpid");
  }
  print("ALERTS_WATCHER TERMINATED\n");
}

void create_unnamed_pipes(int pipes[][2]){
  for (int i = 0; i < config->num_workers; i++) {
        if (pipe(pipes[i]) == -1) {
          print("CANNOT CREATE UNNAMED PIPE -> EXITING\n");
          exit(1);
        }
    }
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
    print("ERROR CREATING MSG QUEUE\n");
    exit(1);
  }
  FILE *fp = fopen(MSQ_FILE, "w");
  if (fp == NULL) {
    print("ERROR OPENING FILE -> MSG_QUEUE_ID\n");
    exit(1);
  }
  fprintf(fp, "%d", msq_id);
  fclose(fp);
}

void cleanup(int sig) {
  print("SIGINT SIGNALED\n");
  print("SYSTEM EXITING\n");
  running = 0;


  //CLOSE PROCESSES
  if (alerts_watcher_process > 0) kill(alerts_watcher_process, SIGTERM);
  for (int i = 0; i < config->num_workers; i++) {
    if (worker_pid[i] > 0) kill(worker_pid[i], SIGTERM);
  }

  //Terminate Threads
  pthread_cancel(dispatcher_thread);
  pthread_cancel(sensor_reader_thread);
  pthread_cancel(console_reader_thread);

  write_Queue(queue_sensor);
  destroyQueue(queue_sensor);

  //Detach/delete shared memory
  if (shmdt(sensor) == -1)print("ERROR DETACHING SHM SEGMENT\n");
  if(shmctl(shm_id, IPC_RMID, NULL) == -1)print("ERROR REMOVING SHM SEGMENT\n");

  //Delete message queue
  if (msgctl(msq_id, IPC_RMID, NULL) == -1)print("ERROR DELETING MSG_QUEUE\n");

  //Close and unlink named pipes
  if (fcntl(fd_1, F_GETFL) != -1) {
    if (close(fd_1) == -1)print("ERROR CLOSING PIPE FD1\n");
  }
  if (unlink(PIPENAME_1) == -1)print("ERROR UNLINKING PIPE PIPENAME_1\n");

  if (fcntl(fd_2, F_GETFL) != -1) {
    if (close(fd_2) == -1)print("ERROR CLOSING PIPE FD2\n");
  }
  if (unlink(PIPENAME_2) == -1)print("ERROR UNLINKING PIPE PIPENAME_2\n");
    
  if (remove(MSQ_FILE) != 0) {
      print("ERROR DELETING FILE -> MSG_QUEUE_ID\n");
  }

  //Close log file and destroy semaphore
  if (sem_close(log_semaphore) == -1)print("ERROR CLOSING LOG_SEMAPHORE\n");
  if (sem_unlink(LOG_SEM_NAME) == -1)print("ERROR UNLINKING LOG_SEMAPHORE\n");
  if (sem_close(array_sem) == -1)print("ERROR CLOSING ARRAY_SEMAPHORE\n");
  if (sem_unlink(ARRAY_SEM_NAME) == -1)print("ERROR UNLINKING ARRAY_SEMAPHORE\n");
  if (sem_close(worker_sem) == -1)print("ERROR CLOSING WORKER_SEMAPHORE\n");
  if (sem_unlink(WORKER_SEM_NAME) == -1)print("ERROR UNLINKING WORKER_SEMAPHORE\n");
  if (sem_close(control_sem) == -1)print("ERROR CLOSING CONTROL_SEMAPHORE\n");
  if (sem_unlink(CONTROL_SEM_NAME) == -1)print("ERROR UNLINKING CONTROL_SEMAPHORE\n");
  if (fclose(log_file) == EOF)print("ERROR CLOSING LOG FILE\n");
  

  free(configs);
  exit(0);
}

void *sensor_reader(void *arg){

  // Opens the pipe for reading
  if ((fd_1 = open(PIPENAME_1, O_RDONLY)) < 0) {
    print("CANNOT OPEN PIPENAME_1 FOR READING -> THREAD SENSOR_READER\n");
    exit(0);
  }

  char buffer[MESSAGE_SIZE];
  struct Queue* queue = (struct Queue*) arg;

  while (running) {

      memset(buffer, 0, MESSAGE_SIZE);

      // Lê a mensagem do named pipe
      ssize_t bytes_read = read(fd_1, buffer, MESSAGE_SIZE);

      if (bytes_read > 0) {
          // Tenta inserir a mensagem na fila interna
          if (queue_size(queue) < config->queue_slot_number){
            pthread_mutex_lock(&queue_sensor_mutex);
            enqueue(queue,buffer);
            pthread_mutex_unlock(&queue_sensor_mutex);
            sem_post(control_sem);
          }

          else
          {
            print("QUEUE IS FULL! DISCARDING MESSAGE\n");
          }
      } 
  }
  return NULL;
};

void *console_reader(void *arg){
  
  // Opens the pipe for reading
  if ((fd_2 = open(PIPENAME_2, O_RDONLY)) < 0) {
    print("CANNOT OPEN PIPENAME_2 FOR READING -> THREAD CONSOLE_READER\n");
    exit(0);
  }

  // Do some work
  char buffer[MESSAGE_SIZE];
  struct Queue* queue = (struct Queue*) arg;

  while (running) {

     memset(buffer, 0, MESSAGE_SIZE);

      // Lê a mensagem do named pipe
      ssize_t bytes_read = read(fd_2, buffer, MESSAGE_SIZE);

      if (bytes_read > 0) {
        pthread_mutex_lock(&queue_console_mutex);
        while (queue_size(queue) >= config->queue_slot_number) {
          // The queue is full, so wait until there is space available
          int ret = pthread_cond_wait(&cond_block_console, &queue_console_mutex);
          if (ret == EINTR) {
              pthread_exit(NULL);
          }
        }
        enqueue(queue, buffer);
        pthread_mutex_unlock(&queue_console_mutex);
        sem_post(control_sem);
      }
  }
  return NULL; 
};

void *dispatcher_reader(void *arg){
  struct DispatcherArgs *args = (struct DispatcherArgs*) arg;
  int (*pipes)[2] = args->pipes;
  struct Queue* queue_sensor = args->queue_sensor;
  struct Queue* queue_console = args->queue_console;

  while (running) {
    sem_wait(control_sem);
    // Check if there are messages in the queue_console
    pthread_mutex_lock(&queue_console_mutex);
    if (!isEmpty(queue_console)) {
      while (!isEmpty(queue_console)){
        char *msg = dequeue(queue_console);
        if (queue_size(queue_console) == (config->queue_slot_number - 1)) {
          // Notify waiting threads that there is space available in the queue
          pthread_mutex_unlock(&queue_console_mutex);
          pthread_cond_signal(&cond_block_console);
        }
        else
          pthread_mutex_unlock(&queue_console_mutex);
        process_dispatcher_message(msg,pipes);
      }
    }
    else{
      pthread_mutex_unlock(&queue_console_mutex);
    }
    // Check if there are messages in the queue_sensor
    pthread_mutex_lock(&queue_sensor_mutex);
    if (!isEmpty(queue_sensor)) {
      char *msg = dequeue(queue_sensor);
      pthread_mutex_unlock(&queue_sensor_mutex);
      process_dispatcher_message(msg,pipes);
    }
    else{
        pthread_mutex_unlock(&queue_sensor_mutex);
    }
  }
  return NULL;
};