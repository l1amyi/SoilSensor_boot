#include "sensor.h"
#include "gpio.h"
#include "usart.h"


#include "gd32f30x_gpio.h"

#include <stdio.h>
#include "rtc.h"

#include "record.h"

#define SENSOR_COUNT 10

static int sensor_count = 10;

enum sensor_state
{
    SENSOR_STATE_IDLE,
    SENSOR_STATE_POWER_ON,
    SENSOR_STATE_HAS_RESPONSE,
    SENSOR_STATE_RETRY
};

static int time_interval = 60;
static sensor_t sensors[SENSOR_COUNT];

void sensor_switch(int idx, int s)
{
//    printf("sensor switch %d %d\n", idx, s);
    
    if (idx < 0 || idx > SENSOR_COUNT - 1)
    {
        return;
    }

    if (s)
    {
        gpio_bit_set(GPIOC, GPIO_PIN_3);
        gpio_bit_set(CHS_PWR_PORT[idx], CHS_PWR_PIN[idx]); // set CHx_PWR
    }
    else
    {
        gpio_bit_reset(GPIOC, GPIO_PIN_3);
        gpio_bit_reset(CHS_PWR_PORT[idx], CHS_PWR_PIN[idx]); // reset CHx_PWR
    }
}

// crc16 modbus
uint16_t crc_cal(uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

#define DE() gpio_bit_set(GPIOB, GPIO_PIN_6);gpio_bit_set(GPIOB, GPIO_PIN_7); // set DE
#define RE() gpio_bit_reset(GPIOB, GPIO_PIN_7);gpio_bit_reset(GPIOB, GPIO_PIN_6); // set RE


// 读取传感器的温度和湿度
int sensor_read_temperature_humidity(uint16_t *temperature, uint16_t *humidity)
{
    uint8_t tx_data[8] = {0x01, 0x03, 0x00, 0x04, 0x00, 0x02, 0x85, 0xca};
    uint8_t rx_data[9] = {0};
    
    uint16_t crc = crc_cal(tx_data, 6);
    tx_data[6] = crc & 0xff;
    tx_data[7] = (crc >> 8) & 0xff;

    uart2_transmit(tx_data, sizeof(tx_data));
    int r = uart2_receiver(rx_data, 9, 800);

    if (r)
    {
        if (rx_data[0] == 0x01 && rx_data[1] == 0x03 && rx_data[2] == 0x04)
        {
            uint16_t crc = crc_cal(rx_data, 7);
            if ((rx_data[8] << 8 | rx_data[7]) == crc)
            {
                *temperature = (rx_data[3] << 8) | rx_data[4];
                *humidity = (rx_data[5] << 8) | rx_data[6];
                return 1;
            }
        }
    }

    return 0;
}

// 读取传感器的电导率
int sensor_read_conductivity(uint16_t *conductivity)
{
    uint8_t tx_data[8] = {0x01, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B};
    uint8_t rx_data[7] = {0};
    
    uint16_t crc = crc_cal(tx_data, 6);
    
    tx_data[6] = crc & 0xff;
    tx_data[7] = (crc >> 8) & 0xff;

    uart2_transmit(tx_data, sizeof(tx_data));
    int r = uart2_receiver(rx_data, 7, 800);
    

    if (r)
    {
        if (rx_data[0] == 0x01 && rx_data[1] == 0x03 && rx_data[2] == 0x02)
        {
            uint16_t crc = crc_cal(rx_data, 5);
            if ((rx_data[6] << 8 | rx_data[5]) == crc)
            {
                *conductivity = (rx_data[3] << 8) | rx_data[4];
                return 1;
            }
        }
    }

    return 0;
}

int sensor_readall(uint16_t *temperature, uint16_t *humidity, uint16_t *conductivity)
{
    uint8_t tx_data[8] = {0x01, 0x03, 0x00, 0x04, 0x00, 0x03, 0x00, 0x00};
    uint8_t rx_data[11] = {0};
    
    uint16_t crc = crc_cal(tx_data, 6);
    tx_data[6] = crc & 0xff;
    tx_data[7] = (crc >> 8) & 0xff;

    uart2_transmit(tx_data, sizeof(tx_data));
    int r = uart2_receiver(rx_data, 11, 800);

    if (r)
    {
//        for (int i = 0; i < 11; i++)
//        {
//            printf("%02X ", rx_data[i]);
//        }
//        printf("\n");
      
        if (rx_data[0] == 0x01 && rx_data[1] == 0x03 && rx_data[2] == 0x06)
        {
            uint16_t crc = crc_cal(rx_data, 9);
            if ((rx_data[10] << 8 | rx_data[9]) == crc)
            {
                *humidity = (rx_data[3] << 8) | rx_data[4];
                *temperature = (rx_data[5] << 8) | rx_data[6];
                *conductivity = (rx_data[7] << 8) | rx_data[8];
                return 1;
            }
        }
    }

    return 0;
}

void sensor_init(int interval, int sensor_num)
{
    sensor_count = sensor_num;
    time_interval = interval;

    if (sensor_count > SENSOR_COUNT)
    {
        sensor_count = SENSOR_COUNT;
    }
  
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        sensors[i].id = i;
        sensors[i].timestamp_last = record_last_timestamp(i);
        sensors[i].temperature = 0;
        sensors[i].humidity = 0;
        sensors[i].conductivity = 0;
        sensors[i].state = SENSOR_STATE_IDLE;
        sensors[i].state_start_tick = rtc_getval();
        sensors[i].record_count = record_count(i);
        sensors[i].timestamp_start = record_start_timestamp(i);
        sensors[i].is_activate  = 0;
    }


    
    // 初始化传感器电源
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        sensor_switch(i, 0); // reset CHx_PWR
    }
    
}

