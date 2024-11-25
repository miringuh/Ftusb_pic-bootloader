#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#if !defined(__USB101)
#define __USB101
#define DEV_DESC 0X0100
#define CONF_DESC 0X0200
#define STRING_DESC 0X0300
#define INTF_DESC 0X0400
#define EP_DESC 0X0500
#define STATUS 0X00
// brequests//
#define GetStatus 0x00
#define ClearDevFeature 0x01
#define SetDevFeature 0x03
#define SetAddress 0x05
#define GetDesc 0x06
#define SetDesc 0x07
#define GetConf 0x08
#define SetConf 0x09
#define GetIntf 0x0A
#define SetIntf 0x0B
#define SyncFrame 0xC0

#define id_vid 0x0403
#define id_pid 0x6001 // ftdi chip rs232rl
// #define id_vid 0x0451
// #define id_pid 0x3410 // texas 3410
// #define id_vid 0X0403
// #define id_pid 0x6001 //arduino uno//
// #define id_vid 0X1A86
// #define id_pid 0x7523 //arduino nano
// #define id_vid 0x14CD
// #define id_pid 0X1212 // micro_sdcard_reader

char buffx[1024];
int *buf_ptr;
char rd_buf[64];
int cnts = 0;
char *getchar_2str(char a);
void usb_read_string(libusb_device_handle *handle, struct libusb_endpoint_descriptor epin);
unsigned char Desc_buffer[1024];
unsigned char buffer[1024];
unsigned char i2c_mem[2];

typedef struct libusb_config_descriptor configs;
configs configDescp()
{
    struct libusb_config_descriptor conf;
    conf.bLength = Desc_buffer[0];
    conf.bDescriptorType = Desc_buffer[1];
    conf.wTotalLength = Desc_buffer[2] | (Desc_buffer[3] << 8);
    conf.bNumInterfaces = Desc_buffer[4];
    conf.bConfigurationValue = Desc_buffer[5];
    conf.iConfiguration = Desc_buffer[6];
    conf.bmAttributes = Desc_buffer[7];
    conf.MaxPower = Desc_buffer[8];
    return conf;
}
typedef struct libusb_interface_descriptor intf0;
intf0 intfDescp()
{
    struct libusb_interface_descriptor intf_0;
    intf_0.bLength = Desc_buffer[9];
    intf_0.bDescriptorType = Desc_buffer[10];
    intf_0.bInterfaceNumber = Desc_buffer[11];
    intf_0.bAlternateSetting = Desc_buffer[12];
    intf_0.bNumEndpoints = Desc_buffer[13];
    intf_0.bInterfaceClass = Desc_buffer[14];
    intf_0.bInterfaceSubClass = Desc_buffer[15];
    intf_0.bInterfaceProtocol = Desc_buffer[16];
    intf_0.iInterface = Desc_buffer[17];
    return intf_0;
}
typedef struct libusb_device_descriptor devdesc;
devdesc deviceDescp()
{
    struct libusb_device_descriptor desc;
    desc.bLength = Desc_buffer[0];
    desc.bDescriptorType = Desc_buffer[1];
    desc.bcdUSB = (Desc_buffer[2] & 0x00FF) | ((Desc_buffer[3] & 0x00FF) << 8);
    desc.bDeviceClass = Desc_buffer[4];
    desc.bDeviceSubClass = Desc_buffer[5];
    desc.bDeviceProtocol = Desc_buffer[6];
    desc.bMaxPacketSize0 = Desc_buffer[7];
    desc.idVendor = (Desc_buffer[8] & 0x00FF) | ((Desc_buffer[9] & 0x00FF) << 8);
    desc.idProduct = (Desc_buffer[10] & 0x00FF) | ((Desc_buffer[11] & 0x00FF) << 8);
    desc.bcdDevice = (Desc_buffer[12] & 0x00FF) | ((Desc_buffer[13] & 0x00FF) << 8);
    desc.iManufacturer = Desc_buffer[14];
    desc.iProduct = Desc_buffer[15];
    desc.iSerialNumber = Desc_buffer[16];
    desc.bNumConfigurations = Desc_buffer[17];
    return desc;
}
typedef struct libusb_endpoint_descriptor epdesc_in;
epdesc_in endPointDescp_in()
{
    struct libusb_endpoint_descriptor epoint;
    epoint.bLength = Desc_buffer[18];
    epoint.bDescriptorType = Desc_buffer[19];
    epoint.bEndpointAddress = Desc_buffer[20];
    epoint.bmAttributes = Desc_buffer[21];
    epoint.wMaxPacketSize = Desc_buffer[22] | (Desc_buffer[23] << 8);
    epoint.bInterval = Desc_buffer[24];
    return epoint;
}
typedef struct libusb_endpoint_descriptor epdesc_out;
epdesc_out endPointDescp_out()
{
    struct libusb_endpoint_descriptor epoint;
    epoint.bLength = Desc_buffer[25];
    epoint.bDescriptorType = Desc_buffer[26];
    epoint.bEndpointAddress = Desc_buffer[27];
    epoint.bmAttributes = Desc_buffer[28];
    epoint.wMaxPacketSize = Desc_buffer[29] | (Desc_buffer[30] << 8);
    epoint.bInterval = Desc_buffer[31];
    return epoint;
}
typedef struct libusb_endpoint_descriptor epdesc_intr;
epdesc_intr endPointDescp_intr()
{
    struct libusb_endpoint_descriptor epoint;
    epoint.bLength = Desc_buffer[32];
    epoint.bDescriptorType = Desc_buffer[33];
    epoint.bEndpointAddress = Desc_buffer[34];
    epoint.bmAttributes = Desc_buffer[35];
    epoint.wMaxPacketSize = Desc_buffer[36] | (Desc_buffer[37] << 8);
    epoint.bInterval = Desc_buffer[38];
    return epoint;
}

