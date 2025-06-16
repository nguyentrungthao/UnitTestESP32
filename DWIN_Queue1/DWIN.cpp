#include "DWIN.h"

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
#define _NOT(condition) !(condition)

#define TIME_OUT_RECEIVE 1000

TaskHandle_t DWIN::xTaskDWINHandle;

DWIN::DWIN(HardwareSerial &port, uint8_t receivePin, uint8_t transmitPin, long baud, uint16_t sizeLeaseQueue)
    : _dwinSerial((HardwareSerial *)&port),
      listenerCallback(NULL),
      _baudrate(baud),
      _rxPin(receivePin),
      _txPin(transmitPin)
{
}

void DWIN::begin(uint32_t u32StackDepthReceive, BaseType_t xCoreID)
{
  _dwinSerial->begin(_baudrate, SERIAL_8N1, _rxPin, _txPin);
  delay(10);
  xQueueCommandReadWrite = xQueueCreate(1, sizeof(PendingRequest_t));
  delay(10);
  xQueueResponeDataEvent = xQueueCreate(1, sizeof(MAX_RESPONE_LENGTH));
  delay(10);
  xMutexPendingRequest = xSemaphoreCreateMutex();
  delay(10);
  xTaskCreatePinnedToCore(xTaskReceiveUartEvent, "RecvUartEvent", u32StackDepthReceive, (void *)this, configMAX_PRIORITIES - 1, &xTaskDWINHandle, xCoreID);
  delay(10);
  _dwinSerial->onReceive(vTriggerTaskReceiveFromUartEvent, true);
}

void DWIN::setupTouchCallBack(QueueHandle_t *pxTouchQueue, uint16_t sizeOfQueue)
{
  (*pxTouchQueue) = xQueueCreate(sizeOfQueue, sizeof(TouchFrame_t));
  xQueueTouch = (*pxTouchQueue);
  if (xQueueTouch == NULL)
  {
    ESP_LOGE(__FILE__, "%s can't create Queue => esp restart", __func__);
    esp_restart();
  }
}
void DWIN::setupTouchCallBack(hmiListener callBackTouch)
{
  xQueueTouch = xQueueCreate(5, sizeof(TouchFrame_t));
  if (_NOT(callBackTouch))
  {
    ESP_LOGE(__FILE__, "%s INVALID PRAMETER", __func__);
    return;
  }
  listenerCallback = callBackTouch;
}

void DWIN::echoEnabled(bool enabled)
{
  _echo = enabled;
}
void DWIN::crcEnabled(bool enabled)
{
  _crc = enabled;
}

/**
 *@brief send the dwinSendArray and get the respone
 *
 * @param dwinSendArray data send to DWIN without Frame header and Length. example 5AA5 06 (83 00 0F 01 14 10) <---
 * @param arraySize size of dwinSendArray
 * @param pu8OutData pu8BufferReceiver get data respone from DWIN display. input NULL if you dont need the respone
 * @param u16OutDataSize size of pu8OutData. it should be greater than or equal to expect respone data from DWIN display
 *                       Maxvalue is MAX_RESPONE_LENGTH
 * @return esp_err_t ESP_OK: send dwinSendArray and get respone in timeout 100ms
 */
