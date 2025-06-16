#include <AnhLABV01HardWare.h>
#include <FifoBuffer.h>
#include <vector>
#include <esp_task_wdt.h>

#define _NOT(condition) !(condition)

#define _VPAddressSetpointTempText 0x8000  // Length text 5
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
#define _VPAddressTemperatureText 0x8019   // Length text 5
#define _VPAddressCO2Text 0xAFFF           // 5 ky tu
#define _VPAddressSetpointTempText 0x8000  // Length text 5
#define _VPAddressFanSpeedText 0x8005      // Length text 5
#define _VPAddressSetpointCO2Text 0xB005   // 5 ký tự
#define _VPAddressDelayOffText 0x800A      // Length text 15

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


typedef struct {
  uint8_t cmd;
  uint16_t u16VPaddress;
  QueueHandle_t response_queue;
  bool active;
} PendingRequest_t;
PendingRequest_t pxPendingRequest[20] = {};
constexpr uint16_t SIZE_OF_PENDING_REQUEST = sizeof(pxPendingRequest) / sizeof(PendingRequest_t);
SemaphoreHandle_t xMutexPendingRequest = NULL;

struct TouchFrame_t {
  uint16_t u16VPaddress;
  uint16_t u16KeyValue;
};

TaskHandle_t taskHMIHandle;
TaskHandle_t taskDWINHandle;
QueueHandle_t QueueTouch;

const uint8_t chuyenTrang[] = { 0x5A, 0xA5, 0x09, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x46, 0x8B, 0xFC };
const uint8_t getpage[] = { 0x5A, 0xA5, 0x06, 0x83, 0x00, 0x0F, 0x01, 0xED, 0x90 };
const uint8_t arrGetText[][9] = { { 0x5A, 0xA5, 0x06, 0x83, 0x80, 0x00, 0x05, 0xE8, 0x4B }, { 0x5A, 0xA5, 0x06, 0x83, 0x80, 0x05, 0x05, 0xE8, 0x4B } };
// const uint8_t setText[] = { 0x5A, 0xA5, 0x08, 0x82, 0x80, 0x00, 0x41, 0x42, 0xFF, 0xFF, 0x13, 0x50 };
const uint8_t getGUIVersion[] = { 0x5A, 0xA5, 0x04, 0x83, 0x00, 0x0F, 0x01, 0xFF, 0xFF };


bool sendArray(uint8_t *dwinSendArray, uint8_t arraySize, uint8_t *pu8OutData = NULL, uint16_t u16OutDataSize = 0, uint16_t u16TimeOutInSecond = 100);

