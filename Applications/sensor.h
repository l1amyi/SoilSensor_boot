#ifndef _SENSOR_H_
#define _SENSOR_H_
#include <stdint.h>

typedef struct
{
    uint8_t id;
    uint32_t timestamp_start;
    uint32_t timestamp_last;
    uint16_t temperature;
    uint16_t humidity;
    uint16_t conductivity;
    uint8_t state;
    uint32_t state_start_tick;
    uint32_t record_count;
    uint8_t is_activate;
}sensor_t;


void sensor_init(int interval, int sensor_num);
void sensor_process(void);

const sensor_t* sensor_get(int id);

int sensor_get_count(void);

uint32_t sensor_all_last(void);

void sensor_set_num(int num);
void sensor_set_interval(int i);

#endif