esp_err_t DWIN::sendArray(uint8_t *dwin_payload_data, uint8_t payload_data_size, uint8_t *response_buffer, uint16_t response_buffer_size_max)
{
  // Validate input parameters
  if (dwin_payload_data == NULL || payload_data_size < 3)
  {
    ESP_LOGE(__func__, "Invalid parameters: dwin_payload_data is NULL or payload_data_size is too small.");
    return ESP_ERR_INVALID_ARG; 
  }

  uint8_t total_frame_length = payload_data_size + 3;
  if (_crc)
  {                          
    total_frame_length += 2; 
  }

  uint8_t send_frame_buffer[total_frame_length];

  send_frame_buffer[0] = CMD_HEAD1; 
  send_frame_buffer[1] = CMD_HEAD2; // Assuming CMD_HEAD2 is DWIN_CMD_HEADER_BYTE2
  send_frame_buffer[2] = payload_data_size;     // The actual length of the data following the length byte

  // Copy the DWIN payload data into the send pu8BufferReceiver
  memcpy(send_frame_buffer + 3, dwin_payload_data, payload_data_size);

  // Calculate and append CRC if enabled
  if (_crc)
  {
    // CRC is typically calculated over the command and data bytes
    // Adjust the size based on your DWIN CRC specification
    uint16_t crc_value = u16CalculateCRCModbus(send_frame_buffer + 3, payload_data_size); // Assuming calculateCRC takes (data_ptr, len)
    send_frame_buffer[total_frame_length - 2] = (uint8_t)(crc_value & 0xFF);
    send_frame_buffer[total_frame_length - 1] = (uint8_t)((crc_value >> 8) & 0xFF);
  }

  // Prepare xPendingRequest request for the FreeRTOS queue
  uint16_t VPaddress = (((uint16_t)send_frame_buffer[HIGH_VP_POSITION]) << 8) | send_frame_buffer[LOW_VP_POSITION]; // Using defined constants

  PendingRequest_t xrequestInfo = {
      .u8cmd = send_frame_buffer[CMD_POSITION], // Assuming CMD_POSITION is CMD_POSITION
      .u16VPaddress = VPaddress,
      .xQueueGetResponeData = xQueueResponeDataEvent, // Assuming QueueUartEventSenData is renamed to xQueueResponeDataEvent
  };

  esp_err_t function_status = ESP_FAIL; // Initialize status as failure

  // Acquire mutex before sending the request and waiting for a response
  if (xSemaphoreTake(xMutexPendingRequest, portMAX_DELAY) == pdFALSE)
  {
    ESP_LOGE(__func__, "Failed to acquire xPendingRequest request mutex.");
    return function_status;
  }

  if (xQueueCommandReadWrite != NULL)
  {
    ESP_LOGE(__func__, "Command response queue is not initialized.");
    return function_status;
  }
  if (xQueueSend(xQueueCommandReadWrite, &xrequestInfo, portMAX_DELAY) == pdFALSE)
  {
    ESP_LOGE(__func__, "Failed to send xPendingRequest request to command queue.");
    return function_status;
  }

  // Send data over UART
  _dwinSerial->write(send_frame_buffer, total_frame_length); // Assuming _dwinSerial is _dwinSerial

  // Wait for the response
  uint8_t incoming_response_buffer[MAX_RESPONE_LENGTH] = {0}; // Assuming MAX_RESPONE_LENGTH is MAX_RESPONE_LENGTH
  BaseType_t queue_receive_status = xQueueReceive(xQueueResponeDataEvent, incoming_response_buffer, pdMS_TO_TICKS(1000));
  if (queue_receive_status == pdPASS)
  {
    if (response_buffer != NULL && response_buffer_size_max >= MAX_RESPONE_LENGTH)
    {
      // Copy received data to the caller's pu8BufferReceiver
      memcpy(response_buffer, incoming_response_buffer, MAX_RESPONE_LENGTH);
      function_status = ESP_OK;
    }
    else
    {
      ESP_LOGW(__func__, "Response pu8BufferReceiver is NULL or too small for VP address: 0x%04X", VPaddress);
      function_status = ESP_OK;
    }
  }
  else
  {
    ESP_LOGW(__func__, "No response received for VP address: 0x%04X, Command: 0x%02X", VPaddress, send_frame_buffer[CMD_POSITION]);
  }

  // Remove the xPendingRequest request from the queue (even if response failed)
  xQueueReceive(xQueueCommandReadWrite, &xrequestInfo, 0);

  xSemaphoreGive(xMutexPendingRequest); // Release mutex

  return function_status;
}

esp_err_t DWIN::setPage(uint8_t pageID)
{
  uint8_t sendBuffer[] = {CMD_WRITE, 0x00, 0x84, 0x5A, 0x01, 0x00, pageID};

  return sendArray(sendBuffer, sizeof(sendBuffer) / sizeof(sendBuffer[0]));
}

// set Data on VP Address
esp_err_t DWIN::setText(long address, String textData)
{
  uint16_t sendBufferSize = textData.length() + 5;

  uint8_t sendBuffer[sendBufferSize] = {CMD_WRITE, (uint8_t)((address >> 8) & 0xFF), (uint8_t)((address) & 0xFF)};
  sendBuffer[sendBufferSize - 1] = MAX_ASCII;
  sendBuffer[sendBufferSize - 2] = MAX_ASCII;

  memcpy(sendBuffer + 3, textData.c_str(), textData.length());

  return sendArray(sendBuffer, sizeof(sendBuffer) / sizeof(sendBuffer[0]));
}
// get Data on VP Address
String DWIN::getText(uint16_t vpAddress, uint8_t lenText)
{
  String responeText = "";
  uint8_t numWords = lenText / 2 + 1;
  uint8_t sendBuffer[] = {CMD_READ, (uint8_t)((vpAddress >> 8) & 0xFF), (uint8_t)((vpAddress) & 0xFF), numWords};
  uint8_t pu8Data[MAX_RESPONE_LENGTH] = {};

  esp_err_t ret = sendArray(sendBuffer, sizeof(sendBuffer) / sizeof(sendBuffer[0]), pu8Data, MAX_RESPONE_LENGTH);
  if (ret != ESP_OK)
  {
    return responeText;
  }

  // 5a a5 10 83 80 0 5 41 42 ff ff 0 0 0 0 0 0 fc f6
  uint16_t i = 7;
  do
  {
    if (isascii(pu8Data[i]))
    {
      responeText += (char)pu8Data[i];
    }
    i++;
  } while (_NOT(pu8Data[i] == MAX_ASCII && pu8Data[i + 1] == MAX_ASCII));

  return responeText;
}

