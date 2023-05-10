//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

#include "alerts_watcher.h"

void alerts_watcher_init(){
    while(running) {
        sem_wait(alerts_sem);
        sem_wait(array_sem);
        for (int i = 0; i < config->max_keys; i++) {
            for (int j = 0; j < ALERTS_PER_SENSOR; j++) {
                if (strcmp(chave[i].chave, "") != 0 && chave[i].alerts[j].alert_flag == 1) {
                    if (chave[i].last_value < chave[i].alerts[j].alert_min || chave[i].last_value > chave[i].alerts[j].alert_max) {
                        alert_msg msg;
                        msg.mtype = chave[i].alerts[j].pid;
                        strcpy(msg.key, chave[i].chave);
                        msg.triggered_value = chave[i].last_value;
                        if (msgsnd(msq_id, &msg, sizeof(alert_msg)-sizeof(long), 0) == -1)
                            print("ALERTS_WATCHER -> FAILED SENDING TO MSG_QUEUE\n");
                        char buf[MESSAGE_SIZE];
                        sprintf(buf,"ALERT TRIGGERED %s -> VALUE = %d\n", msg.key, msg.triggered_value);
                        print(buf);
                    }
                }
            }
        }
        sem_post(array_sem);
    }
}