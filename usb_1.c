#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "usb_1.h"
#include "myusb.h"

extern unsigned char buffer[1024];
extern unsigned char Desc_buffer[1024];
unsigned char *rd_buffer = 0;
int *actLen;
uint16_t cnt = 0;
int *rdbuff;

void usb_device(uint16_t idv)
{
    struct libusb_device **list, *device;
    struct libusb_device_descriptor devDesc;
    struct libusb_config_descriptor config, **config_ptr;
    struct libusb_interface_descriptor intfs;
    struct libusb_endpoint_descriptor endpin, endpout, endpintr;
    struct libusb_transfer *transfer;
    libusb_device_handle *handle;

    handle = usbGetDevice(idv);
    if (handle == NULL)
    {
        printf("\n\n");
        printf("      CAUTION!! \n");
        printf("Device vendor-Id 0x%.4X not found\n", idv);
        printf("Make sure you are root `sudo-s`... ");
        printf("and/or input a correct device  Vendor-Id \n\n");
        exit(0);
    }
    devDesc = usbDevDesc(handle, 0x80, LIBUSB_REQUEST_GET_DESCRIPTOR, 0, Desc_buffer, 50, 1);
    config = usbConfDesc(handle, 0x80, LIBUSB_REQUEST_GET_DESCRIPTOR, 0, Desc_buffer, 50);
    intfs = intfDescp();
    endpin = endPointDescp_in();
    endpout = endPointDescp_out();
    endpintr = endPointDescp_intr();
    usbStrDesc(handle, devDesc);
    // getDev_desc(devDesc);
    // getConf_desc(config);
    // getIntf_desc(intfs);
    // get_Ep_In(endpin);
    if (devDesc.bNumConfigurations >= 1) // conf desc 2
    {
        // usbConfDesc2(handle, 0x80, LIBUSB_REQUEST_GET_DESCRIPTOR, 0, Desc_buffer, 50, devDesc);
        // config = configDescp();
        // intfs = intfDescp();
        // endpin = endPointDescp_in();
        // endpout = endPointDescp_out();
        // endpintr = endPointDescp_intr();

        // getConf_desc(config);
        // getIntf_desc(intfs);
        // get_Ep_In(endpin);
        // get_Ep_Out(endpout);
        // get_Ep_Intr(endpintr);
    }
    if (libusb_kernel_driver_active(handle, intfs.bInterfaceNumber) == 1) // 0-active 1-not active
    {
        if (libusb_detach_kernel_driver(handle, intfs.bInterfaceNumber) != 0)
            printf("auto detach() Error\n\n");
    }
    if (libusb_claim_interface(handle, intfs.bInterfaceNumber) == 0)
        printf("\n********interface free******** \n\n");

    /// UNCOMMENT FUNCTION_CALLS ////

    usb_setBitmode(handle, 0x00, SET_BITMODE);

    // ep_in_status(handle,endpin);
    // intf_status(handle,intfs);
    // ep_out_status(handle,endpout);
    // dev_status(handle);
    // usb_send_string(handle, endpout, devDesc, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    // clear_halt(handle,endpin);
    // usb_read_string(handle, endpin);
    read_pin(handle, endpin,buffer);

    libusb_close(handle);
    printf("\nEnd: \n");
    libusb_exit(0);
}

int main(int argc, char const *argv[])
{
    usb_device(id_vid);
    // usbGetDevice(id_vid);
    // usbGetaLLDevices();

    return 0;
}
