//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

#include "alerts_watcher.h"

void alerts_watcher_init(){
      while(1) {
        for (int i = 0; i < config->max_sensors; i++) {
            for (int j = 0; j < 4; j++) {
                if (sensor[i].alerts[j].alert_flag == 1) {
                    if (sensor[i].data.last_value < sensor[i].alerts[j].alert_min || sensor[i].data.last_value > sensor[i].alerts[j].alert_max) {
                        int msg_type = sensor[i].alerts[j].pid;
                        alert_msg msg;
                        msg.sensor_id = sensor[i].id;
                        msg.triggered_value = sensor[i].data.last_value;

                        if (msgsnd(msq_id, &msg, sizeof(alert_msg), msg_type) == -1)
                            perror("msgsnd failed");
                        printf("Alert triggered for sensor %s: value = %d\n", sensor[i].id, sensor[i].data.last_value);
                    }
                }
            }
        }
    }
}