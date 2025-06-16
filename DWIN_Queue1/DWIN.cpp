#include "DWIN.h"

#define CMD_HEAD1 0x5A
#define CMD_HEAD2 0xA5
#define CMD_WRITE 0x82
#define CMD_READ 0x83
#define CMD_TOUCH 0x83
#define MAX_ASCII 0xFF


#define FRAME_HEADER_POSTION1 0
#define FRAME_HEADER_POSTION2 1
#define LENGTH_SEND_CMD_POSITION 2
#define CMD_POSITION 3
#define HIGH_VP_POSITION 4
#define LOW_VP_POSITION 5
#define HIGH_KEY_POSITION 7
#define LOW_KEY_POSITION 8
#define MAX_RESPONE_LENGTH 260
#define __GET_VP_ADDRESS(arr, sizeArray) ((sizeArray <= LOW_VP_POSITION) ? 0 : ((((uint16_t)(arr)[HIGH_VP_POSITION]) << 8) | (arr)[LOW_VP_POSITION]))
#define __NOT(condition) !(condition)

#define TIME_OUT_RECEIVE 1000

TaskHandle_t DWIN::xTaskDWINHandle;

LeaseQueue::LeaseQueue(uint16_t sizeOfArrQueueManager) {
  u16SizeOfArrQueueManager = sizeOfArrQueueManager;
  arrQueueManager = (LeaseQueuePaket_t*)malloc(u16SizeOfArrQueueManager * sizeof(LeaseQueuePaket_t));
  for (uint16_t i = 0; i < sizeOfArrQueueManager; i++) {
    arrQueueManager[i].blOccupied = false;
    arrQueueManager[i].xQueueGet = xQueueCreate(1, sizeof(WaitingResponeFrame_t));
    if (arrQueueManager[i].xQueueGet == NULL) {
      ESP_LOGE(_FILE__, "\t\tERROR: LeaseQueue không thể tạo hàng đợi => restart\n");
      esp_restart();
    }
  }
}

LeaseQueue::~LeaseQueue() {
  for (uint16_t i = 0; i < u16SizeOfArrQueueManager; i++) {
    if (arrQueueManager[i].xQueueGet != NULL) {
      vQueueDelete(arrQueueManager[i].xQueueGet);
    }
  }
  if (arrQueueManager)
    free(arrQueueManager);
}

int8_t LeaseQueue::getIndexQueue() {
  for (int8_t i = 0; i < u16SizeOfArrQueueManager; i++) {
    if (arrQueueManager[i].blOccupied == false && arrQueueManager[i].xQueueGet != NULL) {
      arrQueueManager[i].blOccupied = true;
      return i;
    }
  }
  Serial.printf("\tWARNING: nhiều lệnh read/write được gọi cùng lúc => tăng  kích thước arrQueueManager\n");
  return -1;
}
QueueHandle_t LeaseQueue::rentQueueHandle(int8_t index) {
  if (index < 0 || index >= u16SizeOfArrQueueManager) {
    return NULL;
  }
  return arrQueueManager[index].xQueueGet;
}
void LeaseQueue::refundQueue(int8_t index) {
  if (index < 0 || index >= u16SizeOfArrQueueManager) {
    return;
  }
  arrQueueManager[index].blOccupied = false;
}

DWIN::DWIN(HardwareSerial& port, uint8_t receivePin, uint8_t transmitPin, long baud, uint16_t sizeLeaseQueue)
  : LeaseQueue(sizeLeaseQueue),
  _dwinSerial((HardwareSerial*)&port),
  listenerCallback(NULL),
  _baudrate(baud),
  _rxPin(receivePin),
  _txPin(transmitPin) {
}

void DWIN::begin(uint32_t u32StackDepthReceive, BaseType_t xCoreID) {
  _dwinSerial->begin(_baudrate, SERIAL_8N1, _rxPin, _txPin);
  delay(10);
  xReadWriteQueue = xQueueCreate(1, sizeof(WaitingResponeFrame_t));
  delay(10);
  xResponeQueueHandle = xQueueCreate(1, sizeof(WaitingResponeFrame_t));
  xTaskCreatePinnedToCore(xTaskReceiveUartEvent, "RecvUartEvent", u32StackDepthReceive, (void*)this, configMAX_PRIORITIES - 1, &xTaskDWINHandle, xCoreID);
  delay(10);
  _dwinSerial->onReceive(vTriggerTaskReceiveFromUartEvent, true);
}

void DWIN::setupTouchCallBack(QueueHandle_t* pxTouchQueue, uint16_t sizeOfQueue) {
  (*pxTouchQueue) = xQueueCreate(sizeOfQueue, sizeof(TouchFrame_t));
  xTouchQueue = (*pxTouchQueue);
  if (xTouchQueue == NULL) {
    ESP_LOGE(__FILE__, "%s can't create Queue => esp restart", __func__);
    esp_restart();
  }
}
void DWIN::setupTouchCallBack(hmiListener callBackTouch) {
  xTouchQueue = xQueueCreate(5, sizeof(TouchFrame_t));
  if (__NOT(callBackTouch)) {
    ESP_LOGE(__FILE__, "%s INVALID PRAMETER", __func__);
    return;
  }
  listenerCallback = callBackTouch;
}

