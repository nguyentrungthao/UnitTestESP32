// #include <AnhLABV01HardWare.h>
// #include <FifoBuffer.h>
#include <vector>
// #include <esp_task_wdt.h>
#include "HMI.h"

#define HMI_RX_PIN 16
#define HMI_TX_PIN 17

HMI _hmi(Serial2, HMI_RX_PIN, HMI_TX_PIN);

// 2 hàm được gọi task HMI TOUCH có độ ưu tiên cao
void hmiSetEvent(const hmi_set_event_t &event) {
  Serial.printf("%s %d\n", __func__, event.type);

  switch (event.type) {
    case HMI_SET_SETPOINT_TEMP:
      Serial.printf("Set setpoint temp: %.1f\n", event.f_value);
      break;
    case HMI_SET_SETPOINT_CO2:
      Serial.printf("Set setpoint CO2: %d\n", event.u32_value);
      break;
    default:
      Serial.printf("%s nhận error case %d", __func__, event.type);
      break;
  }
}
// 2 hàm được gọi task HMI TOUCH có độ ưu tiên cao
bool hmiGetEvent(hmi_get_type_t event, void *args) {
  switch (event) {
    case HMI_GET_FAN:
      // lấy thời gian từ DS3231, hoặc cảm biến, thông số lưu trong EEPROM, ....
      _hmi.setText(_VPAddressDelayOffText, "hello");
      count(_VPAddressFanSpeedText);
      break;
    default:
      Serial.printf("%s nhận error case %d\n", __func__, event);
      break;
  }
  return 0;
}

void setup() {
  Serial.begin(115200);
  _hmi.KhoiTao(10240);
  _hmi.crcEnabled(true);                   // lưu ý có dùng CRC không
  _hmi.DangKyHamSetCallback(hmiSetEvent);  // được gọi qua hàm enter
  _hmi.DangKyHamGetCallback(hmiGetEvent);  // được gọi qua hàm

  _hmi.setText(_VPAddressTemperatureText, "0");
  _hmi.setText(_VPAddressCO2Text, "500");
  _hmi.setText(_VPAddressSetpointTempText, "0");
  _hmi.setText(_VPAddressSetpointCO2Text, "500");
  _hmi.setText(_VPAddressGraphYValueText1, "0");
  _hmi.setText(_VPAddressGraphYValueText2, "500");
  _hmi.setText(_VPAddressGraphYValueText3, "0");
  _hmi.setText(_VPAddressGraphYValueText4, "500");
  _hmi.setText(_VPAddressGraphYValueText5, "0");
  _hmi.setText(_VPAddressGraphYValueText6, "500");
  _hmi.setText(_VPAddressGraphY_R_ValueText1, "0");
  _hmi.setText(_VPAddressGraphY_R_ValueText2, "500");
  _hmi.setText(_VPAddressGraphY_R_ValueText3, "0");
  _hmi.setText(_VPAddressGraphY_R_ValueText4, "500");
  _hmi.setText(_VPAddressGraphY_R_ValueText5, "0");
  _hmi.setText(_VPAddressGraphY_R_ValueText6, "500");
  _hmi.setPage(71);

  

}

void count(uint16_t VP) {
  String str = _hmi.getText(VP, 5);
  int a = str.toInt();
  a++;
  _hmi.setText(VP, (String)a);
}
void loop() {
  Serial.printf("--------------------------------------\n");
  Serial.println(_hmi.getPage());
  count(_VPAddressTemperatureText);
  count(_VPAddressCO2Text);
  count(_VPAddressSetpointTempText);
  count(_VPAddressSetpointCO2Text);
  count(_VPAddressGraphYValueText1);
  count(_VPAddressGraphYValueText2);
  count(_VPAddressGraphYValueText3);
  count(_VPAddressGraphYValueText4);
  count(_VPAddressGraphYValueText5);
  count(_VPAddressGraphYValueText6);
  count(_VPAddressGraphY_R_ValueText1);
  count(_VPAddressGraphY_R_ValueText2);
  count(_VPAddressGraphY_R_ValueText3);
  count(_VPAddressGraphY_R_ValueText4);
  count(_VPAddressGraphY_R_ValueText5);
  count(_VPAddressGraphY_R_ValueText6);
  delay(1);
}