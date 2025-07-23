#include "11_ScannerPrinter.h"
#include "src/usb_msc_host.h"
#include "src/espUsbHost.h"
#include "FS.h"

#define TAG "MAIN"

bool checkMSCDevice(const usb_intf_desc_t *intf)
{
  const uint8_t SCSI_COMMAND_SET = 0x06;
  const uint8_t BULK_ONLY_TRANSFER = 0x50;
  return (intf->bInterfaceClass == USB_CLASS_MASS_STORAGE) &&
         (intf->bInterfaceSubClass == SCSI_COMMAND_SET) &&
         (intf->bInterfaceProtocol == BULK_ONLY_TRANSFER);
}
bool checkHIDDevice(const usb_intf_desc_t *intf)
{
  return (intf->bInterfaceClass == USB_CLASS_HID) &&
         (intf->bInterfaceSubClass == HID_SUBCLASS_BOOT) &&
         (intf->bInterfaceProtocol == HID_ITF_PROTOCOL_KEYBOARD);
}

usb_host_client_handle_t client_hdl;
usb_device_handle_t dev_hdl;
uint16_t u16SysFlag = 0;
uint8_t usbClass = 0xFF;

ScannerPrinter xScannerPrinter;
fs::USBMSCHOST USB_MSC_HOST = fs::USBMSCHOST(FSImplPtr(new VFSImpl()));

