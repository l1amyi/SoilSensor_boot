#ifndef _SETTING_H_
#define _SETTING_H_
#include <stdint.h>

typedef struct
{
    uint32_t sn;
    uint16_t time_interval; // 传感器采集间隔，单位秒
    uint8_t sensor_num;
    uint8_t sensor_model[10];
} Setting_t;


void setting_init(void);
void setting_save(void);

uint32_t setting_get_sn(void);
void setting_set_sn(uint32_t sn);

uint16_t setting_get_time_interval(void);
void setting_set_time_interval(uint16_t interval);

uint8_t setting_get_sensor_model(int idx);
void setting_set_sensor_model(int idx, uint8_t model);

uint8_t setting_get_sensor_num(void);
void setting_set_sensor_num(uint8_t num);

void setting_reset(void);

#endif

