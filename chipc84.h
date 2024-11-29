#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#if !defined(__P16C84)
#define __P16C84

#define Ld_conf 0x00      // 0,data(14),0
#define Ld_data_prog 0x02 // 0,data(14),0
#define Ld_data_mem 0x03  // 0,data(8),zero(6),0
#define Rd_prog 0x04      // 0,data(14),0
#define Rd_mem 0x05       // 0,data(8),zero(6),0
#define Incr 0x06
#define Begin_prog_only 0x08 // addrsd loc is not erased before
#define Bulk_er_prog 0x09    // internal if CPD==1 word+2007 er
#define Bulk_er_mem 0x0B     // internal IF CPD==1
#define tera 20000           // 6 ms max->erase cycl
#define tprog1 20000         // 3 ms min->prog-int cycle time/del
#define tprog2 20000         // 3 ms min->prog-ext cycle time/deL
#define tdis 50              // 100 ns min

#define tset0 40    // min 100ns
#define thld0 tset0     // 2 us max-> delay b4 vdd^
#define tset1 tset0     // 100 ns min-> datain b4 clk-down
#define thld1 tset1 // 100 ns min->data hold after clk-down
#define tdly2 tset1 // 1 us min->clk-delay up/down data<>comm
#define tdly1 tset1 // 1 us min->delay for data<>comm
#define tdly3 tset1 // 80 ns max->rd-clk^ to data-out

/*
P-00 00 off (stop)
O-11 11 hold
N-01 11 data
M-10 11 clk
L-00 11 start (vpp+vdd)
H-00 01 vpp
4-00 10 vdd
*/
#define clk 'M'    // clk
#define data 'N'   // data
#define hold 'O'   // clk+data
#define no_clk 'L' // no-clk or vdd+vcc
#define vdd '4'    // vcc
#define mclr 'H'   // mclr
#define off 'P'    // off (stop)

unsigned char rdpin[1];
uint8_t auto_cnt = 0;
uint8_t auto_mem_cnt = 0;
uint8_t comm_buff[8];
uint8_t mxbuff[8];
uint8_t mem_buff[16];
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
}
void Start(struct ftdi_context *ftdi)
{
    ftdi_write_data(ftdi, "P", 1); // 0
    usleep(1000);
    ftdi_write_data(ftdi, "H", 1); // vdd
    usleep(2000);
    ftdi_write_data(ftdi, "L", 1); // vdd+vpp
    usleep(1000);
}
void Stop(struct ftdi_context *ftdi)
{
    ftdi_write_data(ftdi, "P", 1);
    usleep(20000);
    ftdi_write_data(ftdi, "4", 1); // vcc
    usleep(20000);
    printf("END !");
}

