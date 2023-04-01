#include "worker.h"

void worker_init(worker_message *msg){

    // Se a mensagem for um dado do sensor
    if (msg->sens.chave) {

        if(count_key < config->max_keys){

            //Primeira vez a receber dados do sensor no sistema.
            if(msg->sens.data->count == 0){
                count_key++;
            }

            for (int i = 0; i < config->max_sensors; i++) {

                if (strcmp(sensor[i].chave, msg->sens.chave) == 0) {

                    sensor[i].data->last_value = msg->sens.data->last_value;

                    if (msg->sens.data->min_value < sensor[i].data->min_value) {
                        sensor[i].data->min_value = msg->sens.data->min_value;
                    }

                    if (msg->sens.data->max_value > sensor[i].data->max_value) {
                        sensor[i].data->max_value = msg->sens.data->max_value;
                    }

                    sensor[i].data->avg = (msg->sens.data->last_value + msg->sens.data->min_value + msg->sens.data->max_value) / 3;
                    
                    sensor[i].data->count++;

                    break;
                } 

            }
        }

        else
            print("Limite de chaves em armazenamento no sistema excedido");   
    }

    else {
       
    }
}