/**
 * @file usbHostMaster.h
 * @author Trung Thao (nguyentrungthao1412@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-07-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef USB_HOST_MASTER
#define USB_HOST_MASTER

#include "usb_msc_host.h"
#include "EspUsbHost.h"

class usbHostMaster : public EspUsbHost, public USBMSCHOST
{
public:
    void begin();
    // khởi tạo usb host 
    void vInitDriverUSB();
    // 
private:
    usb_host_client_handle_t client_hdl;
    usb_device_handle_t dev_hdl;
    uint16_t u16SysFlag = 0;
    uint8_t usbClass = 0xFF;
    fs::USBMSCHOST USB_MSC_HOST = fs::USBMSCHOST(FSImplPtr(new VFSImpl()));
    TaskHandle_t xTaskSDCardhdl = NULL;
};

#endif // USB_HOST_MASTER