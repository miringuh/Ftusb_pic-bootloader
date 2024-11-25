#if !defined(__F4550)
#define __F4550
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
//
#define p1 4  // max 1us mclr^
#define p2 p1  // min 1us clk ---period
#define p2A p1 //(p1 / 2) // min 400ns clk high
#define p2B p1 //(1 / 2) // min 400ns  clk low
#define p3 p2A // min 15ns    data-set
#define p4 p2A // min 15ns    data-hold
#define p5 10  // min 40n    _delay_us b2in comm---comm operand
#define p5A 10 // min 40n    _delay_us b2in comm---comm operand

#define p6 p2A     // min 20ns   pgc-low _delay_us b2in comm---rd
#define p9 2000   // min 1ms *** pgc high
#define p10 200   // min 100us pgc-low
#define p11 6000  // min 5ms write/blk erase delay
#define p11A 5000 // min 4ms data write poll time
#define p12 p2     // min 2us input data hold from mclr^
#define p13 p2     // min 100ns vdd^ to mclr ^
/*
P-00 00 off (stop)
O-11 11 hold
N-01 11 data
M-10 11 clk
L-00 11 noclk start (vpp+vdd)
H-00 01 vpp
4-00 10 vdd
*/
uint8_t auto_cnt = 0;
uint8_t addr_cnt = 0;
unsigned char rdpin[1];
uint8_t auto_mem_cnt = 0;
uint8_t mem_buff[4];
uint8_t comm_buff[4];
uint8_t mxbuff[4];
uint8_t wrd_buff[16];
//
uint8_t char2hex2(unsigned char buff)
{
    uint8_t u;
    if (buff >= 48 && buff <= 57) // 0-9
    {
        u = buff - 48;
        return u;
    }
    if (buff >= 65 && buff <= 70) // A-F
    {
        u = buff - 55;
        return u;
    }
    // if (buff >= 71 && buff <= 90) // G-Z
    // {
    //     u = buff - 55;
    //     return u;
    // }
    // if (buff >= 97 && buff <= 122) // a-z
    // {
    //     u = buff - 61;
    //     return u;
    // }
}
void Start(struct ftdi_context *ftdi)
{
    ftdi_write_data(ftdi, "P", 1); // 0
    usleep(100);
    ftdi_write_data(ftdi, "H", 1); // vdd
    usleep(1000);
    ftdi_write_data(ftdi, "L", 1); // vdd+vpp
    usleep(10000);
}
void Stop(struct ftdi_context *ftdi)
{
    ftdi_write_data(ftdi, "P", 1); // off
    usleep(10000);
    ftdi_write_data(ftdi, "4", 1); // vcc
    usleep(10000);
    printf("END !");
}