void vInitDriverUSB()
{
  const usb_host_config_t host_config = {
      .skip_phy_setup = false, // false => khởi tạo PHY USB internal
      .intr_flags = ESP_INTR_FLAG_LEVEL1,
  };
  esp_err_t err = usb_host_install(&host_config);
  if (err != ESP_OK)
  {
    ESP_LOGE("MAIN", "usb_host_install() failed: %s", esp_err_to_name(err));
    return;
  }
  ESP_LOGI("MAIN", "USB Host installed");
}
void vPrintDeviceInfo()
{
  usb_device_info_t dev_info;
  esp_err_t err = usb_host_device_info(dev_hdl, &dev_info);
  if (err != ESP_OK)
  {
    ESP_LOGI("MAIN", "usb_host_device_info() err=%x", err);
  }
  else
  {
    ESP_LOGI("MAIN", "usb_host_device_info() ESP_OK\n"
                     "# speed                 = %d\n"
                     "# dev_addr              = %d\n"
                     "# vMaxPacketSize0       = %d\n"
                     "# bConfigurationValue   = %d\n"
                     "# str_desc_manufacturer = \"%s\"\n"
                     "# str_desc_product      = \"%s\"\n"
                     "# str_desc_serial_num   = \"%s\"",
             dev_info.speed,
             dev_info.dev_addr,
             dev_info.bMaxPacketSize0,
             dev_info.bConfigurationValue,
             getUsbDescString(dev_info.str_desc_manufacturer).c_str(),
             getUsbDescString(dev_info.str_desc_product).c_str(),
             getUsbDescString(dev_info.str_desc_serial_num).c_str());
  }
}
void vPrintDeviceDescription()
{
  const usb_device_desc_t *dev_desc;
  esp_err_t err = usb_host_get_device_descriptor(dev_hdl, &dev_desc);
  if (err != ESP_OK)
  {
    ESP_LOGI("MAIN", "usb_host_get_device_descriptor() err=%x", err);
  }
  else
  {
    ESP_LOGI("MAIN", "usb_host_get_device_descriptor() ESP_OK\n"
                     "#### DESCRIPTOR DEVICE ####\n"
                     "# bLength            = %d\n"
                     "# bDescriptorType    = %d\n"
                     "# bcdUSB             = 0x%x\n"
                     "# bDeviceClass       = 0x%x\n"
                     "# bDeviceSubClass    = 0x%x\n"
                     "# bDeviceProtocol    = 0x%x\n"
                     "# bMaxPacketSize0    = %d\n"
                     "# idVendor           = 0x%x\n"
                     "# idProduct          = 0x%x\n"
                     "# bcdDevice          = 0x%x\n"
                     "# iManufacturer      = %d\n"
                     "# iProduct           = %d\n"
                     "# iSerialNumber      = %d\n"
                     "# bNumConfigurations = %d",
             dev_desc->bLength,
             dev_desc->bDescriptorType,
             dev_desc->bcdUSB,
             dev_desc->bDeviceClass,
             dev_desc->bDeviceSubClass,
             dev_desc->bDeviceProtocol,
             dev_desc->bMaxPacketSize0,
             dev_desc->idVendor,
             dev_desc->idProduct,
             dev_desc->bcdDevice,
             dev_desc->iManufacturer,
             dev_desc->iProduct,
             dev_desc->iSerialNumber,
             dev_desc->bNumConfigurations);
  }
}
void general_client_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg)
{
  esp_err_t err;
  static const usb_config_desc_t *config_desc;
  static const usb_intf_desc_t *intf;
  switch (event_msg->event)
  {
  case USB_HOST_CLIENT_EVENT_NEW_DEV:
    ESP_LOGI("MAIN", "USB_HOST_CLIENT_EVENT_NEW_DEV new_dev.address=%d", event_msg->new_dev.address);
    err = usb_host_device_open(client_hdl, event_msg->new_dev.address, &dev_hdl);
    if (err != ESP_OK)
    {
      ESP_LOGI("MAIN", "usb_host_device_open() err=%x", err);
      return;
    }
    else
    {
      ESP_LOGI("MAIN", "usb_host_device_open() ESP_OK");
    }
    vPrintDeviceInfo();
    vPrintDeviceDescription();

    err = usb_host_get_active_config_descriptor(dev_hdl, &config_desc);
    if (err != ESP_OK)
    {
      ESP_LOGI("MAIN", "usb_host_get_active_config_descriptor() err=%x", err);
    }
    else
    {
      const uint8_t *p = &config_desc->val[0];
      uint8_t bLength;
      for (int i = 0; i < config_desc->wTotalLength; i += bLength, p += bLength)
      {
        bLength = p[0]; // vị trí đầu tiên là độ dài của data
        if ((i + bLength) > config_desc->wTotalLength)
        {
          break;
        }

        const uint8_t bDescriptorType = *(p + 1);
        if (bDescriptorType == USB_INTERFACE_DESC)
        {
          intf = (const usb_intf_desc_t *)p;
          ESP_LOGI("MAIN", "USB_INTERFACE_DESC(0x04)\n"
                           "# bLength            = %d\n"
                           "# bDescriptorType    = %d\n"
                           "# bInterfaceNumber   = %d\n"
                           "# bAlternateSetting  = %d\n"
                           "# bNumEndpoints      = %d\n"
                           "# bInterfaceClass    = 0x%x\n"
                           "# bInterfaceSubClass = 0x%x\n"
                           "# bInterfaceProtocol = 0x%x\n"
                           "# iInterface         = %d",
                   intf->bLength,
                   intf->bDescriptorType,
                   intf->bInterfaceNumber,
                   intf->bAlternateSetting,
                   intf->bNumEndpoints,
                   intf->bInterfaceClass,
                   intf->bInterfaceSubClass,
                   intf->bInterfaceProtocol,
                   intf->iInterface);
        }
      }
    }
    err = usb_host_device_close(client_hdl, dev_hdl);
    if (err != ESP_OK)
    {
      ESP_LOGI("MAIN", "usb_host_device_close() err=%x", err);
      return;
    }
    else
    {
      ESP_LOGI("MAIN", "usb_host_device_close() ESP_OK");
    }

    if (checkMSCDevice(intf))
    {
      // khởi tạo msc
      ESP_LOGI("MAIN", "\t\tcall msc_client_event_cb");
      msc_setup_client_handle(client_hdl);
      msc_client_event_cb(event_msg, arg);
    }
    else if (checkHIDDevice(intf))
    {
      ESP_LOGI("MAIN", "\t\tcall hid_client_event_cb");
      // HID
    }

    break;
  case USB_HOST_CLIENT_EVENT_DEV_GONE:
    ESP_LOGI("MAIN", "USB_HOST_CLIENT_EVENT_DEV_GONE new_dev.address=%d", event_msg->new_dev.address);
    if (checkMSCDevice(intf))
    {
      // khởi tạo msc
      ESP_LOGI("MAIN", "\t\tcall msc_client_event_cb");
      msc_client_event_cb(event_msg, arg);
      msc_clear_client_handle();
    }
    else if (checkHIDDevice(intf))
    {
      ESP_LOGI("MAIN", "\t\tcall hid_client_event_cb");
      // HID
    }
    break;
  default:
    ESP_LOGI("MAIN", "%s default %d", __func__, event_msg->event);
    break;
  }
}
void vInitGeneralClient()
{
  const usb_host_client_config_t client_config = {
      .is_synchronous = false, // Nên dùng asynchronous cho callback
      .max_num_event_msg = 10,
      .async = {
          .client_event_callback = general_client_event_callback,
          .callback_arg = NULL, // Có thể truyền con trỏ this của UsbManager nếu bạn tạo lớp
      }};
  esp_err_t err = usb_host_client_register(&client_config, &client_hdl); // Bạn sẽ cần một clientHandle_global
  if (err != ESP_OK)
  {
    ESP_LOGE("MAIN", "usb_host_client_register() failed: %s", esp_err_to_name(err));
    return;
  }
  ESP_LOGI("MAIN", "USB Host client registered");
}
String getUsbDescString(const usb_str_desc_t *str_desc)
{
  String str = "";
  if (str_desc == NULL)
  {
    return str;
  }

  for (int i = 0; i < str_desc->bLength / 2; i++)
  {
    if (str_desc->wData[i] > 0xFF)
    {
      continue;
    }
    str += char(str_desc->wData[i]);
  }
  return str;
}
void vReadInfoDevice(const usb_host_client_event_msg_t *event_msg, void *arg)
{
}

