#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <ftdi.h>
#if !defined(__FTUSB)
#define __FTUSB
unsigned char rddpin[1];
//
uint8_t ftchar2hex2(unsigned char buff)
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
    if (buff >= 71 && buff <= 90) // G-Z
    {
        u = buff - 55;
        return u;
    }
    if (buff >= 97 && buff <= 122) // a-z
    {
        u = buff - 61;
        return u;
    }
}

void write_file(unsigned char *fname, unsigned char *buf)
{
    FILE *fd;
    uint8_t buff[255];
    strcpy(buff, buf);
    fd = fopen(fname, "w");

    fflush(fd);

    // fwrite(buff, sizeof(buff), 1, fd);
    for (uint16_t i = 0; i < strlen(buff); i++)
    {
        fprintf(fd, "%c", buff[i]);
    }
    fflush(fd);
    fclose(fd);
}
// usart
int ft_usart(char *message)
{
    struct ftdi_context *ftdi;
    int retval = 0;

    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return EXIT_FAILURE;
    }
    uint8_t f = ftdi_usb_open(ftdi, 0x0403, 0x6001);
    if (f < 0 && f != -5)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(ftdi));
        retval = 1;
        goto done;
    }
    ftdi_set_baudrate(ftdi, 19200);
    ftdi_set_line_property(ftdi, BITS_8, STOP_BIT_1, NONE);
    ftdi_set_latency_timer(ftdi, 255);
    // ftdi_tcioflush(ftdi);
    ftdi_write_data(ftdi, message, strlen(message));

    printf("\n");
    ftdi_usb_close(ftdi);
done:
    ftdi_free(ftdi);
}
int ft_usart_read()
{
    struct ftdi_context *ftdi;
    int retval = 0;
    unsigned char *buff;

    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return EXIT_FAILURE;
    }
    uint8_t f = ftdi_usb_open(ftdi, 0x0403, 0x6001);
    if (f < 0 && f != -5)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(ftdi));
        retval = 1;
        goto done;
    }
    ftdi_set_baudrate(ftdi, 19200);
    ftdi_set_line_property(ftdi, BITS_8, STOP_BIT_1, NONE);
    ftdi_set_latency_timer(ftdi, 255);
    // ftdi_tcioflush(ftdi);

    ftdi_read_data(ftdi, buff, 1);

    printf("\n");
    ftdi_usb_close(ftdi);
done:
    ftdi_free(ftdi);
}
int ft_bitbang(unsigned char *mess)
{
    struct ftdi_context *ftdi;
    int f, i;
    unsigned char buf[255];
    unsigned char buf2[255];
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

    // ftdi_set_baudrate(ftdi, 19200);
    // ftdi_set_line_property(ftdi, BITS_8, STOP_BIT_1, NONE);
    // ftdi_set_latency_timer(ftdi, 255);
    // ftdi_set_bitmode(ftdi, 0xFF, BITMODE_RESET);
    ftdi_set_bitmode(ftdi, 0XFF, BITMODE_BITBANG);
    // tx rx rts cts dtr rsd dcd ri
    strcpy(buf, mess);
    for (uint8_t i = 0; i < strlen(mess); i++)
    {
        buf2[i] = ftchar2hex2(buf[i]);
    }
    for (int i = 0; i <= strlen(mess); i++)
    {
        ftdi_write_data(ftdi, buf2, i);
        usleep(1000000);
    }

    // int vc = ftdi_setdtr_rts(ftdi,1,1);
    // printf("resd %d\n", vc);

    // unsigned char pin[1];
    // ftdi_read_pins(ftdi, pin);
    // printf("read %d \n",pin[0] );

    // printf("disabling bitbang mode\n");
    // ftdi_disable_bitbang(ftdi);

    ftdi_usb_close(ftdi);
done:
    ftdi_free(ftdi);

    return retval;
}
void file_read(unsigned char *filename)
{
    FILE *fd;
    uint8_t buff[255];
    fd = fopen(filename, "r");
    // fflush(fd);
    fread(buff, sizeof(buff), 1, fd);
    for (uint16_t i = 0; i < strlen(buff); i++)
    {
        printf("%c ", buff[i]);
    }
    ft_usart(buff);
    fclose(fd);
}
/// SPI_MODE ////
uint8_t spi_cycle(struct ftdi_context *ftdi, uint8_t byte)
{
    // lsb_send(byte);
    // for (uint8_t i = 0; i < 4; i++)
    // {
    //     if (auto_cnt == 0)
    //     {
    //         ftdi_write_data(ftdi, "M", 1); // clk
    //         usleep(dely);
    //         ftdi_write_data(ftdi, "L", 1); // no-clk
    //         usleep(dely);
    //     }
    //     if (auto_cnt == 14)
    //     {
    //         ftdi_write_data(ftdi, "M", 1); // clk
    //         usleep(dely);
    //         ftdi_write_data(ftdi, "L", 1); // no-clk
    //         usleep(dely);
    //         auto_cnt = 0;
    //         return 0;
    //     }
    //     if (mxbuff[i] == 1)
    //     {
    //         ftdi_write_data(ftdi, "M", 1); // clk
    //         usleep(dely);
    //         ftdi_write_data(ftdi, "O", 1); // data+clk
    //         usleep(dely);
    //         ftdi_write_data(ftdi, "N", 1); // hold-data
    //         usleep(dely);
    //     }
    //     if (mxbuff[i] == 0)
    //     {
    //         ftdi_write_data(ftdi, "M", 1); // clk
    //         usleep(dely);
    //         ftdi_write_data(ftdi, "L", 1); // no-clk
    //         usleep(dely);
    //     }
    //     auto_cnt++;
    // }
}
uint8_t Spi_mode(unsigned char *mesg)
{
    struct ftdi_context *ftdi;
    int f, i;
    unsigned char buf[255];
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
    // tx rx rts cts dtr rsd dcd ri
    strcpy(buf, mesg);
    for (uint8_t i = 0; i < 20; i++)
    // for (uint8_t i = 0; i < strlen(mesg); i++)
    {
        // icsp(ftdi,( buf[i]));
        ftdi_read_pins(ftdi, rddpin);
        printf("%X ",((rddpin[0]&0x02)>>1));

        usleep(20000);
    }
    printf("\n");
    // printf("disabling bitbang mode\n");
    // ftdi_disable_bitbang(ftdi);
    ftdi_usb_close(ftdi);
done:
    ftdi_free(ftdi);
    return retval;
}
uint8_t spi_mode_Rd_file(unsigned char *filename)
{
    struct ftdi_context *ftdi;
    int f, i;
    unsigned char buf[255];
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
    ftdi_set_bitmode(ftdi, 0xF7, BITMODE_BITBANG);
    //
    FILE *fd;
    uint8_t buff[255];
    fd = fopen(filename, "r");
    fflush(fd);
    fread(buff, sizeof(buff), 1, fd);
    printf("size %ld len %ld\n", sizeof(buff), strlen(buff));
    for (uint16_t i = 0; i < strlen(buff); i++)
    {
        // icsp(ftdi, buff[i]);
        // printf("%d ", buff[i]);
    }
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
#endif // __FTUSB
