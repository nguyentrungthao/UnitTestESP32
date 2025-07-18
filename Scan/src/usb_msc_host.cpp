#include "usb_msc_host.h"

static const char* TAG = "USB_FLASH";


using namespace fs;
usb_host_config_t USBMSCHOST::host_config = { 
    .skip_phy_setup = false,
    .intr_flags = ESP_INTR_FLAG_LEVEL1 
};

msc_host_driver_config_t USBMSCHOST::msc_config = {
    .create_backround_task = true,
    .task_priority = 5,
    .stack_size = 4096,
};
USBMSCHOST::USBMSCHOST(FSImplPtr impl) : FS(impl) 
{

}

// Chưa hoạt động ổn Khi gọi nhiều lần
bool USBMSCHOST::begin()
{
    // Create FreeRTOS primitives
    if(flagBeginUsbMsc == false) {
        flagBeginUsbMsc = true;
        if(this->usb_flash_queue == NULL) {
            this->usb_flash_queue = xQueueCreate(5, sizeof(usb_message_t));
            ESP_LOGI(TAG, "Create: usb_flash_queue");
            assert(this->usb_flash_queue);
        }
        
        if(this->usb_background_task == NULL) {
            xTaskCreate(usb_background_task_cb, "usb_background_task_cb", 4096, (void*)this, 3, &this->usb_background_task);
            ESP_LOGI(TAG, "Create: usb_background_task_cb");
            assert(this->usb_background_task);
        }

        ESP_LOGI(TAG, "Waiting for USB flash drive to be connected");
        if(this->usb_handle_task == NULL) {
            xTaskCreate(usb_handle_task_cb, "usb_handle_task_cb", 4096, (void*)this, 2, &this->usb_handle_task);
            ESP_LOGI(TAG, "Create: usb_handle_task_cb");
            assert(this->usb_handle_task);
        }
        ESP_LOGI(TAG, "Usb flash installed");
        this->usb_state.id = USB_UNREADY;
        return 1;
    }
    else {
        return 0;
    }
}

// Chưa hoạt động ổn Khi gọi nhiều lần
void USBMSCHOST::end(void) 
{
    static usb_message_t message;
    message.id = USB_QUIT;
    if (this->usb_flash_queue && flagBeginUsbMsc == true) {
        flagBeginUsbMsc = false;
        xQueueSend(this->usb_flash_queue, &message, portMAX_DELAY);
    }
}

bool USBMSCHOST::isConnected()
{
    if (usb_state.id == USB_READY) return true;
    else return false;
}

void USBMSCHOST::msc_event_cb(const msc_host_event_t* event, void* arg)
{
    USBMSCHOST* usb_msc_host_ptr = static_cast<USBMSCHOST*>(arg);
    // usb_message_t message;
    if (event->event == MSC_DEVICE_CONNECTED) {
        ESP_LOGI(TAG, "MSC device connected");
        usb_msc_host_ptr->usb_state.id = USB_DEVICE_CONNECTED;
        usb_msc_host_ptr->usb_state.data.new_dev_address = event->device.address;
        xQueueSend(usb_msc_host_ptr->usb_flash_queue, &usb_msc_host_ptr->usb_state, portMAX_DELAY);

    }
    else if (event->event == MSC_DEVICE_DISCONNECTED) {
        ESP_LOGI(TAG, "MSC device disconnected");
        usb_msc_host_ptr->usb_state.id = USB_DEVICE_DISCONNECTED;
        xQueueSend(usb_msc_host_ptr->usb_flash_queue, &usb_msc_host_ptr->usb_state, portMAX_DELAY);
    }
}

void USBMSCHOST::print_device_info(msc_host_device_info_t* info)
{
    const size_t megabyte = 1024 * 1024;
    uint64_t capacity = ((uint64_t)info->sector_size * info->sector_count) / megabyte;

    printf("Device info:\n");
    printf("\t Capacity: %llu MB\n", capacity);
    printf("\t Sector size: %"PRIu32"\n", info->sector_size);
    printf("\t Sector count: %"PRIu32"\n", info->sector_count);
    printf("\t PID: 0x%04X \n", info->idProduct);
    printf("\t VID: 0x%04X \n", info->idVendor);
    wprintf(L"\t iProduct: %S \n", info->iProduct);
    wprintf(L"\t iManufacturer: %S \n", info->iManufacturer);
    wprintf(L"\t iSerialNumber: %S \n", info->iSerialNumber);
}

