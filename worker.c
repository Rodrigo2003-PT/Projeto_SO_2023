//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

//TO-DO
//Process messages from user_console

#include "worker.h"

int sensor_exists = 0;
int chave_exists = 0;
int count_sensors = 0;

void worker_init(int* pipe_fd){
    while (running) {
        sensor_exists = 0;
        chave_exists = 0;
        count_sensors = 0;
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
                    for (int i = 0; i < config->max_keys; i++) {
                        if (strcmp(chave[i].chave, "") != 0 && strcmp(chave[i].chave, key) == 0) {
                            for (int j = 0; j < ALERTS_PER_SENSOR; j++){
                                 if (chave[i].alerts[j].alert_id == NULL){
                                    chave[i].alerts[j].pid = console_pid;
                                    chave[i].alerts[j].alert_id = id;
                                    chave[i].alerts[j].alert_flag = 1;
                                    chave[i].alerts[j].alert_min = min_val;
                                    chave[i].alerts[j].alert_max = max_val;
                                    break;
                                 }
                            }
                        chave_exists = 1;
                        break;
                        }
                    }
                    if(!chave_exists){
                        printf("Cannot add_alert to a non existing_chave\n");
                    }
                }
            }

            if(strncmp(msg,"remove_alert",strlen("remove_alert")) == 0){
                char id[MAX_KEY_SIZE];
                if(sscanf(msg,"remove_alert %s",id) == 1){
                    sem_wait(array_sem);
                    for (int i = 0; i < config->max_keys; i++) {
                        for (int j = 0; j < ALERTS_PER_SENSOR; j++){
                            if (strcmp(chave[i].chave, "") != 0 && chave[i].alerts[j].alert_id != NULL && strcmp(chave[i].alerts[j].alert_id, id) == 0) {
                                chave[i].alerts[j].pid = -1;
                                chave[i].alerts[j].alert_min = 0;
                                chave[i].alerts[j].alert_max = 0;
                                chave[i].alerts[j].alert_flag = 0;
                                chave[i].alerts[j].alert_id = NULL;
                                printf("alert_removed successfully\n");
                                chave_exists = 1;
                            }
                        }
                    }
                    if(!chave_exists){
                        printf("Cannot remove_alert from a non existing_chave\n");
                    }
                }
            }

            if(strncmp(msg,"list_alerts",strlen("list_alerts")) == 0){
                sem_wait(array_sem);
                for (int i = 0; i < config->max_keys; i++) {
                    if(strcmp(chave[i].chave, "") != 0){
                        for (int j = 0; j < ALERTS_PER_SENSOR; j++){
                            char* alert = chave[i].alerts[j].alert_id;
                            if (alert != NULL) { 
                                printf("Alerta->%s\n",alert);
                            }
                        }
                    }
                }
            }
        }

        // Se a mensagem for um dado do sensor
        else{
            worker_sensor ws = create_worker_sensor(msg);
            if(count_key != config->max_keys){
                // Search for the sensor structure with the given key
                sem_wait(array_sem);
                for(int i = 0; i < config->max_sensors; i++){
                    if(strcmp(sensor[i].id, "") != 0){
                        count_sensors++;
                        if(strcmp(sensor[i].id,ws.id) == 0){
                            // sensor já comunicou com o sistema
                            sensor_exists = 1;
                            for(int j = 0; j < config->max_keys; j++){
                                if(strcmp(chave[i].chave, "") != 0 && strcmp(chave[i].chave,ws.chave) == 0){
                                    //chave enviada existe no sistema -> atualização
                                    chave[i].last_value = ws.value;
                                    if(ws.value < chave[i].min_value)chave[i].min_value = ws.value;
                                    if(ws.value > chave[i].max_value)chave[i].max_value = ws.value;
                                    chave[i].avg = (chave[i].last_value + chave[i].min_value + chave[i].max_value) / 3;
                                    chave[i].count++;
                                    
                                    free(ws.id);
                                    free(ws.chave);
                                    chave_exists = 1;
                                    break;
                                }
                            }
                            if(!chave_exists){
                                for(int j = 0; j < config->max_keys; j++){
                                    if(strcmp(chave[i].chave, "") == 0){
                                        //chave slot available
                                        strcpy(chave[i].chave,ws.chave);
                                        chave[i].last_value = ws.value;
                                        chave[i].min_value = ws.value;
                                        chave[i].max_value = ws.value;
                                        chave[i].avg = (chave[i].last_value + chave[i].min_value + chave[i].max_value) / 3;
                                        
                                        free(ws.id);
                                        free(ws.chave);
                                        chave_exists = 1;
                                        count_key++;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                if(count_sensors == config->max_sensors){
                    printf("Error: maximum number of sensors reached\n");
                    printf("Mensagem descartada\n");
                }
                else if(!sensor_exists){
                    // comunicação proveniente de um novo sensor
                    for(int i = 0; i < config->max_sensors; i++){
                        // adicionar novo sensor ao sistema
                        if(strcmp(sensor[i].id, "") == 0){
                            strcpy(sensor[i].id, ws.id);
                            break;
                        }
                    }
                    // adicionar chave num slot disponivel
                    for(int j = 0; j < config->max_keys; j++){
                        if(strcmp(chave[j].chave, "") == 0){
                            //chave slot available
                            strcpy(chave[j].chave,ws.chave);
                            chave[j].last_value = ws.value;
                            chave[j].min_value = ws.value;
                            chave[j].max_value = ws.value;
                            chave[j].avg = (chave[j].last_value + chave[j].min_value + chave[j].max_value) / 3;
                            
                            free(ws.id);
                            free(ws.chave);
                            count_key++;
                            break;
                        }
                    }
                }
            }
            else{
                printf("Mensagem descartada");
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