uint8_t *lsb_send(uint8_t val)//mxbuff
{
    uint8_t cnt = 1;
    for (uint8_t i = 0; i < 4; i++)
    {
        mxbuff[i] = ((val & cnt) >> i);
        cnt = (cnt << 1);
    }
    return mxbuff;
}
uint8_t *msb_send(uint8_t val)
{
    uint8_t cnt = 1;
    uint8_t value = (val);
    uint8_t buff[8];
    for (uint8_t i = 0; i < 8; i++)
    {
        buff[i] = ((value & cnt) >> i);
        cnt = (cnt << 1);
    }
    mxbuff[7] = buff[7];
    mxbuff[1] = buff[6];
    mxbuff[2] = buff[5];
    mxbuff[3] = buff[4];
    mxbuff[4] = buff[3];
    mxbuff[5] = buff[2];
    mxbuff[6] = buff[1];
    mxbuff[7] = buff[0];

    return mxbuff;
}
uint8_t icsp_cycle(struct ftdi_context *ftdi, uint8_t byte)
{
    uint8_t cnt = 1;
    lsb_send(byte);
    for (uint8_t i = 0; i < 4; i++)
    {
        if (auto_cnt == 0)
        {
            ftdi_write_data(ftdi, "M", 1); // clk
            usleep(tset1);
            ftdi_write_data(ftdi, "L", 1); // no-clk
            usleep(tset1);
        }
        if (auto_cnt == 14)
        {
            ftdi_write_data(ftdi, "M", 1); // clk
            usleep(tset1);
            ftdi_write_data(ftdi, "L", 1); // no-clk
            usleep(tset1);
            auto_cnt = 0;
            auto_mem_cnt = 0;
            return 0;
        }
        if (mxbuff[i] == 1)
        {
            ftdi_write_data(ftdi, "M", 1); // clk
            usleep(tset1);
            ftdi_write_data(ftdi, "O", 1); // data+clk
            usleep(tset1);
            ftdi_write_data(ftdi, "N", 1); // hold-data
            usleep(tset1);
            mem_buff[auto_mem_cnt] = 1;
            auto_mem_cnt++;
        }
        if (mxbuff[i] == 0)
        {
            ftdi_write_data(ftdi, "M", 1); // clk
            usleep(tset1);
            ftdi_write_data(ftdi, "L", 1); // no-clk
            usleep(tset1);
            mem_buff[auto_mem_cnt] = 0;
            auto_mem_cnt++;
        }
        auto_cnt++;
    }
}
void word(struct ftdi_context *ftdi, uint8_t buffd)
{
    icsp_cycle(ftdi, char2hex2(buffd));
}
void comm6(struct ftdi_context *ftdi, uint8_t buffd)
{
    uint16_t cnt = 1;
    for (uint8_t i = 0; i < 6; i++)
    {
        comm_buff[i] = ((buffd & cnt) >> i);
        cnt = (cnt << 1);
        if (comm_buff[i] == 1)
        {
            ftdi_write_data(ftdi, "M", 1); // clk
            usleep(tset1);
            ftdi_write_data(ftdi, "O", 1); // data+clk
            usleep(tset1);
            ftdi_write_data(ftdi, "N", 1); // data
            usleep(tset1);
        }
        if (comm_buff[i] == 0)
        {
            ftdi_write_data(ftdi, "M", 1);
            usleep(tset1);
            ftdi_write_data(ftdi, "L", 1);
            usleep(tset1);
        }
    }
}
uint8_t read_chip(struct ftdi_context *ftdi)
{
    auto_cnt == 0;
    uint8_t rdVal = 0;
    for (uint8_t i = 0; i < 17; i++)
    {
        switch (i)
        {
        case 0:
            ftdi_write_data(ftdi, "M", 1);
            usleep(tset1);
            ftdi_write_data(ftdi, "L", 1);
            usleep(tset1);
            // printf(" ");
            break;
        case 8:
            printf(" ");
            continue;
        case 16:
            ftdi_write_data(ftdi, "M", 1);
            usleep(tset1);
            ftdi_write_data(ftdi, "L", 1);
            usleep(tset1);
            printf("\n");
            auto_cnt = 0;
            return 0;
            break;
        default:
            ftdi_write_data(ftdi, "M", 1); // clk
            usleep(tset1);
            ftdi_read_pins(ftdi, rdpin);
            usleep(tset1);
            ftdi_write_data(ftdi, "L", 1); // no-clk
            usleep(tset1);
            //
            rdVal = (uint8_t)((rdpin[0] & 0x02) >> 1);
            printf("%d ", rdVal);
            if (mem_buff[auto_cnt] != rdVal)
            {
                // ftdi_write_data(ftdi, "P", 1); // OFF
                // usleep(tset1);
                // printf("Error!!");
                // exit(0);
            }
            auto_cnt++;
            break;
        }
    }
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
    case 'l':
        comm6(ftdi, Ld_conf);
        usleep(tdly2);
        break;
    case 'L':
        comm6(ftdi, Ld_data_prog);
        usleep(tdly2);
        break;
    case 'M':
        comm6(ftdi, Ld_data_mem);
        usleep(tdly2);
        break;
    case 'R':
        comm6(ftdi, Rd_prog);
        usleep(tdly2);
        read_chip(ftdi);
        usleep(tdly2);
        break;
    case 'r':
        comm6(ftdi, Rd_mem);
        usleep(tdly2);
        break;
    case 'G':
        comm6(ftdi, Begin_prog_only);
        usleep(tprog1);
        break;
    case 'I':
        comm6(ftdi, Incr);
        usleep(tdly2);
        break;
    case 'Z':
        comm6(ftdi, Bulk_er_prog);
        usleep(tera);
        break;
    case 'z':
        comm6(ftdi, Bulk_er_mem);
        usleep(tera);
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
    uint8_t buff[32000];
    fd = fopen(filename, "r");
    fread(buff, sizeof(buff), 1, fd);
    fflush(fd);
    printf("size %ld len %ld\n", sizeof(buff), strlen(buff));
    for (uint16_t i = 0; i < strlen(buff); i++)
    {
        icsp(ftdi, buff[i]);
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

/*
       S start
       T stop
       L ld-prog
       l ld-conf
       M ld-mem
       R read-prog
       r read-mem
       I incr
       G Beg_prog
       Z Blk er prog
       z Blk er mem
       */
#endif // __P16C84
