//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

#include "alerts_watcher.h"

void alerts_watcher_init(){
    while(running) {
        sem_wait(alerts_sem);
        sem_wait(array_sem);
        for (int i = 0; i < config->max_sensors; i++) {
            for (int j = 0; j < ALERTS_PER_SENSOR; j++) {
                if (sensor[i].id != NULL && sensor[i].alerts[j].alert_flag == 1) {
                    printf("Memory location: %p\n",(void*)&sensor[i].id);
                    printf("sensor[i].id: %s\n",sensor[i].id);
                    printf("sensor[i].dat.chave: %s\n",sensor[i].data.chave);
                    printf("sensor[i].dat.min: %d\n",sensor[i].data.min_value);
                    if (sensor[i].data.last_value < sensor[i].alerts[j].alert_min || sensor[i].data.last_value > sensor[i].alerts[j].alert_max) {
                        alert_msg msg;
                        msg.mtype = sensor[i].alerts[j].pid;
                        strcpy(msg.sensor_id, sensor[i].id);
                        msg.triggered_value = sensor[i].data.last_value;
                        if (msgsnd(msq_id, &msg, sizeof(alert_msg), 0) == -1)
                            perror("msgsnd failed");
                        char buf[MESSAGE_SIZE];
                        sprintf(buf,"Alert triggered for sensor %s: value = %d\n", msg.sensor_id, msg.triggered_value);
                        printf("%s",buf);
                        print(buf);
                    }
                }
            }
        }
        sem_post(array_sem);
    }
}