bool sendArray(uint8_t *dwinSendArray, uint8_t arraySize, uint8_t *pu8OutData, uint16_t u16OutDataSize, uint16_t u16TimeOutInSecond) {
  if (dwinSendArray == NULL || arraySize < 3) {
    ESP_LOGE(__func__, "loi tham so");
    return 0;
  }

  //*chuẩn bị dữ liệu
  uint8_t dataLen = arraySize + 3;
  if (_crc) {
    dataLen += 2;
  }
  uint8_t sendBuffer[dataLen] = { CMD_HEAD1, CMD_HEAD2, dataLen - 3 };
  uint16_t u16SizeOfSendBuffer = sizeof(sendBuffer) / sizeof(sendBuffer[0]);
  memcpy(sendBuffer + 3, dwinSendArray, arraySize);
  if (_crc) {
    uint16_t crc = calculateCRC(sendBuffer + 3, u16SizeOfSendBuffer - 5);
    sendBuffer[u16SizeOfSendBuffer - 2] = crc & 0xFF;
    sendBuffer[u16SizeOfSendBuffer - 1] = (crc >> 8) & 0xFF;
  }

  //* mượn queue
  int16_t u16IndexQueueInPendingRequest = -1;
  uint16_t VPaddress = (((uint16_t)sendBuffer[HIGH_VP_POSITION]) << 8) | sendBuffer[LOW_VP_POSITION];
  if (_NOT(xSemaphoreTake(xMutexPendingRequest, pdMS_TO_TICKS(100)))) {
    ESP_LOGE(__func__, "khong lay duoc semaphore %x", VPaddress);
    return false;
  }
  for (uint16_t i = 0; i < SIZE_OF_PENDING_REQUEST; i++) {
    if (_NOT(pxPendingRequest[i].active)) {
      u16IndexQueueInPendingRequest = i;
      pxPendingRequest[i].active = true;
      pxPendingRequest[i].u16VPaddress = VPaddress;
      pxPendingRequest[i].cmd = sendBuffer[CMD_POSITION];
      xQueueReset(pxPendingRequest[i].response_queue);
      break;
    }
  }
  xSemaphoreGive(xMutexPendingRequest);

  //*gửi lệnh
  if (u16IndexQueueInPendingRequest < 0) {
    ESP_LOGE(__func__, "khong muon duoc queue %x", VPaddress);
    return false;
  }
  // clearSerial();
  _dwinSerial.write(sendBuffer, sizeof(sendBuffer));
  // _dwinSerial.flush();

  //*chờ dữ liệu về
  uint8_t arr[MAX_RESPONE_LENGTH] = {};
  bool ret = xQueueReceive(pxPendingRequest[u16IndexQueueInPendingRequest].response_queue, arr, pdMS_TO_TICKS(1000));
  if (ret == pdPASS) {
    if (pu8OutData && u16OutDataSize >= MAX_RESPONE_LENGTH) {
      memcpy(pu8OutData, arr, MAX_RESPONE_LENGTH);
    } else {
      ESP_LOGE(__func__, "trỏ nhận null hoặc không đủ độ dài %x", VPaddress);
    }
  } else {
    ESP_LOGE(__func__, "khong nhan duoc phan hoi %x, %x", VPaddress, sendBuffer[CMD_POSITION]);
    for (uint16_t i = 0; i <= u16IndexQueueInPendingRequest; i++) {
      Serial.printf("%x %s, ", pxPendingRequest[i].u16VPaddress, pxPendingRequest[i].active ? "T" : "F");
    }
    Serial.println(u16IndexQueueInPendingRequest);
  }

  //* trả queue
  if (xSemaphoreTake(xMutexPendingRequest, portMAX_DELAY)) {
    if (pxPendingRequest[u16IndexQueueInPendingRequest].active && pxPendingRequest[u16IndexQueueInPendingRequest].u16VPaddress == VPaddress) {
      pxPendingRequest[u16IndexQueueInPendingRequest].active = false;
      pxPendingRequest[u16IndexQueueInPendingRequest].u16VPaddress = 0;
      ESP_LOGD(__func__, "reset pending request");
    }
    xSemaphoreGive(xMutexPendingRequest);
  }
  return ret;
}
void taskDWIN(void *) {
  uint32_t notifyNum;
  static uint8_t buffer[MAX_RESPONE_LENGTH] = {};
  constexpr uint16_t SIZE_OF_BUFFER = sizeof(buffer) / sizeof(buffer[0]);
  TouchFrame_t touchSend;

  /*
  Instruction                    Unable CRC                        Check CRC Check
  0x83 Read Instruction          Tx: 5AA5 04 83 000F 01            Tx: 5AA5 06 83 000F 01ED 90
  0x83 Instruction Response      Rx: 5AA5 06 83 00 0F 01 14 10     Rx: 5AA5 08 83 00 0F 01 14 10 43 F0
  0x82 Write Instruction         Tx: 5AA5 05 82 10 00 31 32        Tx: 5AA5 07 82 10 00 31 32 CC 9B
  0x82 Instruction Response      Rx: 5AA5 03 82 4F 4B              Rx: 5AA5 05 82 4F 4B A5 EF
  0x83 Touch Upload              Rx: 5AA5 06 83 10 01 01 00 5A     Rx: 5AA5 08 83 10 01 01 00 5A 0E 2C
  */
  while (1) {
    xTaskNotifyWait(pdFALSE, pdTRUE, &notifyNum, portMAX_DELAY);
    while (_dwinSerial.available()) {

      if (_dwinSerial.read() != 0x5A) {
        continue;
      }
      if (_dwinSerial.read() != 0xA5) {
        continue;
      }
      buffer[FRAME_HEADER_POSTION1] = 0x5A;
      buffer[FRAME_HEADER_POSTION2] = 0xA5;
      uint16_t length = _dwinSerial.read();
      buffer[LENGTH_SEND_CMD_POSITION] = length;
      _dwinSerial.readBytes(buffer + CMD_POSITION, length);

      if (_crc) {
        uint16_t receivedCRC;
        memcpy((uint8_t *)&receivedCRC, &buffer[length + 1], 2);
        uint16_t calculatedCRC = calculateCRC(buffer + CMD_POSITION, length - 2);
        if (receivedCRC != calculatedCRC) {
          ESP_LOGE(__func__, "Serial check -> Fail");
          continue;
        }
      }

      uint8_t cmd = buffer[CMD_POSITION];
      uint16_t VPaddressFromUart = _GET_VP_ADDRESS(buffer, SIZE_OF_BUFFER);
      uint16_t i;
      if (_NOT(xSemaphoreTake(xMutexPendingRequest, pdMS_TO_TICKS(10000)))) {
        ESP_LOGE(__func__, "khong lay duoc semaphore");
        continue;
      }

      for (i = 0; i < SIZE_OF_PENDING_REQUEST; i++) {
        // Serial.printf("%x,%s,%lu ", pxPendingRequest[i].u16VPaddress, pxPendingRequest[i].active ? "T" : "F", i);
        if (_NOT(pxPendingRequest[i].active)) {
          continue;
        }
        if (cmd != pxPendingRequest[i].cmd) {
          continue;
        }

        if (cmd == CMD_WRITE) {
          pxPendingRequest[i].active = false;
          pxPendingRequest[i].u16VPaddress = 0;
          break;
        } else if (cmd == CMD_READ && VPaddressFromUart == pxPendingRequest[i].u16VPaddress) {  // có task mượn và gửi lệnh WRITE
          break;
        }
      }
      // Serial.println();
      xSemaphoreGive(xMutexPendingRequest);

      if (i < SIZE_OF_PENDING_REQUEST) {  // tức là phản hồi của lệnh READ/WRITE
        if (xQueueSend(pxPendingRequest[i].response_queue, buffer, 10) == pdTRUE) {
          // Serial.println("gửi queue oke");
        }
      } else {  // lệnh touch
        touchSend.u16VPaddress = VPaddressFromUart;
        touchSend.u16KeyValue = (((uint16_t)buffer[7]) << 8) | buffer[8];
        xQueueSend(QueueTouch, &touchSend, 0);
      }
    }
    memset(buffer, 0, SIZE_OF_BUFFER);
  }
}

