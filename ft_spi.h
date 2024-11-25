#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <ftdi.h>
#if !defined(__SPI)
#define __SPI
#define pclk 0x01
#define pdata 0x02
#define ss 0x04
// uint8_t mbuff[8];
// uint8_t *msb_send(uint8_t val)
// {
//     uint8_t cnt = 1;
//     uint8_t value = (val);
//     uint8_t buff[8];
//     for (uint8_t i = 0; i < 8; i++)
//     {
//         buff[i] = ((value & cnt) >> i);
//         cnt = (cnt << 1);
//         mbuff[6] = buff[7];
//     }
//     mbuff[0] = buff[7];
//     mbuff[1] = buff[6];
//     mbuff[2] = buff[5];
//     mbuff[3] = buff[4];
//     mbuff[4] = buff[3];
//     mbuff[5] = buff[2];
//     mbuff[6] = buff[1];
//     mbuff[7] = buff[0];
//     return mbuff;
// }

void spi_wr(struct ftdi_context *ftdi, uint8_t val) // cycles
{
    uint8_t mbuff[8];
    uint16_t cnt = 1;
    uint8_t innerbuff[1];
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        mbuff[i] = ((val & cnt) >> i);
        cnt = (cnt << 1);
        if (mbuff[i] == 1)
        {
            innerbuff[0] = (pdata);
            ftdi_write_data(ftdi, innerbuff, 1);
        }
        innerbuff[0] = pclk;
        ftdi_write_data(ftdi, innerbuff, 1);
        innerbuff[0] = 0;
        ftdi_write_data(ftdi, innerbuff, 1); // cycle
    }
    innerbuff[0] = ss;
    ftdi_write_data(ftdi, innerbuff, 1);
    innerbuff[0] = 0;
    ftdi_write_data(ftdi, innerbuff, 1); // latch
    usleep(100000);
}
uint8_t char2hex(uint8_t val)
{
    uint8_t u;
    if ((isdigit(val)) == 0)
    {
        u = val - 30;
        return u;
    }
    if ((isxdigit(val)) == 0)
    {
        u = val - 55;
        return u;
    }
}

int ft_spi_wr(uint8_t buffx[]) // ftdi
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
    ftdi_set_bitmode(ftdi, 0x0F, BITMODE_BITBANG);
    ///////////
    // tx rx rts cts dtr rsd dcd ri
    // for (uint8_t i = 30; i < 70; i++)
    // {
    // }
    spi_wr(ftdi, char2hex(30));
    
    // for (uint16_t i = 0; i < strlen(buffx); i++)
    // {
    //     spi_wr(ftdi, (buffx[i] << 4) +buffx[i + 1]);
    //     // spi_wr(ftdi, (char2hex(buffx[i]) << 4) + char2hex(buffx[i + 1]));
    //     // printf("%.2d \n", (char2hex(buffx[i]) << 4) + char2hex(buffx[i + 1]));
    //     i++;
    // }
    ftdi_setrts(ftdi, 1);

    // ftdi_write_data(ftdi, 0, 0);
    ftdi_set_bitmode(ftdi, 0xFF, BITMODE_RESET);

    //
    printf("files\n");
    ftdi_disable_bitbang(ftdi);

        ftdi_usb_close(ftdi);
    done:
        ftdi_free(ftdi);

    return retval;
}
void readFile(const char *filename) // reads data
{
    FILE *fd;
    uint8_t buff[255];
    fd = fopen(filename, "r");
    // fwrite(buff, sizeof(buff), 1, fd);
    fflush(fd);
    fread(buff, sizeof(buff), 1, fd);

    ft_spi_wr(buff);

    fclose(fd);
    printf("\n");
}
#endif // __SPI