///////////////////////////////////////////////
typedef struct libusb_device_descriptor devdescs;
devdescs usbDevDesc(libusb_device_handle *handle, uint8_t dir, uint8_t cntrl, uint16_t windex, unsigned char *buf, uint16_t bufSize, uint8_t see)
{
    int devDesc = libusb_control_transfer(handle, dir, cntrl, DEV_DESC, windex, buf, bufSize, 0);
    if (see == 1)
    {
        printf("devDesc       %d  %s\n", devDesc, libusb_error_name(devDesc));
        printf("DEV_DESC      => ");
        for (int i = 0; i < devDesc; i++)
        {
            printf("%.2d ", buf[i]);
        }
        printf("\n\n");
    }
    else if (see == 0)
    {
    }
    return deviceDescp();
}
typedef struct libusb_config_descriptor confdescs;
confdescs usbConfDesc(libusb_device_handle *handle, uint8_t dir, uint8_t cntrl, uint16_t windex, unsigned char *buf, uint16_t bufSize)
{
    int devDesc = libusb_control_transfer(handle, dir, cntrl, CONF_DESC, windex, buf, bufSize, 0);
    printf("confDesc      %d  %s\n", devDesc, libusb_error_name(devDesc));
    printf("CONF_DESC     => ");
    for (int i = 0; i < devDesc; i++)
    {
        printf("%.2d ", buf[i]);
    }
    printf("\n\n");
    intfDescp();
    endPointDescp_in();
    endPointDescp_out();
    endPointDescp_intr();
    return configDescp();
}
void usbStrDesc(libusb_device_handle *handle, struct libusb_device_descriptor dvs)
{
    int strascii = libusb_get_string_descriptor_ascii(handle, dvs.iManufacturer, Desc_buffer, 60);
    printf("iManufacturer %d ", strascii);
    for (int i = 0; i < strascii; i++)
    {
        printf("%c", Desc_buffer[i]);
    }
    printf("\n");
    int strascii1 = libusb_get_string_descriptor_ascii(handle, dvs.iProduct, Desc_buffer, 60);
    printf("iProduct      %d ", strascii1);
    for (int i = 0; i < strascii1; i++)
    {
        printf("%c", Desc_buffer[i]);
    }
    printf("\n");
    int strascii2 = libusb_get_string_descriptor_ascii(handle, dvs.iSerialNumber, Desc_buffer, 60);
    printf("iSerialNumber %d ", strascii2);
    for (int i = 0; i < strascii2; i++)
    {
        printf("%c", Desc_buffer[i]);
    }
    printf("\n");
}
void usbStrDescIntface(libusb_device_handle *handle, struct libusb_interface_descriptor dvs)
{
    int intf = libusb_get_string_descriptor_ascii(handle, dvs.iInterface, Desc_buffer, 60);
    printf("iInterface:               %d  %s\n", intf, libusb_error_name(intf));
    for (int i = 0; i < intf; i++)
    {
        printf("%c ", Desc_buffer[i]);
    }
    printf("\n");
}

