#include "record.h"
#include <stdio.h>
#include <string.h>

#include "flash.h"


#define BLOCK_ALL_COUNT 512

typedef struct
{
    uint8_t chip;
    uint16_t block;
    uint32_t start_timestamp;
    uint32_t last_timestamp;
    uint8_t sensor_id;
    uint32_t record_count;
    uint8_t is_full; // 0 or 1
} Block;

Block blocks[BLOCK_ALL_COUNT];


#define RECORD_LEN (4+1+2+2+2+1)

static uint8_t tx_buffer[1024] = {0};
static uint8_t rx_buffer[1024] = {0};

static int BLOCK_COUNT_ALL_CHIP = 0;

Flash flash[2] = {0};

void record_init()
{
    flash_get(0, &flash[0]);
    flash_get(1, &flash[1]);
    
    if (flash[0].is_active)
    {
        BLOCK_COUNT_ALL_CHIP += flash[0].block_per_chip;
    }

    if (flash[1].is_active)
    {
        BLOCK_COUNT_ALL_CHIP += flash[1].block_per_chip;
    }

    if (flash[0].is_active)
    {
        for (int i = 0; i < flash[0].block_per_chip; i++) {
            flash_read(0, i * flash[0].block_size, RECORD_LEN, rx_buffer);

            if (rx_buffer[RECORD_LEN - 1] == 0x55) 
            {
                blocks[i].start_timestamp = (rx_buffer[0] << 24) | (rx_buffer[1] << 16) | (rx_buffer[2] << 8) | rx_buffer[3];
                blocks[i].sensor_id = rx_buffer[4];
                blocks[i].chip = 0;
                blocks[i].block = i;
                
            }
            else
            {
                blocks[i].start_timestamp = 0xffffffff;
                blocks[i].sensor_id = 0xff;
                blocks[i].chip = 0;
                blocks[i].block = i;
            }

            
        }
    }

    if (flash[1].is_active)
    {
        for (int i = 0; i < flash[1].block_per_chip; i++) {
            flash_read(1, i * flash[1].block_size, RECORD_LEN, rx_buffer);

            int block_i = i + flash[0].block_per_chip;

            if (rx_buffer[RECORD_LEN - 1] == 0x55) 
            {
                blocks[block_i].start_timestamp = (rx_buffer[0] << 24) | (rx_buffer[1] << 16) | (rx_buffer[2] << 8) | rx_buffer[3];
                blocks[block_i].sensor_id = rx_buffer[4];
                blocks[block_i].chip = 1;
                blocks[block_i].block = i;
            }
            else
            {
                blocks[block_i].start_timestamp = 0xffffffff;
                blocks[block_i].last_timestamp = 0xffffffff;
                blocks[block_i].sensor_id = 0xff;
                blocks[block_i].chip = 1;
                blocks[block_i].block = i;
            }
        }
    }


    for (int i = 0; i < BLOCK_COUNT_ALL_CHIP; i++)
    {
        if (blocks[i].start_timestamp == 0xffffffff)
        {
            blocks[i].record_count = 0;
            blocks[i].is_full = 0;
        } 
        else
        {
            int sector_index = 0;
            int page_index = 0;
            int record_index = 0;
            
            for (int j = flash[blocks[i].chip].sector_per_block - 1; j >= 0; j--) {
                uint32_t sector_address = blocks[i].block * flash[blocks[i].chip].block_size + j * flash[blocks[i].chip].sector_size;
                flash_read(blocks[i].chip, sector_address, RECORD_LEN, rx_buffer);
                
                if (rx_buffer[RECORD_LEN - 1] == 0x55) 
                {
                    sector_index = j;
                    for (int k = flash[blocks[i].chip].page_per_sector - 1; k >= 0; k--) {
                        uint32_t page_address = sector_address + k * flash[blocks[i].chip].page_size;
                        flash_read(blocks[i].chip, page_address, RECORD_LEN, rx_buffer);
                        
                        if (rx_buffer[RECORD_LEN - 1] == 0x55) 
                        {
                            page_index = k;
                            int l = 1;
                            for (; l < (flash[blocks[i].chip].page_size / RECORD_LEN); l++) {
                                flash_read(blocks[i].chip, page_address + l * RECORD_LEN, RECORD_LEN, rx_buffer);

                                if (rx_buffer[RECORD_LEN - 1] != 0x55) {
                                    break;
                                }

                                blocks[i].last_timestamp = (rx_buffer[0] << 24) | (rx_buffer[1] << 16) | (rx_buffer[2] << 8) | rx_buffer[3];
                            }
                            record_index = l;
                            break;
                        }
                    }
                    break;
                }
            }

            int record_count_per_page = flash[blocks[i].chip].page_size / RECORD_LEN;

            blocks[i].record_count = sector_index * flash[blocks[i].chip].page_per_sector * record_count_per_page +
                                     page_index * record_count_per_page + record_index;

            if (blocks[i].record_count == 1)
            {
                blocks[i].last_timestamp = blocks[i].start_timestamp;
            }
            
            if (sector_index == flash[blocks[i].chip].sector_per_block - 1 &&
                page_index == flash[blocks[i].chip].page_per_sector - 1 &&
                record_index == (flash[blocks[i].chip].page_size / RECORD_LEN))
            {
                blocks[i].is_full = 1; // Block is full
            }
            else
            {
                blocks[i].is_full = 0; // Block is not full
            }
        }
    }
}


