#include <AnhLABV01HardWare.h>
#include <FifoBuffer.h>
#include <vector>
#include <esp_task_wdt.h>
#include "DWIN.h"

#define _NOT(condition) !(condition)

#define _VPAddressSetpointTempText 0x8000 // Length text 5
#define _dwinSerial Serial2

#define _crc true
#define CMD_HEAD1 0x5A
#define CMD_HEAD2 0xA5
#define CMD_WRITE 0x82
#define CMD_READ 0x83
#define CMD_TOUCH 0x83
#define MIN_ASCII 32
#define MAX_ASCII 255

#define FRAME_HEADER_POSTION1 0
#define FRAME_HEADER_POSTION2 1
#define LENGTH_SEND_CMD_POSITION 2
#define CMD_POSITION 3
#define HIGH_VP_POSITION 4
#define LOW_VP_POSITION 5
#define HIGH_KEY_POSITION 7
#define LOW_KEY_POSITION 8
#define MAX_RESPONE_LENGTH 260
#define _GET_VP_ADDRESS(arr, sizeArray) ((sizeArray <= LOW_VP_POSITION) ? 0 : ((((uint16_t)(arr)[HIGH_VP_POSITION]) << 8) | (arr)[LOW_VP_POSITION]))

#define CMD_READ_TIMEOUT 100
#define READ_TIMEOUT 100
#define READ_TIMEOUT_UPDATE 10000
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
#define _VPAddressTemperatureText 0x8019  // Length text 5
#define _VPAddressCO2Text 0xAFFF          // 5 ky tu
#define _VPAddressSetpointTempText 0x8000 // Length text 5
#define _VPAddressFanSpeedText 0x8005     // Length text 5
#define _VPAddressSetpointCO2Text 0xB005  // 5 ký tự
#define _VPAddressDelayOffText 0x800A     // Length text 15

// #define _KeyValueSetFlap 0x0F
// #define _KeyValueEnterFlap 0x10
#define _KeyValueEnterSetRTC 0x11
#define _KeyValueEnterSetAlarm 0x12
#define _KeyValueSetAlarmBelow 0x13
#define _KeyValueSetAlarmAbove 0x14

#define _KeyValueRunButton 0x15
#define _KeyValueResetCalibTemp 0x16

#define _VPAddressGraphY_R_ValueText1 0xB069
#define _VPAddressGraphY_R_ValueText2 0xB06E
#define _VPAddressGraphY_R_ValueText3 0xB073
#define _VPAddressGraphY_R_ValueText4 0xB078
#define _VPAddressGraphY_R_ValueText5 0xB07D
#define _VPAddressGraphY_R_ValueText6 0xB082
#define _VPAddressGraphYValueText1 0x8300
#define _VPAddressGraphYValueText2 0x8305
#define _VPAddressGraphYValueText3 0x830A
#define _VPAddressGraphYValueText4 0x830F
#define _VPAddressGraphYValueText5 0x8314
#define _VPAddressGraphYValueText6 0x8319

DWIN _dwin(Serial2, HMI_RX_PIN, HMI_TX_PIN);

TaskHandle_t taskHMIHandle;
QueueHandle_t QueueTouch;


TickType_t xLastWakeTime;
void setup()
{
  // put your setup code here, to run once:
  esp_task_wdt_deinit();
  Serial.begin(115200);
  _dwin.begin();
  _dwin.crcEnabled(true);
  _dwin.setupTouchCallBack(&QueueTouch, 5);

  // sendArray((uint8_t *)(chuyenTrang + 3), sizeof(chuyenTrang) - 5);
  _dwin.setPage(70);
  Serial.println(_dwin.getPage());
  _dwin.setText(_VPAddressTemperatureText, "0");
  _dwin.setText(_VPAddressCO2Text, "500");
  _dwin.setText(_VPAddressSetpointTempText, "0");
  _dwin.setText(_VPAddressSetpointCO2Text, "500");
  _dwin.setText(_VPAddressGraphYValueText1, "0");
  _dwin.setText(_VPAddressGraphYValueText2, "500");
  _dwin.setText(_VPAddressGraphYValueText3, "0");
  _dwin.setText(_VPAddressGraphYValueText4, "500");
  _dwin.setText(_VPAddressGraphYValueText5, "0");
  _dwin.setText(_VPAddressGraphYValueText6, "500");
  _dwin.setText(_VPAddressGraphY_R_ValueText1, "0");
  _dwin.setText(_VPAddressGraphY_R_ValueText2, "500");
  _dwin.setText(_VPAddressGraphY_R_ValueText3, "0");
  _dwin.setText(_VPAddressGraphY_R_ValueText4, "500");
  _dwin.setText(_VPAddressGraphY_R_ValueText5, "0");
  _dwin.setText(_VPAddressGraphY_R_ValueText6, "500");

  pinMode(0, INPUT);
  while (digitalRead(0) == 1)
  {
    delay(1);
  }
  xTaskCreatePinnedToCore(taskHMI, "taskHMI", 8096, NULL, configMAX_PRIORITIES - 2, &taskHMIHandle, 0);

  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressTemperatureText, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressCO2Text, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressSetpointTempText, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressSetpointCO2Text, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphYValueText1, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphYValueText2, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphYValueText3, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphYValueText4, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphYValueText5, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphYValueText6, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText1, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText2, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText3, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText4, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText5, 2, NULL, 0);
  xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText6, 2, NULL, 0);
  xLastWakeTime = xTaskGetTickCount();
}
void loop()
{
  Serial.printf("--------------------------------------\n");
  vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
}
void cout(uint16_t VP)
{
  String text = _dwin.getText(VP, 5);
  int c = text.toInt() + 1;
  if (c > 999)
    c = 0;
  _dwin.setText(VP, (String)c);
}
void foo(void *a)
{
  int VP = (int)a;
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    cout(VP);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
void taskHMI(void *)
{
  uint32_t notifyNum;
  TouchFrame_t touchRev;
  _dwin.setText(_VPAddressDelayOffText, "0");

  pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(SERVO_PIN, 0);
  while (1)
  {
    xQueueReceive(QueueTouch, &touchRev, portMAX_DELAY);
    Serial.printf("%s nhận %x %x\n", __func__, touchRev.u16VPaddress, touchRev.u16KeyValue);
    switch (touchRev.u16VPaddress)
    {
    case _VPAddressCacNutNhan:
      switch (touchRev.u16KeyValue)
      {
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

uint16_t calculateCRC(uint8_t *data, size_t length)
{
  uint16_t crc = 0xFFFF; // Giá trị khởi tạo CRC

  for (size_t i = 0; i < length; ++i)
  {
    crc ^= data[i]; // XOR dữ liệu với CRC hiện tại

    for (uint8_t j = 0; j < 8; ++j)
    { // Xử lý từng bit
      if (crc & 0x01)
      {
        crc >>= 1;     // Dịch phải
        crc ^= 0xA001; // XOR với hằng số
      }
      else
      {
        crc >>= 1; // Chỉ dịch phải
      }
    }
  }

  return crc; // Trả về CRC tính toán
}
void clearSerial()
{
  while (_dwinSerial.available())
  {
    _dwinSerial.read();
  }
}
