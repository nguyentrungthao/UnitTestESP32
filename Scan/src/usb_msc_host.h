#ifndef _USB_MSC_HOST_ESP32S3_
#define _USB_MSC_HOST_ESP32S3_

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <dirent.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "usb/usb_host.h"
#include "msc_host.h"
#include "msc_host_vfs.h"
#include "ffconf.h"
#include "errno.h"
#include "driver/gpio.h"
#include "vfs_api.h"

#include "FS.h"

#define MNT_PATH         "/usb"     // Path in the Virtual File System, where the USB flash drive is going to be mounted
#define BUFFER_SIZE      4096       // The read/write performance can be improved with larger buffer for the cost of RAM, 4kB is enough for most usecases
typedef enum {
    USB_UNREADY,
    USB_READY,
    USB_QUIT,                // Signals request to exit the application
    USB_DEVICE_CONNECTED,    // USB device connect event
    USB_DEVICE_DISCONNECTED, // USB device disconnect event
} usb_state_t;
  

typedef struct {
    usb_state_t id;
    union {
        uint8_t new_dev_address; // Address of new USB device for APP_DEVICE_CONNECTED event if
    } data;
} usb_message_t;

namespace fs
{
class USBMSCHOST : public FS {

public:
    USBMSCHOST(FSImplPtr impl);
    
    bool begin();
    bool isConnected();
    void end();

    static void usb_background_task_cb(void* agrs);
    static void usb_handle_task_cb(void* pvParameters);
    TaskHandle_t usb_background_task = NULL;
    TaskHandle_t usb_handle_task = NULL;
    static void msc_event_cb(const msc_host_event_t* event, void* arg);
    static void print_device_info(msc_host_device_info_t* info);
    QueueHandle_t usb_flash_queue = NULL;
    usb_message_t usb_state;
    msc_host_device_info_t usb_info;
    msc_host_device_handle_t msc_device = NULL;
    msc_host_vfs_handle_t vfs_handle = NULL;
    static usb_host_config_t host_config;
    static msc_host_driver_config_t msc_config;
    bool flagBeginUsbMsc = false;
};
}

extern fs::USBMSCHOST USB_MSC_HOST;

#endif