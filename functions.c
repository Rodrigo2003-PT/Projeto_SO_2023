#include "functions.h"
#include "shared_mem.h"
#include <time.h>

//Config file gesture
int *read_config_file(){

    /* Config int[] format
    0 - queue_slot_number (must be >= 1);
    1 - num_workers (must be >= 1);
    2 - max_keys (must be >= 1);
    3 - max_sensors (must be >= 1);
    4 - max_alerts (must be >= 0);
    */

    FILE *conf_file = fopen("config.txt", "r");

    if(conf_file == NULL)
        return NULL;
    
    
    int *configs = malloc(sizeof(int) * number_of_configs);
    int current_index = 0;

    char *line = malloc(sizeof(char) * line_lenght);

    while(fgets(line,line_lenght, conf_file) != NULL){
        int value = strtol(line, NULL, 10);

        switch (current_index) {
            case 0: // queue_slot_number (must be >= 1)
                if (value < 1) {
                    fclose(conf_file);
                    free(line);
                    free(configs);
                    return NULL;
                }
                break;

            case 1: // num_workers (must be >= 1)
                if (value < 1) {
                    fclose(conf_file);
                    free(line);
                    free(configs);
                    return NULL;
                }
                break;

            case 2: // max_keys (must be >= 1)
                if (value < 1) {
                    fclose(conf_file);
                    free(line);
                    free(configs);
                    return NULL;
                }
                break;

            case 3: // max_sensors (must be >= 1)
                if (value < 1) {
                    fclose(conf_file);
                    free(line);
                    free(configs);
                    return NULL;
                }
                break;

            case 4: // max_alerts (must be >= 0)
                if (value < 0) {
                    fclose(conf_file);
                    free(line);
                    free(configs);
                    return NULL;
                }
                break;

            default: // too many lines in file
                fclose(conf_file);
                free(line);
                free(configs);
                return NULL;
        }

        configs[current_index++] = value;
    }

    if (current_index != number_of_configs) { // too few lines in file
        fclose(conf_file);
        free(line);
        free(configs);
        return NULL;
    }

    free(line);
    fclose(conf_file);
    return configs;
}


void process_config_file(int *configs){
  config = malloc(sizeof(config_struct));
  config->queue_slot_number = configs[0];
  config->num_workers = configs[1];
  config->max_keys = configs[2];
  config->max_sensors= configs[3];
  config->max_alerts = configs[4];
}

//Debug
void print_config_file(){
    printf("%d\n", config->queue_slot_number);
    printf("%d\n", config->num_workers);
    printf("%d\n", config->max_keys);
    printf("%d\n", config->max_sensors);
    printf("%d\n", config->max_alerts);
}