void vUSBDeamonTask(void *ptr)
{
  if (NULL == ptr)
  {
    ESP_LOGE("MAIN", "%s ptr == NULL", __func__);
  }
  vTaskSuspend((TaskHandle_t)ptr); // đồng bộ khởi tạo driver USB thành công trước khi đăng khí client
  vInitDriverUSB();
  vTaskResume((TaskHandle_t)ptr);

  bool has_clients = true;
  bool has_devices = true;
  while (has_devices || has_clients)
  {
    uint32_t event_flags;
    usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
    // Release devices once all clients has deregistered
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS)
    {
      has_clients = false;
      ESP_LOGI(TAG, "has_clients = false");
      if (usb_host_device_free_all() == ESP_OK)
      {
        ESP_LOGI(TAG, "usb_host_device_free_all");
      }
    }
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE && !has_clients)
    {
      ESP_LOGI(TAG, "has_devices = false");
      has_devices = false;
    }
  }
  vTaskDelay(10); // Give clients some time to uninstall
  ESP_LOGI(TAG, "Deinitializing USB");
  // flagInstallUSB = false;
  ESP_ERROR_CHECK(usb_host_uninstall());
  ESP_LOGI(TAG, "usb_host_uninstall");
  // usb_msc_host_ptr->usb_background_task = NULL;
  vTaskDelete(NULL);
}

void vUSBClient(void *)
{
  vInitGeneralClient();
  while (1)
  {
    usb_host_client_handle_events(client_hdl, portMAX_DELAY);
  }
}

const char *pcFilePath = "/hello.csv";
void setup()
{
  Serial.begin(115200);
  xTaskCreate(vUSBDeamonTask, "USB deamon", 8096, xTaskGetCurrentTaskHandle(), 2, NULL);
  xTaskCreate(vUSBClient, "USB Client", 8096, NULL, 2, NULL);

  USB_MSC_HOST.begin();
  delay(1000);

  File file;
  file = USB_MSC_HOST.open(pcFilePath, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }

  file.write((const uint8_t *)"hello", strlen("hello"));

  file.close();
}

void loop()
{
}