// private menthod

bool DWIN::isFirmwareFile(String &fileName)
{
  const char *const pcCONST_END_FILE[] = {
      ".bin", ".BIN", ".icl", ".ICL", ".dzk", ".DZK",
      ".hzk", ".HZK", ".lib", ".LIB", ".wae", ".WAE",
      ".uic", ".UIC", ".gif", ".GIF"};
  for (const auto &ext : pcCONST_END_FILE)
  {
    if (fileName.endsWith(ext))
    {
      return true;
    }
  }
  return false;
}

/**
 *@brief wait until TX have done
 *
 */
void DWIN::flushSerial()
{
  _dwinSerial->flush();
}
void DWIN::clearSerial()
{
  while (_dwinSerial->available())
  {
    _dwinSerial->read();
  }
}
uint16_t DWIN::u16CalculateCRCModbus(uint8_t *data, size_t length)
{
  uint16_t crc = 0xFFFF;

  for (size_t i = 0; i < length; ++i)
  {
    crc ^= data[i];

    for (uint8_t j = 0; j < 8; ++j)
    {
      if (crc & 0x01)
      {
        crc >>= 1;
        crc ^= 0xA001;
      }
      else
      {
        crc >>= 1;
      }
    }
  }

  return crc;
}

void DWIN::xTaskReceiveUartEvent(void *ptr)
{
  DWIN *pxDWIN = (DWIN *)ptr;
  uint32_t notifyNum;
  while (1)
  {
    xTaskNotifyWait(pdFALSE, pdTRUE, &notifyNum, portMAX_DELAY);
    pxDWIN->xHandle();
  }
}
void DWIN::xHandle()
{
  uint8_t pu8BufferReceiver[MAX_RESPONE_LENGTH] = {};
  TouchFrame_t xTouchData = {};
  while (_dwinSerial->available())
  {
    if (_dwinSerial->read() != CMD_HEAD1)
    {
      continue;
    }
    if (_dwinSerial->read() != CMD_HEAD2)
    {
      continue;
    }
    pu8BufferReceiver[FRAME_HEADER_POSTION1] = CMD_HEAD1;
    pu8BufferReceiver[FRAME_HEADER_POSTION2] = CMD_HEAD2;
    uint16_t length = _dwinSerial->read();
    pu8BufferReceiver[LENGTH_SEND_CMD_POSITION] = length;
    _dwinSerial->readBytes(pu8BufferReceiver + CMD_POSITION, length);

    if (_crc)
    {
      uint16_t u16ReceivedCRC;
      memcpy((uint8_t *)&u16ReceivedCRC, &pu8BufferReceiver[length + 1], 2);
      uint16_t u16CalculatedCRC = u16CalculateCRCModbus(pu8BufferReceiver + CMD_POSITION, length - 2);
      if (u16ReceivedCRC != u16CalculatedCRC)
      {
        ESP_LOGE(__func__, "DWIN frame CRC check failed. Received: 0x%04X, Calculated: 0x%04X", u16ReceivedCRC, u16CalculatedCRC);
        continue;
      }
    }
    PendingRequest_t xPendingRequest;
    uint16_t u16VPAddressReceive = _GET_VP_ADDRESS(pu8BufferReceiver, MAX_RESPONE_LENGTH);
    uint8_t cmd = pu8BufferReceiver[CMD_POSITION];
    if (xQueuePeek(xQueueCommandReadWrite, &xPendingRequest, 0) && cmd == xPendingRequest.u8cmd)
    {
      if (cmd == CMD_WRITE || (cmd == CMD_READ) && u16VPAddressReceive == xPendingRequest.u16VPaddress)
      {
        xQueueSend(xPendingRequest.xQueueGetResponeData, pu8BufferReceiver, 10);
        continue;
      }
    }

    // lệnh touch
    xTouchData.u16VPaddress = u16VPAddressReceive;
    xTouchData.u16KeyValue = (((uint16_t)pu8BufferReceiver[7]) << 8) | pu8BufferReceiver[8];
    xQueueSend(xQueueTouch, &xTouchData, 0);
  }
}
void DWIN::vTriggerTaskReceiveFromUartEvent()
{
  xTaskNotify(xTaskDWINHandle, 0x01, eSetBits);
}