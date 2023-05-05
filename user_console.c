//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

//TO-DO
//Implementar a receive_function que recebe info da message_queue

#include "user_console.h"

int pipe_fd;

int main(int argc, char **argv){

     if (argc != 2) {
        printf("Usage: %s <console_id>\n", argv[0]);
        exit(0);
    }

    console_pid = getpid();

    signal(SIGINT, sigint_handler);

    start_user_console();

    return 0;
}


void start_user_console() {

    if (pthread_create(&console_thread, NULL, console_function, NULL) != 0) {
        perror("Cannot create console thread: ");
        exit(1);
    }

    if (pthread_create(&console_receive, NULL, receive_function, NULL) != 0) {
        perror("Cannot create console thread: ");
        exit(1);
    }

    pthread_join(console_thread, NULL);

    pthread_join(console_receive, NULL);
}

void process_command(char *command) {

    char *cmd_copy = strdup(command);

    char *token = strtok(command, " ");

    if (strcmp(token, "exit") == 0) {
        pthread_cancel(console_thread);
        pthread_cancel(console_receive);
        exit(0);
    } 

    else if (strcmp(token, "stats") == 0) {
        send_command("stats\n");
    } 

    else if (strcmp(token, "reset") == 0) {
        send_command("reset\n");
    } 

    else if (strcmp(token, "sensors") == 0) {
        send_command("sensors\n");
    } 

    else if (strcmp(token, "add_alert") == 0) {

        char id[MAX_KEY_SIZE];
        char key[MAX_KEY_SIZE];
        int min_val, max_val;
        int num_scanned = sscanf(cmd_copy, "add_alert %s %s %d %d", id, key, &min_val, &max_val);

        if (num_scanned < 4) {
            printf("Usage: add_alert [id] [key] [min] [max]\n");
        }

        else {
            char buf[MAX_MSG_SIZE];
            sprintf(buf, "add_alert %d %s %s %d %d\n",console_pid, id, key, min_val, max_val);
            send_command(buf);
        }
    } 

    else if (strcmp(token, "remove_alert") == 0) {

        char id[MAX_KEY_SIZE];
        int num_scanned = sscanf(cmd_copy, "remove_alert %s", id);

        if (num_scanned < 1) {
            printf("Usage: remove_alert [id]\n");
        }
        
        else{
            char buf[MAX_MSG_SIZE];
            sprintf(buf, "remove_alert %s\n", id);       
            send_command(buf);

        }
    } 

    else if (strcmp(token, "list_alerts") == 0) {
        send_command("list_alerts\n");
    } 
    
    else {
        printf("Invalid command.\n");
    }

    free(cmd_copy);
}

void send_command(char *command) {

    if ((pipe_fd = open(PIPENAME_2, O_WRONLY)) < 0) {
            perror("Cannot open pipe for writing: ");
            exit(1);
        }

    if (write(pipe_fd, command, strlen(command)) == -1) {
        perror("write");
        exit(1);
    }
}

void *console_function(void *arg){

    while (1) {
        char command[MAX_MSG_SIZE];
        printf("> ");
        fflush(stdout);
        fgets(command, MAX_MSG_SIZE, stdin);
        command[strcspn(command, "\n")] = '\0';
        process_command(command);
    }
    return NULL;
}

void *receive_function(void *arg){
    alert_msg msg;
    while(1) {
        msgrcv(msq_id, &msg, sizeof(alert_msg), console_pid, 0);
        printf("Alert received for sensor %s: value = %d\n", msg.sensor_id, msg.triggered_value);
    }
    return NULL;
}

void sigint_handler(int sig) {
    printf("Console  process interrupted\n");
    close(pipe_fd);
    pthread_cancel(console_thread);
    pthread_cancel(console_receive);
    exit(0);
}