void sensor_process()
{   
    int oldest_id = 0;
    int processing_id = -1;
    
    for (int i = 0; i < sensor_count; i++)
    {
        if (sensors[i].state != SENSOR_STATE_IDLE)
        {
            processing_id = i;
        }
    }

    if (processing_id == -1)
    {
        for (int i = 0; i < sensor_count; i++)
        {
            if (sensors[i].timestamp_last < sensors[oldest_id].timestamp_last)
            {
                oldest_id = i;
            }
        }
        
        processing_id = oldest_id;
    }
    
    if (sensors[processing_id].state == SENSOR_STATE_IDLE)
    {
        if (sensors[processing_id].timestamp_last + time_interval - 3 <= rtc_getval())
        {
            sensors[processing_id].state = SENSOR_STATE_POWER_ON;
            sensors[processing_id].state_start_tick = rtc_getval();
            sensor_switch(processing_id, 1); // set CHx_PWR
            
            return; // 只处理一个传感器
        }
        else
        {
            return; // 继续检查下一个传感器
        }
    }
    else if (sensors[processing_id].state == SENSOR_STATE_POWER_ON)
    {
        if (sensors[processing_id].state_start_tick + 1 < rtc_getval())
        {
            uint16_t temperature = 0, humidity = 0, conductivity = 0;
            if (sensor_readall(&temperature, &humidity, &conductivity))
            {
                sensors[processing_id].state = SENSOR_STATE_HAS_RESPONSE;
                sensors[processing_id].state_start_tick = rtc_getval();
            }
            else
            {
                sensors[processing_id].state = SENSOR_STATE_IDLE;
                sensors[processing_id].state_start_tick = rtc_getval();
                sensors[processing_id].timestamp_last = rtc_getval();
                sensors[processing_id].is_activate = 0;
                sensor_switch(processing_id, 0); // reset CHx_PWR
            }
        }
        return;
    }
    else if (sensors[processing_id].state == SENSOR_STATE_HAS_RESPONSE)
    {
        if (sensors[processing_id].state_start_tick + 0 < rtc_getval())
        {
            uint16_t temperature = 0, humidity = 0, conductivity = 0;
            if (sensor_readall(&temperature, &humidity, &conductivity))
            {
                sensors[processing_id].temperature = temperature;
                sensors[processing_id].humidity = humidity;
                sensors[processing_id].conductivity = conductivity;
                
                sensors[processing_id].state = SENSOR_STATE_IDLE;
                sensors[processing_id].state_start_tick = rtc_getval();
                sensors[processing_id].timestamp_last = rtc_getval();
                sensors[processing_id].is_activate = 1;
              
                sensor_switch(processing_id, 0); // reset CHx_PWR
                record_add(sensors[processing_id].timestamp_last, processing_id, temperature, humidity, conductivity);
            }
            else
            {
                sensors[processing_id].state = SENSOR_STATE_IDLE;
                sensors[processing_id].state_start_tick = rtc_getval();
                sensors[processing_id].timestamp_last = rtc_getval();
                sensors[processing_id].is_activate = 0;
                sensor_switch(processing_id, 0); // reset CHx_PWR
            }
        }
        
        return;
    }
}

const sensor_t* sensor_get(int id)
{
    if (id < 0 || id >= SENSOR_COUNT)
        return 0;
    return &sensors[id];
}

int sensor_get_count()
{
    return sensor_count;
}

uint32_t sensor_all_last()
{
    uint32_t timestamp = 0;
  
    for (int i = 0; i < sensor_count; i++)
    {
        if (timestamp < sensors[i].timestamp_last)
        {
            timestamp = sensors[i].timestamp_last;
        }
    }

    return timestamp;
}

void sensor_set_num(int num)
{
    sensor_count = num;
}

void sensor_set_interval(int i)
{
    time_interval = i;
}

