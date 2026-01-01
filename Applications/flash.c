#include "flash.h"
#include "gd32f30x_gpio.h"
#include "gd32f30x_spi.h"

#include <stdio.h>

#define SPI_FLASH0_CS_LOW() gpio_bit_reset(GPIOA, GPIO_PIN_4)
#define SPI_FLASH0_CS_HIGH() gpio_bit_set(GPIOA, GPIO_PIN_4)

#define SPI_FLASH1_CS_LOW() gpio_bit_reset(GPIOC, GPIO_PIN_5)
#define SPI_FLASH1_CS_HIGH() gpio_bit_set(GPIOC, GPIO_PIN_5)

#define SPI_FLASH_CS_LOW(chip) do { \
    if (chip == 0) { \
        SPI_FLASH0_CS_LOW(); \
    } else if (chip == 1) { \
        SPI_FLASH1_CS_LOW(); \
    } \
} while(0)

#define SPI_FLASH_CS_HIGH(chip) do { \
    if (chip == 0) { \
        SPI_FLASH0_CS_HIGH(); \
    } else if (chip == 1) { \
        SPI_FLASH1_CS_HIGH(); \
    } \
} while(0)

#define DUMMY 0xA5

static Flash flash[2] = {0};

uint8_t spi_flash_send_byte(uint8_t byte)
{
    /* loop while data register in not empty */
    while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));

    /* send byte through the SPI0 peripheral */
    spi_i2s_data_transmit(SPI0, byte);

    /* wait to receive a byte */
    while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE));

    /* return the byte read from the SPI bus */
    return(spi_i2s_data_receive(SPI0));
}

int flash_is_busy(int chip)
{
    SPI_FLASH_CS_LOW(chip);
    spi_flash_send_byte(0x05); // Read Status Register command
    uint8_t status = spi_flash_send_byte(DUMMY);
    SPI_FLASH_CS_HIGH(chip);

    // Check if the busy bit (bit 0) is set
    return (status & 0x01);
}

int flash_is_writable(int chip)
{
    SPI_FLASH_CS_LOW(chip);
    spi_flash_send_byte(0x05); // Read Status Register command
    uint8_t status = spi_flash_send_byte(DUMMY);
    SPI_FLASH_CS_HIGH(chip);

//    printf("status %02X\n", status);
    
    // Check if the write enable bit (bit 1) is set
    return (status & 0x02);
}

int flash_write_enable(int chip)
{
    SPI_FLASH_CS_LOW(chip);
    spi_flash_send_byte(0x06); // Write Enable command
    SPI_FLASH_CS_HIGH(chip);

    // Check if the write enable bit is set
    return 1;//flash_is_writable(chip);
}

void flash_reset(int chip)
{
    SPI_FLASH_CS_LOW(chip);
    spi_flash_send_byte(0x66);
    SPI_FLASH_CS_HIGH(chip);

    SPI_FLASH_CS_LOW(chip);
    spi_flash_send_byte(0x99);
    SPI_FLASH_CS_HIGH(chip);
    
    SPI_FLASH_CS_LOW(chip);
    uint8_t status1 = spi_flash_send_byte(0x05);
    SPI_FLASH_CS_HIGH(chip);
    
    SPI_FLASH_CS_LOW(chip);
    uint8_t status2 = spi_flash_send_byte(0x35);
    SPI_FLASH_CS_HIGH(chip);
    
    SPI_FLASH_CS_LOW(chip);
    uint8_t wsr = spi_flash_send_byte(0x01);
    SPI_FLASH_CS_HIGH(chip);
//    
//    printf("status1 %02X, status2 %02X, wsr %02X\n", status1, status2, wsr);
    
}

void flash_readid(int chip, uint8_t *mid, uint16_t *id)
{
    SPI_FLASH_CS_LOW(chip);
    spi_flash_send_byte(0x9f);
    *mid = spi_flash_send_byte(DUMMY);
    uint8_t i2 = spi_flash_send_byte(DUMMY);
    uint8_t i3 = spi_flash_send_byte(DUMMY);
    SPI_FLASH_CS_HIGH(chip);
    *id = (i2 << 8) | i3;
    // printf("MID: %02X, ID: %04X\n", *mid, *id);
}

int flash_read(int chip, uint32_t address, uint32_t length, uint8_t *data)
{
    SPI_FLASH_CS_LOW(chip);
    spi_flash_send_byte(0x03);
    spi_flash_send_byte((address >> 16) & 0xff);
    spi_flash_send_byte((address >> 8) & 0xff);
    spi_flash_send_byte((address >> 0) & 0xff);
    
    
    for (int i = 0; i < length; i++)
    {
        data[i] = spi_flash_send_byte(DUMMY);
    }


    SPI_FLASH_CS_HIGH(chip);

    return 1;
}

