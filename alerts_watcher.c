//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

#include "alerts_watcher.h"

void alerts_watcher_init(){
    while(running) {
        for (int i = 0; i < config->max_sensors; i++) {
            for (int j = 0; j < ALERTS_PER_SENSOR; j++) {
                if (sensor[i].id != NULL && sensor[i].alerts[j].alert_flag == 1) {
                    if (sensor[i].data.last_value < sensor[i].alerts[j].alert_min || sensor[i].data.last_value > sensor[i].alerts[j].alert_max) {
                        int msg_type = sensor[i].alerts[j].pid;
                        alert_msg msg;
                        msg.sensor_id = strdup(sensor[i].id);
                        msg.triggered_value = sensor[i].data.last_value;
                        if (msgsnd(msq_id, &msg, sizeof(alert_msg), msg_type) == -1)
                            perror("msgsnd failed");
                        char buf[MESSAGE_SIZE];
                        sprintf(buf,"Alert triggered for sensor %s: value = %d\n", msg.sensor_id, msg.triggered_value);
                        free(msg.sensor_id);
                        printf("%s",buf);
                        print(buf);
                    }
                }
            }
        }
    }
}