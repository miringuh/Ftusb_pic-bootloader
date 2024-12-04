#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ftdi.h>
// #include "ftusb.h"
// #include "ft_spi.h"
// #include "chip4550.h"
#include "chip690.h"
// #include "chip628a.h"
// #include "chip630.h"
// #include "chipc84.h"

void addressx(uint32_t addr)
{
    uint8_t upper = (uint8_t)((addr & 0xFF0000) >> 16);
    uint8_t high = (uint8_t)((addr & 0x00FF00) >> 8);
    uint8_t low = (uint8_t)(addr & 0x0000FF);
    printf("00 %.4X ", (0x0E00 | upper));
    printf("00 6EF8 ");
    printf("00 %.4X ", (0x0E00 | high));
    printf("00 6EF7 ");
    printf("00 %.4X ", (0x0E00 | low));
    printf("00 6EF6\n");
}
uint8_t byte_swap(uint8_t val)
{
    uint8_t value1 = ((val & 0x0F) << 4);
    uint8_t value2 = ((val & 0xF0) >> 4);
    return (value1 | value2);
}
uint16_t swap_wrd(uint16_t addr)
{
    uint8_t high = (uint8_t)((addr & 0xFF00) >> 8);
    uint8_t low = (uint8_t)(addr & 0x00FF);

    uint8_t higher = byte_swap(high);
    uint8_t lower = byte_swap(low);
    uint16_t val = ((lower << 8) | higher);
    return val;
}
void swap_address(uint32_t addr)
{ 
    uint8_t upper = (uint8_t)((addr & 0x3F0000) >> 16);
    uint8_t high = (uint8_t)((addr & 0x00FF00) >> 8);
    uint8_t low = (uint8_t)(addr & 0x0000FF);
    printf("00 %.4X ", byte_swap(upper) << 8 | 0xE0);
    printf("00 %.4X ", swap_wrd(0x6EF8));
    printf("00 %.4X ", byte_swap(high) << 8 | 0xE0);
    printf("00 %.4X ", swap_wrd(0x6EF7));
    printf("00 %.4X ", byte_swap(low) << 8 | 0xE0);
    printf("00 %.4X\n", swap_wrd(0x6EF6));
}

int main(int argc, char **argv)
{
    usleep(100000); 
    icsp_Rd_file("/home/george/devs/P690/icsp.txt");
    // icsp_Rd_file("/home/george/devs/Pic18F4550/headers/test.txt");
    // icsp_Rd_file("/home/george/devs/Pic628a/icsp.txt");
    // icsp_Rd_file("/home/george/devs/P630/icsp.txt");
    // icsp_Rd_file("/home/george/devs/P16C84/icsp.txt");
}