////////////////CONF2 //////
typedef struct libusb_config_descriptor confdescs2;
confdescs2 usbConfDesc2(libusb_device_handle *handle, uint8_t dir, uint8_t cntrl, uint16_t windex, unsigned char *buf, uint16_t bufSize, struct libusb_device_descriptor devdesc)
{
    if (devdesc.bNumConfigurations != 1)
    {

        int devDesc = libusb_control_transfer(handle, dir, cntrl, CONF_DESC | 0x02, windex, buf, bufSize, 0);
        printf("confDesc2      %d  %s\n", devDesc, libusb_error_name(devDesc));
        printf("CONF_DESC2     => ");
        for (int i = 0; i < devDesc; i++)
        {
            printf("%.2d ", buf[i]);
        }
        printf("\n\n");
        intfDescp();
        endPointDescp_in();
        endPointDescp_out();
        endPointDescp_intr();
    }
    return configDescp();
}

uint16_t dev_status(libusb_device_handle *handle)
{
    int devDesc;
    devDesc = libusb_control_transfer(handle, 0xC0, GetStatus, 0, 0, buffer, 4, 0);
    printf("devStatus        %d %s  ", devDesc, libusb_error_name(devDesc));
    for (int i = 0; i < devDesc; i++)
    {
        printf("%X ", buffer[i]);
    }
    printf("\n");

    return devDesc;
}
uint16_t intf_status(libusb_device_handle *handle, struct libusb_interface_descriptor intf)
{
    int devDesc = libusb_control_transfer(handle, 0xC2, GetStatus, 0, intf.bInterfaceNumber, buffer, 4, 0);
    printf("intf_status      %d %s  ", devDesc, libusb_error_name(devDesc));
    for (int i = 0; i < devDesc; i++)
    {
        printf("%X ", buffer[i]);
    }
    printf("\n");

    return devDesc;
}
uint16_t ep_in_status(libusb_device_handle *handle, struct libusb_endpoint_descriptor ep)
{
    int devDesc = libusb_control_transfer(handle, 0XC2, GetStatus, 0, ep.bEndpointAddress, buffer, 4, 0);
    printf("ep_in_status    %d  %s", devDesc, libusb_error_name(devDesc));
    for (int i = 0; i < devDesc; i++)
    {
        printf("%X ", buffer[i]);
    }
    printf("\n");
    return devDesc;
}
uint16_t ep_out_status(libusb_device_handle *handle, struct libusb_endpoint_descriptor ep)
{
    int devDesc = libusb_control_transfer(handle, 0X40, GetStatus, 0, ep.bEndpointAddress, buffer, 4, 0);
    printf("ep_out_status   %d  %s", devDesc, libusb_error_name(devDesc));
    for (int i = 0; i < devDesc; i++)
    {
        printf(" %X", buffer[i]);
    }
    printf("\n");
    return devDesc;
}
////////////////////////////////////////////////////
void dev_reset(libusb_device_handle *handle)
{
    int reset = libusb_reset_device(handle);
    printf("reset      %d %s\n\n", reset, libusb_error_name(reset));
}
int enp_size(libusb_device *device, int e_point)
{
    int pack_size = libusb_get_max_packet_size(device, e_point);
    printf("enp_size   %d %s\n", pack_size, libusb_error_name(pack_size));
    return pack_size;
}
void remote_wake(libusb_device_handle *handle, uint8_t set_get_ft)
{
    unsigned char buffwk[20];
    bzero(buffwk, 20);

    int remote_wk = libusb_control_transfer(handle, 0xC0, set_get_ft, 1, 0, buffwk, 10, 0); // texas
    printf("set remote_wk    %d  %s  ", remote_wk, libusb_error_name(remote_wk));

    for (int i = 0; i < remote_wk; i++)
    {
        printf("%d", buffwk[i]);
    }
    printf("\n");
}
int clear_halt(libusb_device_handle *handle, struct libusb_endpoint_descriptor epin)
{
    int halt = libusb_clear_halt(handle, epin.bEndpointAddress);
    printf("halt            %d  %s\n", halt, libusb_error_name(halt));
    return halt;
}
///////////////////////////
void usb_read_string(libusb_device_handle *handle, struct libusb_endpoint_descriptor epin)
{
    int datalen[epin.wMaxPacketSize];

    clear_halt(handle, epin);
    bzero(rd_buf, epin.wMaxPacketSize);
    int rd_str = libusb_bulk_transfer(handle, epin.bEndpointAddress, rd_buf, epin.wMaxPacketSize, datalen, 0);
    printf("readRx =        %d %s\n", rd_str, libusb_error_name(rd_str));
    printf("rd_buf = ");

    for (int i = 0; i < strlen(rd_buf); i++)
    {
        printf("%d ", rd_buf[i]);
    }
    printf("\n");
}
void usb_send_string(libusb_device_handle *handle, struct libusb_endpoint_descriptor epout, struct libusb_device_descriptor devDesc, char str[])
{
    // libusb_bulk_transfer(ftdi->usb_dev, ftdi->in_ep, (unsigned char *)buf+offset, write_size, &actual_length, ftdi->usb_write_timeout)
    int tx;
    printf("****** Wait Sending**********\n\n");
    printf("tx:\n");
    for (int i = 0; i < strlen(str); i++)
    {
        // clear_halt(handle, epout);
        if (libusb_bulk_transfer(handle, epout.bEndpointAddress, getchar_2str(str[i]), 1, 0, 0) != 0)
        {
            printf("send error");
            exit(0);
        }
        printf("%c ", str[i]);
        usleep(150000);
    }
    printf("\n");
    printf("\n****** Send Complete**********\n\n");
}
///////////////////////////
char *getchar_2str(char a)
{
    char data[2] = {a, '\n'};
    strcpy(buffx, data);
    return buffx;
}
///////////////////////////////////////////////////////
int usb_file_read(libusb_device_handle *handle, struct libusb_endpoint_descriptor epin, struct libusb_device_descriptor devDesc, char filename[])
{
    char buff[2048];
    int fd;
    fd = open(filename, 'r', R_OK | W_OK | F_OK);
    int rd = read(fd, buff, sizeof(buff));
    printf("size=       %d   %d\n", rd, cnts);
    printf("strlen=     %ld\n", strlen(buff));
    printf("size-buff=  %ld\n", sizeof(buff));

    usb_send_string(handle, epin, devDesc, buff);

    cnts++;
    close(fd);
}
int usb_file_stream_write(char filename[], int num)
{
    FILE *fd;
    fd = fopen(filename, "w");
    freopen(filename, "w", fd);
    fwrite(filename, 0, 1, fd);
    fprintf(fd, "%d", num);
    fflush(fd);
    fclose(fd);
}
/////////////////////////////

