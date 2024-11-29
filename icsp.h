#if !defined(_ICSP_)
#define _ICSP_
/*
P-00 00 off (stop)
O-11 11 hold
N-01 11 data
M-10 11 clk
L-00 11 start (vpp+vdd)
H-00 01 vpp
4-00 10 vdd
*/
#define clk "M"    // 1 clk
#define data "N"   // 2 data
#define hold "O"   // 3 clk+data
#define no_clk "L" // no-clk or vdd+vcc
#define vdd "4"    // vcc
#define mclr "H"   // mclr
#define off "P"    // off (stop)
#define cycl 2
//
unsigned char rdpin[1];
uint8_t auto_cnt = 0;
uint8_t auto_mem_cnt = 0;
uint8_t comm_buff[8];
uint8_t mxbuff[8];
uint8_t mem_buff[16];
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
uint8_t *lsb_send(uint8_t val)
{
    uint8_t cnt = 1;
    for (uint8_t i = 0; i < 4; i++)
    {
        mxbuff[i] = ((value & cnt) >> i);
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
void comm6(struct ftdi_context *ftdi, uint8_t buffd)
{
    uint16_t cnt = 1;
    for (uint8_t i = 0; i < 6; i++)
    {
        comm_buff[i] = ((buffd & cnt) >> i);
        cnt = (cnt << 1);
        if (comm_buff[i] == 1)
        {
            ftdi_write_data(ftdi, clk, 1); // clk
            usleep(cycl);
            ftdi_write_data(ftdi, hold, 1); // data+clk
            usleep(cycl);
            ftdi_write_data(ftdi, data, 1); // data
            usleep(cycl);
        }
        if (comm_buff[i] == 0)
        {
            ftdi_write_data(ftdi, clk, 1);
            usleep(cycl);
            ftdi_write_data(ftdi, no_clk, 1);
            usleep(cycl);
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
            ftdi_write_data(ftdi, clk, 1);
            usleep(cycl);
            ftdi_write_data(ftdi, no_clk, 1);
            usleep(cycl);
            break;
        case 8:
            printf(" ");
            continue;
        case 16:
            ftdi_write_data(ftdi, clk, 1);
            usleep(cycl);
            ftdi_write_data(ftdi, no_clk, 1);
            usleep(cycl);
            printf("\n");
            auto_cnt = 0;
            return 0;
            break;
        default:
            ftdi_write_data(ftdi, clk, 1); // clk
            usleep(cycl);
            ftdi_read_pins(ftdi, rdpin);
            usleep(cycl);
            ftdi_write_data(ftdi, no_clk, 1); // no-clk
            usleep(cycl);
            rdVal = (uint8_t)((rdpin[0] & 0x02) >> 1);
            printf("%d ", rdVal);
            if (mem_buff[auto_cnt] != rdVal)
            {
                ftdi_write_data(ftdi, "P", 1); // OFF
                usleep(cycl);
                printf("Error!!");
                exit(0);
            }
            auto_cnt++;
            break;
        }
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
//
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

#endif // _ICSP_
