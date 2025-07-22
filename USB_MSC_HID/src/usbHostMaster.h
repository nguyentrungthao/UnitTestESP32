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
private:
};

#endif // USB_HOST_MASTER