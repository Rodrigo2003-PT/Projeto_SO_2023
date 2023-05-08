//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

//TO-DO
//Sincronizar sensor_thread

#include "sensor_process.h"

int msg_sent = 0;
int fd;

int main(int argc, char *argv[]){
    int value;

    //Ignora os sinais até fazer todas as inicializações
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);

    if (argc != 7) {
        printf("Usage: {sensor} {sensor id} {interval} {key} {min} {max}\n");
        exit(1);
    }

    char *id_input = argv[2];
    int interval = atoi(argv[3]);
    char *key = argv[4];
    int min = atoi(argv[5]);
    int max = atoi(argv[6]);

    // Validate input parameters
    if (strlen(id_input) < MIN_SENSOR_ID_SIZE || strlen(id_input) > MAX_SENSOR_ID_SIZE) {
        printf("Error: sensor id must be between 3 and 32 characters\n");
        exit(1);
    }
    if (strlen(key) < MIN_KEY_SIZE || strlen(key) > MAX_KEY_SIZE) {
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

    sensor_id sensor = {0};
    strcpy(sensor.id,id_input);

    // Set up sensor data
    sensor_chave data_chave = {
        .last_value = 0,
        .min_value = min,
        .max_value = max,
        .count = 0,
        .avg = -1,
        .alerts = {{0}}
    };

    for (int i = 0; i < ALERTS_PER_SENSOR; i++) {
        data_chave.alerts[i] = (sensor_alerts) {
            .pid = -1, 
            .alert_min = -1,
            .alert_max = -1,
            .alert_flag = 0,
            .alert_id = ""
        };
    }

    strcpy(data_chave.chave,key);

    while (1) {
        value = min + rand() % (max - min + 1);
        send_message(sensor.id, data_chave.chave, value);
        sleep(interval);
    }

    return 0;
}

void handle_sigtstp(int sig) {
    printf("Sent %d messages\n", msg_sent);
}

void handle_sigint(int sig) {
    printf("Sensor process terminated\n");
    close(fd);
    exit(0);
}

// function to send a message through the named pipe
void send_message(char* id, char* key, int value) {

    char message[MESSAGE_SIZE];

    sprintf(message, "%s#%s#%d\n", id, key, value);

    if ((fd = open(PIPENAME_1, O_WRONLY)) < 0) {
            perror("Cannot open pipe for writing: ");
            close(fd);
            exit(0);
    }

    if (write(fd, message, strlen(message)) == -1) {
        perror("write");
        exit(1);
    }

    printf("%s", message);

    msg_sent++;
}
