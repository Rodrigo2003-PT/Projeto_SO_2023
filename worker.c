//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

//TO-DO
//Process messages from user_console

#include "worker.h"

void worker_init(int* pipe_fd){

    while (1) {
        close(pipe_fd[1]);
        char *msg = read_from_pipe(pipe_fd[0]);
        printf("Worker message: %s", msg);

        char *result = strchr(msg, '#');
        
        // Se a mensagem for um dado da user_console
        if(result == NULL){
            if(strncmp(msg, "add_alert", strlen("add_alert")) == 0){
                char *id = NULL, *key = NULL;
                int min_val, max_val;
                if (sscanf(msg, "add_alert %s %s %d %d", id, key, &min_val, &max_val) == 4){
                    for (int i = 0; i < config->max_sensors; i++) {
                        if (strcmp(sensor[i].data.chave, key) == 0) {
                            for (int j = 0; j < 4; j++){
                                 if (sensor[i].alerts[j].alert_id == NULL){
                                    sensor[i].alerts[j].alert_id = id;
                                    sensor[i].alerts[j].alert_flag = 1;
                                    sensor[i].alerts[j].alert_min = min_val;
                                    sensor[i].alerts[j].alert_max = max_val;
                                    break;
                                 }
                            }
                        }
                    }
                }
            }

            if(strncmp(msg,"remove_alert",strlen("remove_alert")) == 0){
                char *id = NULL;
                if(sscanf(msg,"remove_alert %s",id) == 1){
                     for (int i = 0; i < config->max_sensors; i++) {
                        for (int j = 0; j < 4; j++){
                            if (strcmp(sensor[i].alerts[j].alert_id, id) == 0) {
                                sensor[i].alerts[j].alert_flag = 0;
                                sensor[i].alerts[j].alert_id = NULL;
                                break;
                            }
                        }
                    }
                }
            }

            if(strncmp(msg,"list_alerts",strlen("list_alerts")) == 0){
                for (int i = 0; i < config->max_sensors; i++) {
                    for (int j = 0; j < 4; j++){
                        char* alert = sensor[i].alerts[j].alert_id;
                        if (alert != NULL) { 
                            printf("Alerta->%s",alert);
                        }
                    }
                }
            }
        }

        // Se a mensagem for um dado do sensor
        else{

            worker_sensor ws = create_worker_sensor(msg);

            if(count_key < config->max_keys){
                // Search for the sensor structure with the given key
                for(int i = 0; i < config->max_sensors; i++){
                    if(strcmp(sensor[i].id, ws.id) == 0){
                        // Found the sensor structure, now access its data fields
                        sensor_chave data = sensor[i].data;
                        if(strcmp(data.chave, ws.chave) == 0){
                            //Primeira vez a receber dados do sensor no sistema.
                            if(data.count == 0)count_key++;
                            data.last_value = ws.value;
                            data.avg = (data.last_value + data.min_value + data.max_value) / 3;
                            if(ws.value < data.min_value)data.min_value = ws.value;
                            if(ws.value > data.max_value)data.max_value = ws.value;
                            data.count++;
                        }
                        break;
                    }
                }

                sem_wait(array_sem);
                for (int i = 0; i < config->num_workers; i++) {
                    int worker_state = *(first_worker + i);
                    if (worker_state == 0) {
                        worker_state = 1;
                        break;
                    }
                }
                sem_post(array_sem);
                sem_post(worker_sem); 
            }
            else
                print("Limite de chaves em armazenamento no sistema excedido");   
        }
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