//////////
typedef libusb_device_handle *devs;
devs usbGetaLLDevices()
{
    struct libusb_device **list, *device;
    struct libusb_config_descriptor config;
    struct libusb_interface_descriptor intfs;
    struct libusb_device_descriptor devDesc;
    struct libusb_endpoint_descriptor endpin, endpout, endpintr;

    libusb_device_handle *handle;
    libusb_init(0);
    int devlist = libusb_get_device_list(0, &list);
    printf("List of devices detected=  %d\n\n", devlist);

    for (int i = 0; i < devlist; i++)
    {
        device = libusb_ref_device(list[i]);
        int opendev = libusb_open(list[i], &handle);
        if (opendev != 0)
        {
            exit(0);
        }
        devDesc = usbDevDesc(handle, 0x80, 0x06, 0, Desc_buffer, 18, 1);
        printf("vendor_id= 0x%.4x found\n", devDesc.idVendor);
        usbStrDesc(handle, devDesc);
        printf("\n");
        libusb_unref_device(device);
        libusb_close(handle);
    }
    libusb_exit(0);
}
typedef libusb_device_handle *dev;
dev usbGetDevice(uint16_t idv)
{
    struct libusb_device **list, *device;
    struct libusb_config_descriptor config;
    struct libusb_interface_descriptor intfs;
    struct libusb_device_descriptor devDesc;
    struct libusb_endpoint_descriptor endpin, endpout, endpintr;

    libusb_device_handle *handle;
    libusb_init(0);
    int Mydevice = libusb_get_device_list(0, &list);
    for (int i = 0; i < Mydevice; i++)
    {
        device = libusb_ref_device(list[i]);
        int opendev = libusb_open(list[i], &handle);
        if (opendev != 0)
        {
            exit(0);
        }
        devDesc = usbDevDesc(handle, 0x80, 0x06, 0, Desc_buffer, 18, 0);
        if (devDesc.idVendor == idv)
        {
            uint8_t busNum = libusb_get_bus_number(list[i]);
            printf("busNum=   %d\n", busNum);
            uint8_t devAddr = libusb_get_device_address(list[i]);
            printf("devAddr=  %d\n", devAddr);
            int devSpeed = libusb_get_device_speed(list[i]);
            printf("devSpeed= %d\n", devSpeed);
            uint8_t portNum = libusb_get_port_number(list[i]);
            printf("portNum=  %d\n", portNum);
            return handle;
        }
        libusb_unref_device(device);
        // libusb_close(handle);
    }
    // libusb_exit(0);
}
#endif // __USB101