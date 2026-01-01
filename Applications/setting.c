#include "setting.h"

#include "gd32f30x_fmc.h"

#include <string.h>

Setting_t setting = {0};

#define FMC_ADDRESS 0x800f800

void fmc_clear()
{
    FlagStatus status;

    status = fmc_flag_get(FMC_FLAG_BANK0_BUSY);
    if (status == SET)
    {
        fmc_flag_clear(FMC_FLAG_BANK0_BUSY);
    }

    status = fmc_flag_get(FMC_FLAG_BANK0_PGERR);
    if (status == SET)
    {
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    }

    status = fmc_flag_get(FMC_FLAG_BANK0_WPERR);
    if (status == SET)
    {
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    }

    status = fmc_flag_get(FMC_FLAG_BANK0_END);
    if (status == SET)
    {
        fmc_flag_clear(FMC_FLAG_BANK0_END);
    }
}

void setting_init()
{
    fmc_clear();

    if (*(uint32_t*)FMC_ADDRESS == 0xFFFFFFFF)
    {
        // 如果没有设置过，则使用默认值
        setting.sn = 1;
        setting.time_interval = 60; // 默认间隔60秒
        setting.sensor_num = 10;

        memset(setting.sensor_model, 1, 10);
        
        setting_save();
    }
    else
    {
        // 从FMC读取设置
        memcpy(&setting, (void*)FMC_ADDRESS, sizeof(Setting_t));
        
    }

    fmc_clear();
}

void setting_save()
{
    fmc_clear();

    fmc_unlock();
    fmc_page_erase(FMC_ADDRESS);
    fmc_lock();

    fmc_unlock();

    for (int i = 0; i < sizeof(Setting_t) / 4 + 1; i++)
    {
        uint32_t *data = (uint32_t*)((uint8_t*)&setting + i * 4);
        fmc_word_program(FMC_ADDRESS + i * 4, *data);
    }

    fmc_lock();

    fmc_clear();
}

uint32_t setting_get_sn()
{
    return setting.sn;
}

void setting_set_sn(uint32_t sn)
{
    setting.sn = sn;
}

uint16_t setting_get_time_interval()
{
    return setting.time_interval;
}

void setting_set_time_interval(uint16_t interval)
{
    setting.time_interval = interval;
}

uint8_t setting_get_sensor_model(int idx)
{
    if (idx >= 10 || idx < 0)
        return 0;
    return setting.sensor_model[idx];
}

void setting_set_sensor_model(int idx, uint8_t model)
{
    if (idx >= 10 || idx < 0)
        return;
    setting.sensor_model[idx] = model;
}

uint8_t setting_get_sensor_num()
{
    return setting.sensor_num;
}

void setting_set_sensor_num(uint8_t num)
{
    setting.sensor_num = num;
}

void setting_reset()
{
    setting.sn = 1;
    setting.time_interval = 60; // 默认间隔60秒
    setting.sensor_num = 10;
    memset(setting.sensor_model, 1, 10);
    
    setting_save();
}
