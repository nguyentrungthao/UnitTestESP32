/**
 * @file DWIN.h
 * @author Trung Thao (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-04-21
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef DWIN_H
#define DWIN_H

#include <ctype.h>
#include <stdint.h>

#include <Arduino.h>
#include <HardwareSerial.h>
#include <FS.h>
#include <list>

#define DWIN_DEFAULT_BAUD_RATE 115200
#define ARDUINO_RX_PIN 10
#define ARDUINO_TX_PIN 11

struct TouchFrame_t
{
  uint16_t u16VPaddress;
  uint16_t u16KeyValue;
};
struct PendingRequest_t
{
  uint8_t u8cmd;
  uint16_t u16VPaddress;
  QueueHandle_t xQueueGetResponeData;
};

class DWIN
{
public:
  typedef void (*hmiListener)(String address, uint16_t lastBytes, String message, String response);

  DWIN(HardwareSerial &port, uint8_t receivePin = ARDUINO_RX_PIN, uint8_t transmitPin = ARDUINO_TX_PIN, long baud = DWIN_DEFAULT_BAUD_RATE, uint16_t sizeLeaseQueue = 10);

  void begin(uint32_t u32StackDepthReceive = 8096, BaseType_t xCoreID = tskNO_AFFINITY);
  void setupTouchCallBack(QueueHandle_t *pxTouchQueue, uint16_t sizeOfQueue);
  void setupTouchCallBack(hmiListener callBackTouch);

  void echoEnabled(bool enabled);
  void crcEnabled(bool enabled);

  esp_err_t sendArray(uint8_t *dwin_payload_data, uint8_t payload_data_size, uint8_t *response_buffer = NULL, uint16_t response_buffer_size_max = 0);

  void setVPWord(long address, int data);
  void readVPWord(long address, uint8_t numWords);

  // Get Version
  double getHWVersion();
  // get GUI software version
  double getGUISoftVersion();
  // restart HMI
  void restartHMI();

  // set Particular Page
  esp_err_t setPage(uint8_t pageID);
  // get Current Page ID
  uint8_t getPage();

  // set LCD Brightness
  void setBrightness(uint8_t pConstrast);
  // set LCD Brightness
  uint8_t getBrightness();

  // Play a sound
  void playSound(uint8_t soundID);
  // beep Buzzer
  void beepHMI(int time);

  // set the hardware RTC The first two digits of the year are automatically added
  void setRTC(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
  // upHMI_DATE the software RTC The first two digits of the year are automatically added
  void setRTCSOFT(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday, uint8_t hour, uint8_t minute, uint8_t second);

  // set Data on VP Address
  esp_err_t setText(long address, String textData);
  // get Data on VP Address
  String getText(uint16_t vpAddress, uint8_t lenText);

  void setTextColor(long spAddress, long spOffset, long color);

  void setInt16Value(uint16_t vpAddress, int16_t value);
  void setInt32Value(uint16_t vpAddress, int32_t value);
  void setUint16Value(uint16_t vpAddress, uint16_t value);
  void setUint32Value(uint16_t vpAddress, uint32_t value);
  void setFloatValue(long vpAddress, float fValue);
  int16_t getInt16Value(uint16_t vpAddress);
  int32_t getInt32Value(uint16_t vpAddress);
  uint16_t getUint16Value(uint16_t vpAddress);
  uint32_t getUint32Value(uint16_t vpAddress);
  float getFloatValue(uint16_t vpAddress);

  void setGraphYCentral(uint16_t spAddr, int value);
  void setGraphVDCentral(uint16_t spAddr, int value);
  void setGraphMulY(uint16_t spAddr, int value);
  void setGraphColor(uint16_t spAddr, int value);
  void setGraphRightToLeft(uint16_t spAddr);
  void setGraphLeftToRight(uint16_t spAddr);
  void sendGraphValue(int channel, int value);
  void sendGraphValue(int channel, const int *values, size_t valueCount);
  void resetGraph(int channel);

  // Chỉ trả về False khi xảy ra lỗi trong quá trình cập nhật
  // Các trường hợp không tìm thấy thư mục hoặc file cập nhật
  // Tức là không có bản cập nhật mới sẽ trả về true
  bool updateHMI(fs::FS &filesystem, const char *dirPath);

private:
  HardwareSerial *_dwinSerial;
  uint8_t _rxPin, _txPin;
  long _baudrate;
  bool _isSoft;       // Is serial interface software
  long _baud;         // DWIN HMI Baud rate
  bool _echo; // Response Command Show
  bool _crc;
  hmiListener listenerCallback;
  QueueHandle_t xQueueTouch;
  QueueHandle_t xQueueCommandReadWrite;
  QueueHandle_t xQueueResponeDataEvent;
  SemaphoreHandle_t xMutexPendingRequest;

  static TaskHandle_t xTaskDWINHandle;

  bool isFirmwareFile(String &fileName);
  void flushSerial();
  void clearSerial();
  uint16_t u16CalculateCRCModbus(uint8_t *data, size_t length);

  static void xTaskReceiveUartEvent(void *ptr);
  void xHandle();
  static void vTriggerTaskReceiveFromUartEvent();
};
#endif // DWIN_H