int record_add(uint32_t timestamp, uint8_t id, uint16_t temperature, uint16_t humidity, uint16_t conductivity)
{
    for (int i = 0; i < BLOCK_COUNT_ALL_CHIP; i++)
    {
        // 找到一个未满的块
        if (blocks[i].sensor_id == id && blocks[i].is_full == 0)
        {
            int record_count_per_page = flash[blocks[i].chip].page_size / RECORD_LEN;
            int sector_index = blocks[i].record_count / (flash[blocks[i].chip].page_per_sector * record_count_per_page);
            int page_index = (blocks[i].record_count % (flash[blocks[i].chip].page_per_sector * record_count_per_page)) / record_count_per_page;
            int record_index = blocks[i].record_count % record_count_per_page;

            uint32_t sector_address = blocks[i].block * flash[blocks[i].chip].block_size + sector_index * flash[blocks[i].chip].sector_size;
            uint32_t page_address = sector_address + page_index * flash[blocks[i].chip].page_size;
            

            // 准备数据
            tx_buffer[0] = (timestamp >> 24) & 0xFF;
            tx_buffer[1] = (timestamp >> 16) & 0xFF;
            tx_buffer[2] = (timestamp >> 8) & 0xFF;
            tx_buffer[3] = timestamp & 0xFF;
            tx_buffer[4] = id;
            tx_buffer[5] = (temperature >> 8) & 0xFF;
            tx_buffer[6] = temperature & 0xFF;
            tx_buffer[7] = (humidity >> 8) & 0xFF;
            tx_buffer[8] = humidity & 0xFF;
            tx_buffer[9] = (conductivity >> 8) & 0xFF;
            tx_buffer[10] = conductivity & 0xFF;
            tx_buffer[11] = 0x55; // 标记为有效记录

            flash_write_enable(blocks[i].chip);
//            printf("block %d, sector %d, page %d, record index %d\n", i, sector_index, page_index, record_index);
//            printf("write address %d\n", page_address + record_index * RECORD_LEN);
            if (!flash_write(blocks[i].chip, page_address + record_index * RECORD_LEN, RECORD_LEN, tx_buffer))
            {
                //printf("flash write err\n");
            }
            
            blocks[i].record_count++;
            blocks[i].last_timestamp = timestamp;


            if (blocks[i].record_count >= flash[blocks[i].chip].sector_per_block * flash[blocks[i].chip].page_per_sector * (flash[blocks[i].chip].page_size / RECORD_LEN)) 
            {
//                printf("chip %d, sector %d, page %d, page_size %d, record_len %d\n", blocks[i].chip, flash[blocks[i].chip].sector_per_block, flash[blocks[i].chip].page_per_sector, flash[blocks[i].chip].page_size, RECORD_LEN);
//                printf("record count %d max %d\n", blocks[i].record_count, flash[blocks[i].chip].sector_per_block * flash[blocks[i].chip].page_per_sector * (flash[blocks[i].chip].page_size / RECORD_LEN));
//                printf("set block %d full\n", i);
                blocks[i].is_full = 1; // 如果记录数超过块的容量，则标记为满
            }

            return 1; // 成功添加记录
        }
    }
    
//    printf("block full\n");

    // 找到一个空闲的块
    for (int i = 0; i < BLOCK_COUNT_ALL_CHIP; i++)
    {
        if (blocks[i].start_timestamp == 0xffffffff) // 找到一个空块
        {
            blocks[i].start_timestamp = timestamp;
            blocks[i].last_timestamp = timestamp;
            blocks[i].sensor_id = id;
            blocks[i].chip = (i < flash[0].block_per_chip) ? 0 : 1;
            blocks[i].block = i % flash[0].block_per_chip;
            blocks[i].record_count = 1; // 初始化记录数为1
            blocks[i].is_full = 0; // 初始状态为未满

            int sector_index = 0;
            int page_index = 0;
            int record_index = 0;

            uint32_t sector_address = blocks[i].block * flash[blocks[i].chip].block_size + sector_index * flash[blocks[i].chip].sector_size;
            uint32_t page_address = sector_address + page_index * flash[blocks[i].chip].page_size;

            // 准备数据
            tx_buffer[0] = (timestamp >> 24) & 0xFF;
            tx_buffer[1] = (timestamp >> 16) & 0xFF;
            tx_buffer[2] = (timestamp >> 8) & 0xFF;
            tx_buffer[3] = timestamp & 0xFF;
            tx_buffer[4] = id;
            tx_buffer[5] = (temperature >> 8) & 0xFF;
            tx_buffer[6] = temperature & 0xFF;
            tx_buffer[7] = (humidity >> 8) & 0xFF;
            tx_buffer[8] = humidity & 0xFF;
            tx_buffer[9] = (conductivity >> 8) & 0xFF;
            tx_buffer[10] = conductivity & 0xFF;
            tx_buffer[11] = 0x55; // 标记为有效记录
            
            flash_write_enable(blocks[i].chip);
//            printf("write address %d\n", page_address + record_index * RECORD_LEN);
            if (!flash_write(blocks[i].chip, page_address + record_index * RECORD_LEN, RECORD_LEN, tx_buffer))
            {
//                printf("flash write err\n");
            }
            
            
            return 1; // 成功添加记录
        }
    }

    return 0; // 没有可用的块，添加记录失败
}

