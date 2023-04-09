#include "sensor_process.h"

int msg_sent = 0;

int main(int argc, char *argv[]){

    time_t start_time, current_time;
    int value;

    //Ignora os sinais até fazer todas as inicializações
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);

    sensor_pid = getpid();

    if (argc != 7) {
        printf("Usage: {sensor} {sensor id} {interval} {key} {min} {max}\n");
        exit(1);
    }

    char *sensor_id = argv[2];
    int interval = atoi(argv[3]);
    char *key = argv[4];
    int min = atoi(argv[5]);
    int max = atoi(argv[6]);

    // Validate input parameters
    if (strlen(sensor_id) < MIN_LENGTH || strlen(sensor_id) > MAX_LENGTH) {
        printf("Error: sensor id must be between 3 and 32 characters\n");
        exit(1);
    }
    if (strlen(key) < MIN_LENGTH || strlen(key) > MAX_LENGTH) {
        printf("Error: key must be between 3 and 32 characters\n");
        exit(1);
    }

    if (interval < 0) {
        printf("Error: interval must be greater than or equal to 0\n");
        return 1;
    }

    if (min >= max) {
        printf("Error: min must be less than max\n");
        exit(1);
    }

    // Set up signal handlers
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);

    // Set up sensor data
    sensor_struct sensor = {
    .data = {
        {
            .chave = key,
            .last_value = 0,
            .min_value = min,
            .max_value = max,
            .count = 0,
            .avg = 0
        }
    },

    .intervalo = interval,
    .id = sensor_id,
    .alerts = {}
};
    
    start_time = time(NULL);

    while (1) {
        current_time = time(NULL);
        if (current_time - start_time >= interval) {
            value = min + rand() % (max - min + 1);
            send_message(sensor.id, sensor.data->chave, value);
            start_time = current_time;
        }
    }

    return 0;
}

void handle_sigtstp(int sig) {
    printf("Sent %d messages\n", msg_sent);
}

void handle_sigint(int sig) {
    printf("Sensor process terminated\n");
    unlink(PIPENAME_1);
    exit(0);
}

// function to send a message through the named pipe
void send_message(char* id, char* key, int value) {

    int fd;
    char message[MESSAGE_SIZE];

    sprintf(message, "%s#%s#%d\n", id, key, value);

    if ((fd = open(PIPENAME_1, O_WRONLY)) < 0) {
            perror("Cannot open pipe for writing: ");
            exit(0);
    }

    // Program frozes here because the open call blocks until another process or thread opens the same named pipe for reading.

    if (write(fd, message, strlen(message)) == -1) {
        perror("write");
        exit(1);
    }

    close(fd);

    msg_sent++;
}
