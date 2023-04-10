//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

//TO-DO
//Process messages from user_console

#include "worker.h"

void worker_init(char *msg){

    char *result = strchr(msg, '#');
    
    // Se a mensagem for um dado da user_console
    if(result == NULL){

    }
     // Se a mensagem for um dado do sensor
    else{

        worker_sensor ws = create_worker_sensor(msg);

        if(count_key < config->max_keys){

            // Search for the sensor structure with the given key
            for(int i = 0; i < config->max_sensors; i++){
                if(strcmp(sensor[i].id, ws.id) == 0){
                    // Found the sensor structure, now access its data fields
                    sensor_chave* data = sensor[i].data;

                    for(int j = 0; j < 6; j++){
                        if(strcmp(data[j].chave, ws.chave) == 0){
                             //Primeira vez a receber dados do sensor no sistema.
                            if(data[j].count == 0)count_key++;

                            data[j].last_value = ws.value;
                            data[j].avg = (data[j].last_value + data[j].min_value + data[j].max_value) / 3;
                            if(ws.value < data[j].min_value)data[j].min_value = ws.value;
                            if(ws.value > data[j].max_value)data[j].max_value = ws.value;
                            data[j].count++;
                            break;
                        }
                    }

                    break;
                }
            }
        }
        else
            print("Limite de chaves em armazenamento no sistema excedido");   
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