int record_query_by_time(uint32_t time, uint8_t id, uint32_t *timestamp, uint16_t *temperature, uint16_t *humidity, uint16_t *conductivity,
                        int *block_i, int *sector_i, int *page_i, int *record_i)
{
    for (int i = 0; i < BLOCK_COUNT_ALL_CHIP; i++)
    {
        if (blocks[i].sensor_id == id && blocks[i].record_count > 0)
        {
            // 找到time所在的block
            if (blocks[i].start_timestamp <= time && blocks[i].last_timestamp >= time)
            {
                uint32_t timestamp_start = blocks[i].start_timestamp;
                uint32_t timestamp_start_next = 0;
                for (int j = 0; j < flash[blocks[i].chip].sector_per_block; j++)
                {

                    uint32_t sector_address_next = 0;

                    if (j == flash[blocks[i].chip].sector_per_block - 1) 
                    {
                        // 如果是最后一个sector，设置下一个sector的起始地址为块的结束地址
                        timestamp_start_next = blocks[i].last_timestamp + 1;
                        sector_address_next = blocks[i].block * flash[blocks[i].chip].block_size + (j + 1) * flash[blocks[i].chip].sector_size;
                    }
                    else
                    {
                        sector_address_next = blocks[i].block * flash[blocks[i].chip].block_size + (j + 1) * flash[blocks[i].chip].sector_size;

                        flash_read(blocks[i].chip, sector_address_next, RECORD_LEN, rx_buffer);

                        
                        if (rx_buffer[RECORD_LEN - 1] == 0x55) 
                        {
                            timestamp_start_next = (rx_buffer[0] << 24) | (rx_buffer[1] << 16) | (rx_buffer[2] << 8) | rx_buffer[3];
                        }
                        else
                        {
                            timestamp_start_next = blocks[i].last_timestamp; // 如果没有找到下一个时间戳，则设置为最大值
                        }
                    }

                    if (time >= timestamp_start && time < timestamp_start_next)
                    {
                        // 找到对应的sector
                        int sector_index = j;
                        int page_index = 0;
                        int record_index = 0;

                        uint32_t sector_timestamp_next = timestamp_start_next;
                        uint32_t page_address_next;

                        for (int k = 0; k < flash[blocks[i].chip].page_per_sector; k++)
                        {
                            if (k == flash[blocks[i].chip].page_per_sector - 1) 
                            {
                                // 如果是最后一个page，
                                timestamp_start_next = sector_timestamp_next;
                                page_address_next = sector_address_next - flash[blocks[i].chip].sector_size + (k + 1) * flash[blocks[i].chip].page_size;
                            }
                            else
                            {
                                page_address_next = sector_address_next - flash[blocks[i].chip].sector_size + (k + 1) * flash[blocks[i].chip].page_size;

                                flash_read(blocks[i].chip, page_address_next, RECORD_LEN, rx_buffer);

                                if (rx_buffer[RECORD_LEN - 1] == 0x55) 
                                {
                                    // 检查记录时间
                                    timestamp_start_next = (rx_buffer[0] << 24) | (rx_buffer[1] << 16) | (rx_buffer[2] << 8) | rx_buffer[3];
                                }
                                else
                                {
                                    timestamp_start_next = 0xffffffff; // 如果没有找到下一个时间戳，则设置为最大值
                                }
                            }

                            

                            if (time >= timestamp_start && time <= timestamp_start_next)
                            {
                                page_index = k;
                                for (int l = 0; l < (flash[blocks[i].chip].page_size / RECORD_LEN) - 1; l++)
                                {
                                    flash_read(blocks[i].chip, page_address_next - flash[blocks[i].chip].page_size + (l + 1) * RECORD_LEN, RECORD_LEN, rx_buffer);

                                    if (rx_buffer[RECORD_LEN - 1] == 0x55)
                                    {
                                        timestamp_start_next = (rx_buffer[0] << 24) | (rx_buffer[1] << 16) | (rx_buffer[2] << 8) | rx_buffer[3];


                                        if (time >= timestamp_start && time <= timestamp_start_next)
                                        {
                                            record_index = l + 1; // 找到对应的记录索引

                                            // 找到对应的记录
                                            *timestamp = timestamp_start_next;
                                            *temperature = (rx_buffer[5] << 8) | rx_buffer[6];
                                            *humidity = (rx_buffer[7] << 8) | rx_buffer[8];
                                            *conductivity = (rx_buffer[9] << 8) | rx_buffer[10];

                                            if (block_i != NULL) {
                                                *block_i = i;
                                            }

                                            if (sector_i != NULL) {
                                                *sector_i = sector_index;
                                            }

                                            if (page_i != NULL) {
                                                *page_i = page_index;
                                            }

                                            if (record_i != NULL) {
                                                *record_i = record_index;
                                            }

                                            return 1; // 成功查询到记录
                                        }
                                    }
                                    else
                                    {
                                        timestamp_start_next = 0xffffffff; // 如果没有找到下一个时间戳，则设置为最大值
                                        continue;
                                    }

                                    timestamp_start = timestamp_start_next; // 更新起始时间戳
                                }
                            }

                            timestamp_start = timestamp_start_next; // 更新起始时间戳

                        }
                    }

                    timestamp_start = timestamp_start_next; // 更新起始时间戳
                }
            }
        }
    }

    // 如果没有找到，返回默认值
    *timestamp = 0;
    *temperature = 0;
    *humidity = 0;
    *conductivity = 0;

    return 0; // 没有找到对应的记录
}