void DWIN::echoEnabled(bool enabled) {
  _echo = enabled;
}
void DWIN::crcEnabled(bool enabled) {
  _crc = enabled;
}


/**
 *@brief send the dwinSendArray and get the respone
 *
 * @param dwinSendArray data send to DWIN without Frame header and Length. example 5AA5 06 (83 00 0F 01 14 10) <---
 * @param arraySize size of dwinSendArray
 * @param pu8OutData buffer get data respone from DWIN display. input NULL if you dont need the respone
 * @param u16OutDataSize size of pu8OutData. it should be greater than or equal to expect respone data from DWIN display
 *                       Maxvalue is MAX_RESPONE_LENGTH
 * @return esp_err_t ESP_OK: send dwinSendArray and get respone in timeout 100ms
 */
esp_err_t DWIN::sendArray(uint8_t* dwinSendArray, uint8_t arraySize, uint8_t* pu8OutData, uint16_t u16OutDataSize) {
  if (dwinSendArray == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  //chuẩn bị dữ liệu
  uint8_t dataLen = arraySize + 3;
  if (_crc) {
    dataLen += 2;
  }
  uint8_t sendBuffer[dataLen] = { CMD_HEAD1, CMD_HEAD2, dataLen - 3 };
  const uint16_t u16SizeOfSendBuffer = sizeof(sendBuffer) / sizeof(sendBuffer[0]);
  memcpy(sendBuffer + 3, dwinSendArray, arraySize);
  if (_crc) {
    uint16_t crc = u16CalculateCRCModbus(sendBuffer + 3, u16SizeOfSendBuffer - 5);
    sendBuffer[u16SizeOfSendBuffer - 2] = crc & 0xFF;
    sendBuffer[u16SizeOfSendBuffer - 1] = (crc >> 8) & 0xFF;
  }

  if (_echo) {
    Serial.printf("Send array: ");
    for (uint8_t i = 0; i < dataLen; i++) {
      Serial.printf("%x ", sendBuffer[i]);
    }
    Serial.println();
  }

  // đưa vào hàng đợt trước khi gửi lệnh
  uint8_t pu8Data[MAX_RESPONE_LENGTH] = {};
  WaitingResponeFrame_t xWaitingElement = {
    .pu8SendCMD = sendBuffer,
    .u8SendCMDSize = dataLen,
    .pu8DataReceive = pu8Data,
    .u8DataReceiveSize = 0,
    .xQueueGet = xResponeQueueHandle
  };
  if (xQueueSend(xReadWriteQueue, &xWaitingElement, pdMS_TO_TICKS(1000)) != pdPASS) {
    Serial.printf("xQueueSend fail\n");
    return ESP_ERR_TIMEOUT;
  }

  // gửi lệnh
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));

  // chờ data
  bool ret = xQueueReceive(xResponeQueueHandle, &xWaitingElement, pdMS_TO_TICKS(TIME_OUT_RECEIVE));
  if (ret == pdPASS) {
    if (pu8OutData && u16OutDataSize >= xWaitingElement.u8DataReceiveSize) {
      memcpy(pu8OutData, xWaitingElement.pu8DataReceive, xWaitingElement.u8DataReceiveSize);
    }
  }
  else {
    Serial.printf("time out\n");
  }

  return ret ? ESP_OK : ESP_ERR_TIMEOUT;
}


esp_err_t DWIN::setPage(uint8_t pageID) {
  uint8_t sendBuffer[] = { CMD_WRITE, 0x00, 0x84, 0x5A, 0x01, 0x00, pageID };

  return sendArray(sendBuffer, sizeof(sendBuffer) / sizeof(sendBuffer[0]));
}


// set Data on VP Address
esp_err_t DWIN::setText(long address, String textData) {
  uint16_t sendBufferSize = textData.length() + 5;

  uint8_t sendBuffer[sendBufferSize] = { CMD_WRITE, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address) & 0xFF) };
  sendBuffer[sendBufferSize - 1] = MAX_ASCII;
  sendBuffer[sendBufferSize - 2] = MAX_ASCII;

  memcpy(sendBuffer + 3, textData.c_str(), textData.length());

  return sendArray(sendBuffer, sizeof(sendBuffer) / sizeof(sendBuffer[0]));
}
// get Data on VP Address
String DWIN::getText(uint16_t vpAddress, uint8_t lenText) {
  String responeText = "";
  uint8_t numWords = lenText / 2 + 1;
  uint8_t sendBuffer[] = { CMD_READ, (uint8_t)((vpAddress >> 8) & 0xFF), (uint8_t)((vpAddress) & 0xFF), numWords };
  uint8_t pu8Data[MAX_RESPONE_LENGTH] = {};

  esp_err_t ret = sendArray(sendBuffer, sizeof(sendBuffer) / sizeof(sendBuffer[0]), pu8Data, MAX_RESPONE_LENGTH);
  if (ret != ESP_OK) {
    return responeText;
  }

  //5a a5 10 83 80 0 5 41 42 ff ff 0 0 0 0 0 0 fc f6
  uint16_t i = 7;
  do {
    if (isascii(pu8Data[i])) {
      responeText += (char)pu8Data[i];
    }
    i++;
  } while (__NOT(pu8Data[i] == MAX_ASCII && pu8Data[i + 1] == MAX_ASCII));

  return responeText;
}