void USBMSCHOST::usb_handle_task_cb(void* pvParameters)
{
    USBMSCHOST* usb_msc_host_ptr = static_cast<USBMSCHOST*>(pvParameters);

    // msc_host_device_handle_t msc_device = NULL;
    // msc_host_vfs_handle_t vfs_handle = NULL;
    while (1) {
        usb_message_t msg;
        xQueueReceive(usb_msc_host_ptr->usb_flash_queue, &msg, portMAX_DELAY);

        if (msg.id == USB_DEVICE_CONNECTED) {
        	vTaskDelay(pdMS_TO_TICKS(100));
            // 1. MSC flash drive connected. Open it and map it to Virtual File System
            if (msc_host_install_device(msg.data.new_dev_address, &(usb_msc_host_ptr->msc_device)) == ESP_OK)
            {
                const esp_vfs_fat_mount_config_t mount_config = {
                    .format_if_mount_failed = false,
                    .max_files = 2,
                    .allocation_unit_size = 8192,
                };
                if (msc_host_vfs_register(usb_msc_host_ptr->msc_device, MNT_PATH, &mount_config, &(usb_msc_host_ptr->vfs_handle)) == ESP_OK)
                {
                    // 2. Print information about the connected disk
                    ESP_ERROR_CHECK(msc_host_get_device_info(usb_msc_host_ptr->msc_device, &(usb_msc_host_ptr->usb_info)));
                    msc_host_print_descriptors(usb_msc_host_ptr->msc_device);
                    print_device_info(&(usb_msc_host_ptr->usb_info));
                    usb_msc_host_ptr->usb_state.id = USB_READY;
                    usb_msc_host_ptr->_impl->mountpoint(MNT_PATH);
                    ESP_LOGI(TAG, "The disk is mounted to Virtual File System");
                }
            }
        }
        if ((msg.id == USB_DEVICE_DISCONNECTED) || (msg.id == USB_QUIT)) {
            if (usb_msc_host_ptr->vfs_handle) {
                ESP_LOGI(TAG, "msc_host_vfs_unregister");
                ESP_ERROR_CHECK(msc_host_vfs_unregister(usb_msc_host_ptr->vfs_handle));
                usb_msc_host_ptr->vfs_handle = NULL;
                usb_msc_host_ptr->_impl->mountpoint(NULL); 
            }
            if (usb_msc_host_ptr->msc_device) {
                ESP_LOGI(TAG, "msc_host_uninstall_device");
                ESP_ERROR_CHECK(msc_host_uninstall_device(usb_msc_host_ptr->msc_device));
                usb_msc_host_ptr->msc_device = NULL;
            }
            if (msg.id == USB_QUIT) {
                // This will cause the usb_task to exit
                ESP_LOGI(TAG, "msc_host_uninstall");
                ESP_ERROR_CHECK(msc_host_uninstall());
                break;
            }
        }
        // size_t stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        // printf("Task RAM Usage: %u bytes\n", configMINIMAL_STACK_SIZE - stackHighWaterMark);
    }
    vTaskDelay(10);
    // ESP_LOGI(TAG, "Done");
    vQueueDelete(usb_msc_host_ptr->usb_flash_queue);    
    usb_msc_host_ptr->usb_flash_queue = NULL;
    usb_msc_host_ptr->usb_handle_task = NULL;
    vTaskDelete(NULL);
}

void USBMSCHOST::usb_background_task_cb(void* agrs)
{
    USBMSCHOST* usb_msc_host_ptr = static_cast<USBMSCHOST*>(agrs);
    static bool flagInstallUSB = false;
    if(flagInstallUSB == false) {
        ESP_ERROR_CHECK(usb_host_install((const usb_host_config_t*)&usb_msc_host_ptr->host_config));
        flagInstallUSB = true;
    }

    usb_msc_host_ptr->msc_config.callback = msc_event_cb;
    usb_msc_host_ptr->msc_config.callback_arg = agrs;
    ESP_ERROR_CHECK(msc_host_install((const msc_host_driver_config_t *)&usb_msc_host_ptr->msc_config));

    bool has_clients = true;
    while (1) {
        uint32_t event_flags;
		usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        // Release devices once all clients has deregistered
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            has_clients = false;
            ESP_LOGI(TAG, "has_clients = false");
            if(usb_msc_host_ptr->usb_state.id == USB_DEVICE_DISCONNECTED || usb_msc_host_ptr->usb_state.id == USB_UNREADY)
            {
                if (usb_host_device_free_all() == ESP_OK) {
                    ESP_LOGI(TAG, "usb_host_device_free_all");
                    break;
                }
            }
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE && !has_clients) {
            break;
        }
    }
    vTaskDelay(10); // Give clients some time to uninstall
    ESP_LOGI(TAG, "Deinitializing USB");
    flagInstallUSB = false;
    ESP_ERROR_CHECK(usb_host_uninstall());
    ESP_LOGI(TAG, "usb_host_uninstall");
    usb_msc_host_ptr->usb_background_task = NULL;
    vTaskDelete(NULL);
}

USBMSCHOST USB_MSC_HOST = USBMSCHOST(FSImplPtr(new VFSImpl()));