int record_next_record(int *block_i, int *sector_i, int *page_i, int *record_i, uint32_t *timestamp, uint16_t *temperature, uint16_t *humidity, uint16_t *conductivity)
{
    if (*block_i < 0 || *block_i >= BLOCK_COUNT_ALL_CHIP) {
        return 0; // 无效的块索引
    }

    if (*sector_i < 0 || *sector_i >= flash[blocks[*block_i].chip].sector_per_block) {
        return 0; // 无效的扇区索引
    }

    if (*page_i < 0 || *page_i >= flash[blocks[*block_i].chip].page_per_sector) {
        return 0; // 无效的页索引
    }

    if (*record_i < 0 || *record_i >= (flash[blocks[*block_i].chip].page_size / RECORD_LEN)) {
        return 0; // 无效的记录索引
    }

    int block_index = *block_i;
    int sector_index = *sector_i;
    int page_index = *page_i;
    int record_index = *record_i + 1;

    int hss_record = 0;


    if (record_index >= (flash[blocks[block_index].chip].page_size / RECORD_LEN)) {
        record_index = 0;
        page_index += 1;

        if (page_index >= flash[blocks[block_index].chip].page_per_sector) {
            page_index = 0;
            sector_index += 1;

            if (sector_index >= flash[blocks[block_index].chip].sector_per_block) {
                sector_index = 0;
                
                if (block_index + 1 >= BLOCK_COUNT_ALL_CHIP) {
                    return 0; // 已经到达最后一个块，没有更多记录
                }

                for (int i = block_index + 1; i < BLOCK_COUNT_ALL_CHIP; i++) {
                    if (blocks[i].sensor_id == blocks[*block_i].sensor_id && blocks[i].record_count > 0) {
                        block_index = i;
                        sector_index = 0;
                        page_index = 0;
                        record_index = 0;
                        hss_record = 1;
                        break; // 找到下一个有记录的块
                    }
                }
            }
            else
            {
                hss_record = 1; // 标记为跨扇区记录
            }
        }
        else
        {
            hss_record = 1; // 标记为跨扇区记录
        }
        
    }
    else
    {
        hss_record = 1; // 标记为跨页记录
    }

    if (hss_record == 0)
    {
        return 0; // 没有更多记录
    }

    uint32_t sector_address = blocks[block_index].block * flash[blocks[block_index].chip].block_size + sector_index * flash[blocks[block_index].chip].sector_size;
    uint32_t page_address = sector_address + page_index * flash[blocks[block_index].chip].page_size;

    flash_read(blocks[block_index].chip, page_address + record_index * RECORD_LEN, RECORD_LEN, rx_buffer);

    if (rx_buffer[RECORD_LEN - 1] != 0x55) {
        return 0; // 没有有效记录
    }

    *timestamp = (rx_buffer[0] << 24) | (rx_buffer[1] << 16) | (rx_buffer[2] << 8) | rx_buffer[3];
    *temperature = (rx_buffer[5] << 8) | rx_buffer[6];
    *humidity = (rx_buffer[7] << 8) | rx_buffer[8];
    *conductivity = (rx_buffer[9] << 8) | rx_buffer[10];

    *block_i = block_index;
    *sector_i = sector_index;
    *page_i = page_index;
    *record_i = record_index;

    return 1; // 成功获取记录
}

