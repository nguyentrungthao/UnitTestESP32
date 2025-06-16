#include "DWIN.h"
#include "AnhLABV01HardWare.h"

#define _VPAddressCacNutNhan 0x5000

// Keyvalue của từng loại nút nhấn Menu hoặc nút nhấn cài đặt giá trị TRỪ nút Enter
#define _KeyValueSetSetpointTemp 0x01
#define _KeyValueSetFanSpeed 0x02
#define _KeyValueSetDelayOff 0x03
#define _KeyValueZoomGraph 0x04
#define _KeyValueSettings 0x05
#define _KeyValueProgram 0x06
#define _KeyValueAlarm 0x07
#define _KeyValueSterilization 0x08
#define _KeyValueDataRecord 0x09
#define _KeyValueCalib 0x0A
#define _KeyValueSetTimeRTC 0x0B
#define _KeyValueInformation 0x0C
//* key trong trang _CalibTempPage
#define _KeyValueEditCalibTemp 0x0D
#define _KeyValueEnterCalibTemp 0x0E
#define _VPAddressTemperatureText 0x8019   // Length text 5
#define _VPAddressCO2Text 0xAFFF           // 5 ky tu
#define _VPAddressSetpointTempText 0x8000  // Length text 5
#define _VPAddressFanSpeedText 0x8005      // Length text 5
#define _VPAddressSetpointCO2Text 0xB005   // 5 ký tự
#define _VPAddressDelayOffText 0x800A      // Length text 15
#define _KeyValueEnterSetRTC 0x11
#define _KeyValueEnterSetAlarm 0x12
#define _KeyValueSetAlarmBelow 0x13
#define _KeyValueSetAlarmAbove 0x14
#define _KeyValueRunButton 0x15
#define _KeyValueResetCalibTemp 0x16

QueueHandle_t xTouchQueue;
// DWIN _dwin(Serial2, HMI_RX_PIN, HMI_TX_PIN);
DWIN _dwin(Serial2, 21, 19);


void cout(uint16_t VP) {
  String text = _dwin.getText(VP, 5);
  int c = text.toInt() + 1;
  if (c > 999) c = 0;
  _dwin.setText(VP, (String)c);
}

void foo(void* a) {
  int VP = (int)a;
  while (1) {
    cout(VP);
    delay(1);
  }
}

TaskHandle_t taskHMIHandle;
void taskHMI(void*) {
  uint32_t notifyNum;
  TouchFrame_t touchRev;

  pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(SERVO_PIN, 0);
  while (1) {
    xQueueReceive(xTouchQueue, &touchRev, portMAX_DELAY);
    Serial.printf("%s nhận %x %x\n", __func__, touchRev.u16VPaddress, touchRev.u16KeyValue);
    switch (touchRev.u16VPaddress) {
    case _VPAddressCacNutNhan:
      switch (touchRev.u16KeyValue) {
      case _KeyValueRunButton:
        digitalWrite(SERVO_PIN, !digitalRead(SERVO_PIN));
        cout(_VPAddressDelayOffText);
        break;
      case _KeyValueSetSetpointTemp:
        digitalWrite(SERVO_PIN, 0);
        break;
      }
      break;
    default:
      break;
    }
  }
}

void setupDWIN() {
  _dwin.begin();
  _dwin.setupTouchCallBack(&xTouchQueue, 5);
  if (xTouchQueue == NULL) {
    Serial.println("CÚT");
    esp_restart();
  }
  // _dwin.echoEnabled(true);
  _dwin.crcEnabled(true);
}

void setup() {
  Serial.begin(115200);

  setupDWIN();
  _dwin.setPage(70);
  _dwin.setText(_VPAddressTemperatureText, "0");
  _dwin.setText(_VPAddressCO2Text, "500");
  _dwin.setText(_VPAddressSetpointTempText, "0");
  _dwin.setText(_VPAddressSetpointCO2Text, "500");
  _dwin.setText(_VPAddressFanSpeedText, "0");
  _dwin.setText(_VPAddressDelayOffText, "0");

  xTaskCreatePinnedToCore(taskHMI, "taskHMI", 8096, NULL, configMAX_PRIORITIES - 2, &taskHMIHandle, 0);
  xTaskCreatePinnedToCore(foo, "_VPAddressTemperatureText", 8096, (void*)_VPAddressTemperatureText, 2, NULL, 0);
  // Serial.printf("heap %lu\n", esp_get_minimum_free_heap_size());
  // xTaskCreatePinnedToCore(foo, "_VPAddressCO2Text", 8096, (void*)_VPAddressCO2Text, 2, NULL, 0);
  // Serial.printf("heap %lu\n", esp_get_minimum_free_heap_size());
  // xTaskCreatePinnedToCore(foo, "_VPAddressSetpointTempText", 8096, (void*)_VPAddressSetpointTempText, 2, NULL, 0);
  // Serial.printf("heap %lu\n", esp_get_minimum_free_heap_size());
  // xTaskCreatePinnedToCore(foo, "_VPAddressSetpointCO2Text", 8096, (void*)_VPAddressSetpointCO2Text, 2, NULL, 0);
  // Serial.printf("heap %lu\n", esp_get_minimum_free_heap_size());
  // xTaskCreatePinnedToCore(foo, "_VPAddressFanSpeedText", 8096, (void*)_VPAddressFanSpeedText, 2, NULL, 0);
  // Serial.printf("heap %lu\n", esp_get_minimum_free_heap_size());
}

void loop() {
  cout(_VPAddressTemperatureText);
  delay(1);
}
