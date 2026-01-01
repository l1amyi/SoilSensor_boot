#ifndef _FLASH_H_
#define _FLASH_H_

#include <stdint.h>

typedef struct
{
    int is_active; // 0 or 1
    int block_per_chip;
    int sector_per_block;
    int page_per_sector;
    int byte_per_page;

    uint32_t block_size;
    uint32_t sector_size;
    uint32_t page_size;
} Flash;

void flash_reset(int chip);
void flash_readid(int chip, uint8_t *mid, uint16_t *id);

int flash_write_enable(int chip);
int flash_is_busy(int chip);
int flash_is_writable(int chip);

int flash_read(int chip, uint32_t address, uint32_t length, uint8_t *data);
int flash_write(int chip, uint32_t address, uint32_t length, const uint8_t *data);

int flash_erase_sector(int chip, uint32_t address);
int flash_erase_block(int chip, uint32_t address);
int flash_erase_chip(int chip);

void flash_get(int chip, Flash *f);

void flash_init(void);

#endif