int record_count_page(int block_i, uint32_t page_address)
{
    int PAGE_MAX_RECORD_COUNT = flash[blocks[block_i].chip].page_size / RECORD_LEN;
    int count = 0;

    if (blocks[block_i].is_full) {
        return PAGE_MAX_RECORD_COUNT; // 如果块已满，直接返回最大记录数
    }

    for (int i = 0; i < PAGE_MAX_RECORD_COUNT; i++)
    {
        flash_read(blocks[block_i].chip, page_address + i * RECORD_LEN, RECORD_LEN, rx_buffer);

        if (rx_buffer[RECORD_LEN - 1] == 0x55) 
        {
            count++;
        }
        else
        {
            break; // 如果遇到无效记录，则停止计数
        }
    }

    return count;
}

int record_count_sector(int block_i, uint32_t sector_address)
{
    int PAGE_MAX_RECORD_COUNT = flash[blocks[block_i].chip].page_size / RECORD_LEN;
    int count = 0;

    if (blocks[block_i].is_full) {
        return flash[blocks[block_i].chip].page_per_sector * PAGE_MAX_RECORD_COUNT; // 如果块已满，直接返回最大记录数
    }

    for (int i = flash[blocks[block_i].chip].page_per_sector - 1; i >= 0; i--)
    {
        uint32_t page_address = sector_address + i * flash[blocks[block_i].chip].page_size;
        flash_read(blocks[block_i].chip, page_address, RECORD_LEN, rx_buffer);

        if (rx_buffer[RECORD_LEN - 1] != 0x55) 
        {
            continue;
        }

        count += record_count_page(block_i, page_address) + PAGE_MAX_RECORD_COUNT * i;
        break;
    }

    return count;
}