int flash_erase_sector(int chip, uint32_t address)
{
    if (!flash_is_writable(chip)) {
        return 0; // Write enable failed
    }

//    printf("erase sector\n");
    
    SPI_FLASH_CS_LOW(chip);
    spi_flash_send_byte(0x20); // Block Erase command
    spi_flash_send_byte((address >> 16) & 0xff);
    spi_flash_send_byte((address >> 8) & 0xff);
    spi_flash_send_byte((address >> 0) & 0xff);
    SPI_FLASH_CS_HIGH(chip);

    // Wait for the erase operation to complete
    while (flash_is_busy(chip));

    return 1;
}

int flash_erase_block(int chip, uint32_t address)
{
    if (!flash_is_writable(chip)) {
        return 0; // Write enable failed
    }

    SPI_FLASH_CS_LOW(chip);
    spi_flash_send_byte(0xD8); // Block Erase command
    spi_flash_send_byte((address >> 16) & 0xff);
    spi_flash_send_byte((address >> 8) & 0xff);
    spi_flash_send_byte((address >> 0) & 0xff);
    SPI_FLASH_CS_HIGH(chip);

    // Wait for the erase operation to complete
    while (flash_is_busy(chip));

    return 1;
}

int flash_erase_chip(int chip)
{
    if (!flash_is_writable(chip)) {
        return 0; // Write enable failed
    }

    SPI_FLASH_CS_LOW(chip);
    spi_flash_send_byte(0xC7); // Chip Erase command
    SPI_FLASH_CS_HIGH(chip);

    // Wait for the erase operation to complete
    while (flash_is_busy(chip));

    return 1;
}

int flash_write(int chip, uint32_t address, uint32_t length, const uint8_t *data)
{
    if (!flash_is_writable(chip)) {
        return 0; // Write enable failed
    }

    SPI_FLASH_CS_LOW(chip);
    spi_flash_send_byte(0x02); // Page Program command
    spi_flash_send_byte((address >> 16) & 0xff);
    spi_flash_send_byte((address >> 8) & 0xff);
    spi_flash_send_byte((address >> 0) & 0xff);

    for (uint32_t i = 0; i < length; i++) {
        spi_flash_send_byte(data[i]);
    }

    SPI_FLASH_CS_HIGH(chip);
    

    // Wait for the write operation to complete
    while (flash_is_busy(chip));

    return 1;
}

void flash_get(int chip, Flash *f)
{
    if (chip < 0 || chip > 1) {
        return; // Invalid chip index
    }
    
    f->is_active = flash[chip].is_active;
    f->block_per_chip = flash[chip].block_per_chip;
    f->sector_per_block = flash[chip].sector_per_block;
    f->page_per_sector = flash[chip].page_per_sector;
    f->byte_per_page = flash[chip].byte_per_page;

    f->block_size = flash[chip].block_size;
    f->sector_size = flash[chip].sector_size;
    f->page_size = flash[chip].page_size;
}

void flash_init()
{
    flash_reset(0);

    uint8_t mid = 0;
    uint16_t id = 0;

    flash_readid(0, &mid, &id);
    
//    printf("0 mid %02X, id %04X\n", mid, id);
    
    if (mid == 0xBA && id == 0x3217)
    {
        flash[0].is_active = 1;
        flash[0].block_per_chip = 128;
        flash[0].sector_per_block = 16;
        flash[0].page_per_sector = 16;
        flash[0].byte_per_page = 256;

        flash[0].block_size = flash[0].sector_per_block * flash[0].page_per_sector * flash[0].byte_per_page;
        flash[0].sector_size = flash[0].page_per_sector * flash[0].byte_per_page;
        flash[0].page_size = flash[0].byte_per_page;
    }
    else
    {
        flash[0].is_active = 0;
    }

    flash_reset(1);
    
    mid = 0;
    id = 0;
    flash_readid(1, &mid, &id);
    
//    printf("1 mid %02X, id %04X\n", mid, id);
    
    if (mid == 0xBA && id == 0x3217)
    {
        flash[1].is_active = 1;
        flash[1].block_per_chip = 128;
        flash[1].sector_per_block = 16;
        flash[1].page_per_sector = 16;
        flash[1].byte_per_page = 256;

        flash[1].block_size = flash[1].sector_per_block * flash[1].page_per_sector * flash[1].byte_per_page;
        flash[1].sector_size = flash[1].page_per_sector * flash[1].byte_per_page;
        flash[1].page_size = flash[1].byte_per_page;
    }
    else
    {
        flash[1].is_active = 0;
    }
}
