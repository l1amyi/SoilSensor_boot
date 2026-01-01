#ifndef _RECORD_H_
#define _RECORD_H_
#include <stdint.h>

void record_init(void);
int record_add(uint32_t timestamp, uint8_t id, uint16_t temperature, uint16_t humidity, uint16_t conductivity);

int record_query_by_time(uint32_t time, uint8_t id, uint32_t *timestamp, uint16_t *temperature, uint16_t *humidity, uint16_t *conductivity,
                        int *block_i, int *sector_i, int *page_i, int *record_i);
int record_next_record(int *block_i, int *sector_i, int *page_i, int *record_i, uint32_t *timestamp, uint16_t *temperature, uint16_t *humidity, uint16_t *conductivity);


int record_query_index_offset_end(uint8_t id, uint32_t index_offest_end, uint32_t *timestamp, uint16_t *temperature, uint16_t *humidity, uint16_t *conductivity,
                                  int *block_i, int *sector_i, int *page_i, int *record_i);

void record_erase_all(void);
void record_erase_sensor(uint8_t id);


uint32_t record_count(int id);

uint32_t record_start_timestamp(int id);
uint32_t record_last_timestamp(int id);

void record_test(void);

#endif