int record_query_index_offset_end(uint8_t id, uint32_t index_offest_end, uint32_t *timestamp, uint16_t *temperature, uint16_t *humidity, uint16_t *conductivity,
                                  int *block_i, int *sector_i, int *page_i, int *record_i)
{
    uint32_t current_index_offset_end = 0;

    for (int i = BLOCK_COUNT_ALL_CHIP - 1; i >= 0; i--)
    {
        if (blocks[i].sensor_id == id && blocks[i].record_count > 0)
        {
            if (current_index_offset_end + blocks[i].record_count >= index_offest_end)
            {
                // 找到对应的block
                int block_index = i;
                int sector_index = 0;
                int page_index = 0;

              

                uint32_t block_address = blocks[block_index].block * flash[blocks[block_index].chip].block_size;
                

                for (int j = flash[blocks[block_index].chip].sector_per_block - 1; j >= 0; j--)
                {
                    
                    uint32_t sector_address = block_address + j * flash[blocks[block_index].chip].sector_size;
                    int sector_count = record_count_sector(block_index, sector_address);
                    if (current_index_offset_end + sector_count >= index_offest_end)
                    {
                        // 找到对应的sector
                        sector_index = j;
                        for (int k = flash[blocks[block_index].chip].page_per_sector - 1; k >= 0; k--)
                        {

                            uint32_t page_address = sector_address + k * flash[blocks[block_index].chip].page_size;
                            int page_count = record_count_page(block_index, page_address);

                            if (current_index_offset_end + page_count >= index_offest_end)
                            {
                                page_index = k;
                                int record_index = current_index_offset_end + page_count - index_offest_end;

                                uint32_t record_address = page_address + record_index * RECORD_LEN;
                                flash_read(blocks[block_index].chip, record_address, RECORD_LEN, rx_buffer);

                                *timestamp = rx_buffer[0] << 24 | rx_buffer[1] << 16 | rx_buffer[2] << 8 | rx_buffer[3];
                                *temperature = rx_buffer[5] << 8 | rx_buffer[6];
                                *humidity = rx_buffer[7] << 8 | rx_buffer[8];
                                *conductivity = rx_buffer[9] << 8 | rx_buffer[10];
                                *block_i = block_index;
                                *sector_i = sector_index;
                                *page_i = page_index;
                                *record_i = record_index;

                                return 1; // 成功获取记录
                            }
                            current_index_offset_end += page_count;
                        }
                    }
                    current_index_offset_end += sector_count;
                }
            }

            current_index_offset_end += blocks[i].record_count;
        }
    }

    return 0;
}

void record_erase_all()
{
    for (int i = 0; i < BLOCK_COUNT_ALL_CHIP; i++)
    {
        if (blocks[i].record_count > 0)
        {
            flash_write_enable(blocks[i].chip);
            flash_erase_block(blocks[i].chip, blocks[i].block * flash[blocks[i].chip].block_size);
            
            blocks[i].record_count = 0;
            blocks[i].block = 0;
            blocks[i].start_timestamp = 0xffffffff;
            blocks[i].last_timestamp = 0xffffffff;
            blocks[i].sensor_id = 0xff;
            blocks[i].is_full = 0;
            
        }
    }
}

void record_erase_sensor(uint8_t id)
{
    for (int i = 0; i < BLOCK_COUNT_ALL_CHIP; i++)
    {
        if (blocks[i].sensor_id == id)
        {
            flash_write_enable(blocks[i].chip);
            flash_erase_block(blocks[i].chip, blocks[i].block * flash[blocks[i].chip].block_size);

            blocks[i].record_count = 0;
            blocks[i].block = 0;
            blocks[i].start_timestamp = 0xffffffff;
            blocks[i].last_timestamp = 0xffffffff;
            blocks[i].sensor_id = 0xff;
            blocks[i].is_full = 0;

        }
    }
}

uint32_t record_count(int id)
{
    uint32_t count = 0;

    for (int i = 0; i < BLOCK_COUNT_ALL_CHIP; i++)
    {
        if (blocks[i].sensor_id == id)
        {
            count += blocks[i].record_count;
        }
    }

    return count;
}

uint32_t record_start_timestamp(int id)
{
    for (int i = 0; i < BLOCK_COUNT_ALL_CHIP; i++)
    {
        if (blocks[i].sensor_id == id)
        {
            return blocks[i].start_timestamp;
        }
    }

    return 0;
}

uint32_t record_last_timestamp(int id)
{
    for (int i = BLOCK_COUNT_ALL_CHIP - 1; i >= 0; i--)
    {
        if (blocks[i].sensor_id == id)
        {
            return blocks[i].last_timestamp;
        }
    }

    return 0;
}

void record_test()
{
    for (int i = 0; i < BLOCK_COUNT_ALL_CHIP; i++)
    {
        if (blocks[i].record_count > 0)
        {
            printf("chip %d, block %d, sensor_id %d, timestamp %u, last_timestamp %u, record_count %d, is_full %d\n",
                   blocks[i].chip, blocks[i].block, blocks[i].sensor_id,
                   blocks[i].start_timestamp, blocks[i].last_timestamp,
                   blocks[i].record_count, blocks[i].is_full);
        }
    }
}