void callBackUart() {
  xTaskNotify(taskDWINHandle, 0x01, eSetBits);
}
TickType_t xLastWakeTime;
void setup() {
  // put your setup code here, to run once:
  esp_task_wdt_deinit();
  Serial.begin(115200);
  vDwinInit();

  sendArray((uint8_t *)(chuyenTrang + 3), sizeof(chuyenTrang) - 5);
  setText(_VPAddressTemperatureText, "0");
  setText(_VPAddressCO2Text, "500");
  setText(_VPAddressSetpointTempText, "0");
  setText(_VPAddressSetpointCO2Text, "500");
  setText(_VPAddressGraphYValueText1, "0");
  setText(_VPAddressGraphYValueText2, "500");
  setText(_VPAddressGraphYValueText3, "0");
  setText(_VPAddressGraphYValueText4, "500");
  setText(_VPAddressGraphYValueText5, "0");
  setText(_VPAddressGraphYValueText6, "500");
  setText(_VPAddressGraphY_R_ValueText1, "0");
  setText(_VPAddressGraphY_R_ValueText2, "500");
  setText(_VPAddressGraphY_R_ValueText3, "0");
  setText(_VPAddressGraphY_R_ValueText4, "500");
  setText(_VPAddressGraphY_R_ValueText5, "0");
  setText(_VPAddressGraphY_R_ValueText6, "500");

  pinMode(0, INPUT);
  while (digitalRead(0) == 1) {
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
  // xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText1, 2, NULL, 0);
  // xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText2, 2, NULL, 0);
  // xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText3, 2, NULL, 0);
  // xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText4, 2, NULL, 0);
  // xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText5, 2, NULL, 0);
  // xTaskCreatePinnedToCore(foo, "", 8096, (void *)_VPAddressGraphY_R_ValueText6, 2, NULL, 0);
  xLastWakeTime = xTaskGetTickCount();
}
void loop() {
  Serial.printf("--------------------------------------\n");
  vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
}
void cout(uint16_t VP) {
  String text = getText(VP, 5);
  int c = text.toInt() + 1;
  if (c > 999) c = 0;
  setText(VP, (String)c);
}
void foo(void *a) {
  int VP = (int)a;
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  while (1) {
    cout(VP);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void vDwinInit() {
  Serial2.begin(115200, SERIAL_8N1, HMI_RX_PIN, HMI_TX_PIN);
  QueueTouch = xQueueCreate(5, sizeof(TouchFrame_t));
  if (_NOT(QueueTouch)) {
    Serial.printf("ERROR: không thể cấp phát đủ QueueTouch\n");
    delay(1000);
    esp_restart();
  }

  xMutexPendingRequest = xSemaphoreCreateMutex();
  if (_NOT(QueueTouch)) {
    Serial.printf("ERROR: không thể cấp phát đủ xMutexPendingRequest\n");
    delay(1000);
    esp_restart();
  }

  for (uint8_t i = 0; i < SIZE_OF_PENDING_REQUEST; i++) {
    pxPendingRequest[i].active = false;
    pxPendingRequest[i].response_queue = xQueueCreate(1, MAX_RESPONE_LENGTH);
    if (pxPendingRequest[i].response_queue == NULL) {
      Serial.printf("ERROR: không thể cấp phát đủ pxPendingRequest. số lượng đã cấp: %d => RESET\n", i);
      delay(1000);
      esp_restart();
    }
  }
  xTaskCreatePinnedToCore(taskDWIN, "taskDWIN", 8096, NULL, configMAX_PRIORITIES - 1, &taskDWINHandle, 0);
  Serial2.onReceive(callBackUart, true);
}
void setText(uint16_t VPaddress, String text) {
  // Serial.printf("%s %x\n", __func__, VPaddress);
  uint16_t sendBufferSize = text.length() + 5;

  uint8_t sendBuffer[sendBufferSize] = { CMD_WRITE, (uint8_t)((VPaddress >> 8) & 0xFF), (uint8_t)((VPaddress)&0xFF) };
  sendBuffer[sendBufferSize - 1] = MAX_ASCII;
  sendBuffer[sendBufferSize - 2] = MAX_ASCII;

  memcpy(sendBuffer + 3, text.c_str(), text.length());

  uint8_t pu8Data[MAX_RESPONE_LENGTH] = {};
  sendArray(sendBuffer, sizeof(sendBuffer) / sizeof(sendBuffer[0]), pu8Data, MAX_RESPONE_LENGTH);
}

String getText(uint16_t VPaddress, uint16_t length) {
  // Serial.printf("%s %x\n", __func__, VPaddress);
  uint8_t numWords = length / 2 + 1;
  uint8_t sendBuffer[] = { CMD_READ, (uint8_t)((VPaddress >> 8) & 0xFF), (uint8_t)((VPaddress)&0xFF), numWords };
  uint8_t pu8Data[MAX_RESPONE_LENGTH] = {};

  sendArray(sendBuffer, sizeof(sendBuffer) / sizeof(sendBuffer[0]), pu8Data, MAX_RESPONE_LENGTH);

  String responeText = "";
  //5a a5 10 83 80 0 5 41 42 ff ff 0 0 0 0 0 0 fc f6
  uint16_t i = 7;
  do {
    if (isascii(pu8Data[i])) {
      responeText += (char)pu8Data[i];
    }
    i++;
  } while (_NOT(pu8Data[i] == MAX_ASCII && pu8Data[i + 1] == MAX_ASCII));

  return responeText;
}
void taskHMI(void *) {
  uint32_t notifyNum;
  TouchFrame_t touchRev;
  setText(_VPAddressDelayOffText, "0");

  pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(SERVO_PIN, 0);
  while (1) {
    xQueueReceive(QueueTouch, &touchRev, portMAX_DELAY);
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


uint16_t calculateCRC(uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFF;  // Giá trị khởi tạo CRC

  for (size_t i = 0; i < length; ++i) {
    crc ^= data[i];  // XOR dữ liệu với CRC hiện tại

    for (uint8_t j = 0; j < 8; ++j) {  // Xử lý từng bit
      if (crc & 0x01) {
        crc >>= 1;      // Dịch phải
        crc ^= 0xA001;  // XOR với hằng số
      } else {
        crc >>= 1;  // Chỉ dịch phải
      }
    }
  }

  return crc;  // Trả về CRC tính toán
}
void clearSerial() {
  while (_dwinSerial.available()) {
    _dwinSerial.read();
  }
}