uint8_t *lsb_send(uint8_t val)
{
    uint8_t cnt = 1;
    for (uint8_t i = 0; i < 4; i++)
    {
        mxbuff[i] = ((val & cnt) >> i);
        cnt = (cnt << 1);
    }
    return mxbuff;
}
//
uint8_t read_chip(struct ftdi_context *ftdi)
{
    uint8_t rdVal = 0;
    for (uint8_t i = 0; i < 16; i++)
    {
        ftdi_write_data(ftdi, "M", 1); // clk
        usleep(p1);
        ftdi_read_pins(ftdi, rdpin);
        usleep(p1);
        ftdi_write_data(ftdi, "L", 1); // no-clk
        usleep(p1);
        rdVal = (uint8_t)((rdpin[0] & 0x02) >> 1);
        printf("%x", rdVal);
        if (i == 7)
        {
            usleep(p6);
            printf(" ");
        }
    }
    printf("\n");
}
//
uint8_t word(struct ftdi_context *ftdi, uint8_t byte)
{
    uint8_t cnt = 1;
    for (uint8_t i = 0; i < 4; i++)
    {
        mxbuff[i] = ((byte & cnt) >> i);
        cnt = (cnt << 1);
        if (mxbuff[i] == 1)
        {
            ftdi_write_data(ftdi, "M", 1); // clk
            usleep(p2B);
            ftdi_write_data(ftdi, "O", 1); // data+clk
            usleep(p2B);
            ftdi_write_data(ftdi, "N", 1); //N data
            usleep(p2B);
        }
        if (mxbuff[i] != 1)
        {
            ftdi_write_data(ftdi, "M", 1); // clk
            usleep(p2B);
            ftdi_write_data(ftdi, "L", 1); // no data/clk
            usleep(p2B);
        }
    }
    auto_mem_cnt++;
    if (auto_mem_cnt == 4)
    {
        auto_mem_cnt = 0;
        usleep(p5A);
    }
}
void comm4(struct ftdi_context *ftdi, uint8_t buffd)
{
    uint8_t cnt = 1;
    for (uint8_t i = 0; i < 4; i++)
    {
        comm_buff[i] = ((buffd & cnt) >> i);
        cnt = (cnt << 1);
        if (comm_buff[i] == 1)
        {
            ftdi_write_data(ftdi, "M", 1); // clk
            usleep(p2B);
            ftdi_write_data(ftdi, "O", 1); // data+clk
            usleep(p2B);
            ftdi_write_data(ftdi, "N", 1); // data
            usleep(p2B);
        }
        if (comm_buff[i] != 1)
        {
            ftdi_write_data(ftdi, "M", 1); // clk
            usleep(p2B);
            ftdi_write_data(ftdi, "L", 1); // no data/clk
            usleep(p2B);
        }
    }
    usleep(p5);
}
// p1
void nop(struct ftdi_context *ftdi)
{
    uint16_t cnt = 1;
    for (uint8_t i = 0; i < 3; i++)
    {
        ftdi_write_data(ftdi, "M", 1); // clk
        usleep(p1);
        ftdi_write_data(ftdi, "L", 1); // no clk
        usleep(p1);
    }
    ftdi_write_data(ftdi, "M", 1);
    usleep(p9);
    ftdi_write_data(ftdi, "L", 1);
    usleep(p10);
    comm4(ftdi, 0x00);
    comm4(ftdi, 0x00);
}

uint8_t icsp(struct ftdi_context *ftdi, uint8_t val)
{
    switch (val)
    {
    case 'S':
        Start(ftdi);
        break;
    case 'T':
        Stop(ftdi);
        break;
    case 'K':
        comm4(ftdi, 0X00);
        break;
    case 'k':
        comm4(ftdi, 0x0F);
        break;
    case 'Q':
        comm4(ftdi, 0X09);
        break;
    case 'M':
        comm4(ftdi, 0X0D);
        break;
    case 'm':
        comm4(ftdi, 0X0C);
        break;
    case 'N':
        nop(ftdi);
        break;
    case 'Z':
        nop(ftdi);
        usleep(p11);
        break;
    case 'R':
        read_chip(ftdi);
        break;
    default:
        word(ftdi, char2hex2(val));
        break;
    }
}
uint8_t icsp_Rd_file(unsigned char *filename)
{
    struct ftdi_context *ftdi;
    int f, i;
    uint8_t ccnt = 1;
    int retval = 0;
    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return EXIT_FAILURE;
    }
    f = ftdi_usb_open(ftdi, 0x0403, 0x6001);
    if (f < 0 && f != -5)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(ftdi));
        retval = 1;
        goto done;
    }
    printf("ftdi open succeeded: %d\n", f);
    printf("enabling bitbang mode\n");
    ftdi_set_bitmode(ftdi, 0xFF, BITMODE_BITBANG);
    //
    FILE *fd;
    uint8_t buff[64000];
    fd = fopen(filename, "r");
    fread(buff, sizeof(buff), 1, fd);
    fflush(fd);
    printf("size %ld len %ld\n\n", sizeof(buff), strlen(buff));
    for (uint16_t i = 0; i < strlen(buff); i++)
    {
        icsp(ftdi, buff[i]);
        // printf("%c ", buff[i]);
    }
    ftdi_write_data(ftdi, "4", 1);
    printf("END !\n");
    fflush(fd);
    fclose(fd);

    // printf("disabling bitbang mode\n");
    // ftdi_disable_bitbang(ftdi);
    ftdi_usb_close(ftdi);
done:
    ftdi_free(ftdi);
    printf("\n");
    return retval;
}

#endif // __F4550
 /*
S start
T stop
K comm--0x00
k comm--0x0F
Q comm--0x09
M comm--0x0D
m comm--0x0C
N nop
Z erase_nop >nop/delay
R read_mem
       */