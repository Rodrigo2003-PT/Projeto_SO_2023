//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

//TO-DO
//Process messages from user_console

#include "worker.h"

int sensor_exists = 0;

void worker_init(int* pipe_fd){
    while (running) {
        close(pipe_fd[1]);
        char *msg = read_from_pipe(pipe_fd[0]);
        printf("Worker message: %s\n", msg);
        char *result = strchr(msg, '#');
        
        // Se a mensagem for um dado da user_console
        if(result == NULL){
            if(strncmp(msg, "add_alert", strlen("add_alert")) == 0){
                char id[MAX_KEY_SIZE];
                char key[MAX_KEY_SIZE];
                int min_val, max_val;
                pid_t console_pid;
                if (sscanf(msg, "add_alert %d %s %s %d %d",&console_pid, id, key, &min_val, &max_val) == 5){
                    sem_wait(array_sem);
                    for (int i = 0; i < config->max_sensors; i++) {
                        if (sensor[i].id != NULL && strcmp(sensor[i].data.chave, key) == 0) {
                            for (int j = 0; j < ALERTS_PER_SENSOR; j++){
                                 if (sensor[i].alerts[j].alert_id == NULL){
                                    sensor[i].alerts[j].pid = console_pid;
                                    sensor[i].alerts[j].alert_id = id;
                                    sensor[i].alerts[j].alert_flag = 1;
                                    sensor[i].alerts[j].alert_min = min_val;
                                    sensor[i].alerts[j].alert_max = max_val;
                                 }
                            }
                        sensor_exists = 1;
                        break;
                        }
                    }
                    if(!sensor_exists){
                        printf("Cannot add_alert to a non existing_sensor\n");
                    }
                }
            }

            if(strncmp(msg,"remove_alert",strlen("remove_alert")) == 0){
                char id[MAX_KEY_SIZE];
                if(sscanf(msg,"remove_alert %s",id) == 1){
                    sem_wait(array_sem);
                    for (int i = 0; i < config->max_sensors; i++) {
                        for (int j = 0; j < ALERTS_PER_SENSOR; j++){
                            if (sensor[i].id != NULL && strcmp(sensor[i].alerts[j].alert_id, id) == 0) {
                                sensor[i].alerts[j].pid = -1;
                                sensor[i].alerts[j].alert_min = 0;
                                sensor[i].alerts[j].alert_max = 0;
                                sensor[i].alerts[j].alert_flag = 0;
                                sensor[i].alerts[j].alert_id = NULL;
                                sensor_exists = 1;
                            }
                        }
                        if(sensor_exists){
                            break;
                        }
                    }
                    if(!sensor_exists){
                        printf("Cannot remove_alert from a non existing_sensor\n");
                    }
                }
            }

            if(strncmp(msg,"list_alerts",strlen("list_alerts")) == 0){
                sem_wait(array_sem);
                for (int i = 0; i < config->max_sensors; i++) {
                    if(sensor[i].id != NULL){
                        for (int j = 0; j < ALERTS_PER_SENSOR; j++){
                            char* alert = sensor[i].alerts[j].alert_id;
                            if (alert != NULL) { 
                                printf("Alerta->%s",alert);
                            }
                        }
                    }
                }
            }
        }

        // Se a mensagem for um dado do sensor
        else{

            worker_sensor ws = create_worker_sensor(msg);
            sensor_exists = 0;

            // Search for the sensor structure with the given key
            sem_wait(array_sem);
            for(int i = 0; i < config->max_sensors; i++){
                if(sensor[i].id != NULL && strcmp(sensor[i].id, ws.id) == 0){
                    // Found the sensor structure, now access its data fields
                    if(strcmp(sensor[i].data.chave, ws.chave) == 0){
                        //Primeira vez a receber dados do sensor no sistema.
                        if(sensor[i].data.count == 0)count_key++;
                        sensor[i].data.last_value = ws.value;
                        sensor[i].data.avg = (sensor[i].data.last_value + sensor[i].data.min_value + sensor[i].data.max_value) / 3;
                        if(ws.value < sensor[i].data.min_value)sensor[i].data.min_value = ws.value;
                        if(ws.value > sensor[i].data.max_value)sensor[i].data.max_value = ws.value;
                        sensor[i].data.count++;
                    }
                    free(ws.id);
                    free(ws.chave);
                    sensor_exists = 1;
                    break;
                }
            }

            if(!sensor_exists){
                if(count_key < config->max_keys){
                    int i = 0;
                    for(i = 0; i < config->max_sensors; i++){
                        if(sensor[i].id == NULL) break;
                    }
                    if (i == config->max_sensors) {
                        printf("Error: maximum number of sensors reached\n");
                    }

                    // Allocate memory for the new sensor structure
                    sensor[i].id = strdup(ws.id);
                    free(ws.id);
                    for (int j = 0; j < ALERTS_PER_SENSOR; j++) {
                        sensor[i].alerts[j].alert_flag = 0;
                        sensor[i].alerts[j].pid = -1;
                        sensor[i].alerts[j].alert_min = -1;
                        sensor[i].alerts[j].alert_max = -1;
                        sensor[i].alerts[j].alert_id = NULL;
                    }
                    sensor[i].data.chave = strdup(ws.chave);
                    free(ws.chave);
                    sensor[i].data.last_value = ws.value;
                    sensor[i].data.min_value = ws.value;
                    sensor[i].data.max_value = ws.value;
                    sensor[i].data.count = 1;
                    sensor[i].data.avg = ws.value;
                    count_key++;
                }
            }
        }

        for (int i = 0; i < config->num_workers; i++) {
            int worker_state = *(first_worker + i);
            if (worker_state == 0) {
                worker_state = 1;
                break;
            }
        }
        sem_post(array_sem);
        sem_post(alerts_sem);
        sem_post(worker_sem);  
    }
}

worker_sensor create_worker_sensor(char* msg){
    worker_sensor ws;
    char* token = strtok(msg, "#");
    ws.id = strdup(token);
    token = strtok(NULL, "#");
    ws.chave = strdup(token);
    token = strtok(NULL, "#");
    ws.value = atoi(token);
    return ws;
}