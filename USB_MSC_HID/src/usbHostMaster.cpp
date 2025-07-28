
#include "usbHostMaster.h"

void usbHostMaster::vInitDriverUSB()
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