// private menthod

bool DWIN::isFirmwareFile(String& fileName) {
  const char* const pcCONST_END_FILE[] = {
    ".bin", ".BIN", ".icl", ".ICL", ".dzk", ".DZK",
    ".hzk", ".HZK", ".lib", ".LIB", ".wae", ".WAE",
    ".uic", ".UIC", ".gif", ".GIF"
  };
  for (const auto& ext : pcCONST_END_FILE) {
    if (fileName.endsWith(ext)) {
      return true;
    }
  }
  return false;
}

/**
 *@brief wait until TX have done
 *
 */
void DWIN::flushSerial() {
  _dwinSerial->flush();
}
void DWIN::clearSerial() {
  while (_dwinSerial->available()) {
    _dwinSerial->read();
  }
}
uint16_t DWIN::u16CalculateCRCModbus(uint8_t* data, size_t length) {
  uint16_t crc = 0xFFFF;

  for (size_t i = 0; i < length; ++i) {
    crc ^= data[i];

    for (uint8_t j = 0; j < 8; ++j) {
      if (crc & 0x01) {
        crc >>= 1;
        crc ^= 0xA001;
      }
      else {
        crc >>= 1;
      }
    }
  }

  return crc;
}

void DWIN::xTaskReceiveUartEvent(void* ptr) {
  DWIN* pxDWIN = (DWIN*)ptr;
  uint32_t notifyNum;
  while (1) {
    xTaskNotifyWait(pdFALSE, pdTRUE, &notifyNum, portMAX_DELAY);
    pxDWIN->xHandle();
  }
}
void DWIN::xHandle() {
  static uint8_t buffer[MAX_RESPONE_LENGTH] = {};
  constexpr uint16_t SIZE_OF_BUFFER = sizeof(buffer) / sizeof(buffer[0]);
  while (_dwinSerial->available()) {
    if (_dwinSerial->read() != 0x5A) {
      continue;
    }
    if (_dwinSerial->read() != 0xA5) {
      continue;
    }
    buffer[FRAME_HEADER_POSTION1] = 0x5A;
    buffer[FRAME_HEADER_POSTION2] = 0xA5;
    const uint16_t length = _dwinSerial->read();
    buffer[LENGTH_SEND_CMD_POSITION] = length;
    _dwinSerial->readBytes(buffer + CMD_POSITION, length);

    if (_crc) {
      uint16_t receivedCRC;
      memcpy((uint8_t*)&receivedCRC, &buffer[length + 1], 2);
      uint16_t calculatedCRC = u16CalculateCRCModbus(buffer + CMD_POSITION, length - 2);
      if (receivedCRC != calculatedCRC) {
        Serial.println("Serial check -> Fail");
        continue;
      }
    }
    if (_echo) {
      Serial.printf("DWIN receive: ");
      for (uint8_t i = 0; i < length + 3; i++) {
        Serial.printf("%x ", buffer[i]);
      }
      Serial.println();
    }

    const uint8_t cmd = buffer[CMD_POSITION];
    const uint16_t VPaddressFromUart = __GET_VP_ADDRESS(buffer, SIZE_OF_BUFFER);

    WaitingResponeFrame_t xWaitingResponeElement;
    if (xQueuePeek(xReadWriteQueue, &xWaitingResponeElement, 0)) {

      if (xWaitingResponeElement.pu8DataReceive) {
        memcpy(xWaitingResponeElement.pu8DataReceive, buffer, length + 3);
        xWaitingResponeElement.u8DataReceiveSize = length + 3;
      }
      if (xWaitingResponeElement.xQueueGet) {
        xQueueSend(xWaitingResponeElement.xQueueGet, &xWaitingResponeElement, 0);
      }
    }
    else {
      Serial.printf("TOUCH %lu \n", uxQueueMessagesWaiting(xReadWriteQueue));
      TouchFrame_t xTouchDataSend = {
        .u16VPaddress = VPaddressFromUart,
        .u16KeyValue = (((uint16_t)buffer[7]) << 8) | buffer[8]
      };
      if (xTouchQueue) {
        xQueueSend(xTouchQueue, &xTouchDataSend, 0);
      }
    }
  }
}
void DWIN::vTriggerTaskReceiveFromUartEvent() {
  xTaskNotify(xTaskDWINHandle, 0x01, eSetBits);
}