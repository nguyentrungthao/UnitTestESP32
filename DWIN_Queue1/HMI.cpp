
#include "HMI.h"
#include "esp_log.h"
#include "freertos/freeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "HMIparam.h"
#include <RTClib.h>
#include <WiFi.h>

#define TAG "HMI"
#define MAX_SEMAPHORE 5
#define INIT_SEMAPHORE 2

#define MAX_INPUT_TEMPERATURE 180
#define MIN_INPUT_TEMPERATURE 20

#define MAX_INPUT_DAY_RTC 31
#define MIN_INPUT_DAY_RTC 1

#define MAX_INPUT_MONTH_RTC 12
#define MIN_INPUT_MONTH_RTC 1

#define MAX_INPUT_YEAR_RTC 2099
#define MIN_INPUT_YEAR_RTC 2000

#define MAX_INPUT_HOUR_RTC 23
#define MIN_INPUT_HOUR_RTC 0

#define MAX_INPUT_MINUTE_RTC 59
#define MIN_INPUT_MINUTE_RTC 0

#define MAX_INPUT_SECOND_RTC 59
#define MIN_INPUT_SECOND_RTC 0

#define MAX_INPUT_DAY_DELAYOFF 99
#define MIN_INPUT_DAY_DELAYOFF 0

#define MAX_INPUT_HOUR_DELAYOFF 23
#define MIN_INPUT_HOUR_DELAYOFF 0

#define MAX_INPUT_MINUTE_DELAYOFF 59
#define MIN_INPUT_MINUTE_DELAYOFF 0

#define MAX_INPUT_HOUR_STERILIZATION_DELAYOFF 2
#define MIN_INPUT_HOUR_STERILIZATION_DELAYOFF 0

#define MAX_INPUT_MINUTE_STERILIZATION_DELAYOFF 59
#define MIN_INPUT_MINUTE_STERILIZATION_DELAYOFF 0

static TaskHandle_t _hmiListenTaskHandle;

HMI::HMI(HardwareSerial &port, uint8_t receivePin, uint8_t transmitPin, long baud)
  : DWIN(port, receivePin, transmitPin, baud),
    _TenChuongTrinh(""),
    _ChiMucChuongTrinhTruocDo(0xffff),
    _ChiMucPhanDoanTruocDo(0xffff),
    _CapslockEnable(false),
    _ChuoiPassword("123456"),
    _DiemChonCalib(eNO_POINT) {
}

void HMI::KhoiTao(uint32_t u32StackDepthReceive, BaseType_t xCoreID) {
  addButtonEvent(_VPAddressCacNutNhan, _KeyValueSetSetpointTemp, _NutCaiNhietDoSetpoint_, this);
  addButtonEvent(_VPAddressCacNutNhan, _KeyValueSetSetpointCO2, _NutCaiCO2Setpoint_, this);
  addButtonEvent(_VPAddressNumericKeypad, _AllKeyValue, _XuLyBanPhim_, this);
  addButtonEvent(_VPAddressCacNutNhan, _KeyValueRunButton, _LayThoiGianBatQuat_, this);
  addButtonEvent(_VPAddressCacNutNhan, _KeyValueCalib, _NutVaoChucNangChonCalib_, this);


  DWIN::begin();
  DWIN::restartHMI();
  DWIN::crcEnabled(true);
  DWIN::setupTouchCallBack(&xQueueTouch, 10);
  xTaskCreatePinnedToCore(xHMITouchTask, "HMItouch", u32StackDepthReceive, (void *)this, configMAX_PRIORITIES - 2, &xHMITouchHdl, xCoreID);
}

void HMI::addButtonEvent(uint16_t vpAddr, int32_t lastBytes, HmiButtonEventCB_t ButtonEventCallback, void *args) {
  HmiEvent event;
  event.vpAddr = vpAddr;
  event.lastBytes = lastBytes;
  event.callBack.buttonEvent = ButtonEventCallback;
  event.eventType = hmiBUTTON;
  event.args = args;
  _eventList.push_back(event);
}

// xử lý sự kiện nhấn nút, khi nhận VP và key từ màn hình về
void HMI::xHMITouchTask(void *ptr) {
  HMI *pHMI = (HMI *)ptr;

  uint32_t notifyNum;
  TouchFrame_t touchRev;
  while (true) {
    xQueueReceive(pHMI->xQueueTouch, &touchRev, portMAX_DELAY);
    pHMI->xHandleInHMITask(touchRev);
  }
}

// logic tìm hàm callback cho VP và key tương ứng
void HMI::xHandleInHMITask(TouchFrame_t xTouchEventData) {
  Serial.printf("%s nhận %x %x\n", __func__, xTouchEventData.u16VPaddress, xTouchEventData.u16KeyValue);
  EventInfo info;
  info.vpAddr = xTouchEventData.u16VPaddress;
  info.lastBytes = xTouchEventData.u16KeyValue;
  for (HmiEvent eventElemen : _eventList) {
    switch (eventElemen.eventType) {
      case hmiBUTTON:
        if (eventElemen.vpAddr == info.vpAddr && eventElemen.lastBytes == 0xffff) {
          eventElemen.callBack.buttonEvent(info.lastBytes, eventElemen.args);
        } //
        else if (eventElemen.vpAddr == info.vpAddr && eventElemen.lastBytes == info.lastBytes) {
          eventElemen.callBack.buttonEvent(info.lastBytes, eventElemen.args);
        }
        break;
      case hmiTEXT:
        if (eventElemen.vpAddr == info.vpAddr) {
          eventElemen.callBack.textReceivedEvent("hasn't process TextEvent yet", eventElemen.args);  // truyen text nhan duoc
        }
        break;
      default:
        Serial.println("The event has not been registered");
        break;
    }
    delay(1);
  }
}

void HMI::DangKyHamSetCallback(hmiSetData_t function) {
  _hmiSetDataCallback = function;
}

void HMI::DangKyHamGetCallback(hmiGetData_t function) {
  _hmiGetDataCallback = function;
}
#pragma region HMI  // các hàm mặc định của HMI
/**
    nên đọc sơ để hiểu các KEY của các nút cơ bản như enter, back, delete, caplock
*/
void HMI::_XuLyBanPhim_(int32_t lastBytes, void *args) {
  HMI *hmiPtr = (HMI *)args;
  if (lastBytes == 0xF0)  // nút back
  {
    hmiPtr->DWIN::setPage(hmiPtr->_set_event.pageAfterReturn);
  } else if (lastBytes == 0xF4)  // Caps lock
  {
    hmiPtr->_CapslockEnable = !hmiPtr->_CapslockEnable;
    if (hmiPtr->_CapslockEnable) {
      hmiPtr->setVPByte(_VPAddressIconLowercaseKeyboard, 1);
    } else {
      hmiPtr->setVPByte(_VPAddressIconLowercaseKeyboard, 0);
    }
  } else if (lastBytes == 0x0D)  // Nút Enter
  {
    // Loại bỏ khoảng trắng 2 đầu chuỗi.
    hmiPtr->_ChuoiBanPhimDangNhap.trim();

    if (hmiPtr->_set_event.displayType == HMI_PASSWORD) {
      if (hmiPtr->SoSanhPassWord(hmiPtr->_ChuoiBanPhimDangNhap)) {
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        hmiPtr->setPage(hmiPtr->_set_event.pageAfterEnter);
        hmiPtr->_DemTrangThaiNext++;
      } else {
        hmiPtr->setText(_VPAddressKeyboardWarningText, "Incorrect");
      }
      return;
    } else if (hmiPtr->_set_event.displayType == HMI_EXTERNAL_PASSWORD) {
      hmiPtr->_set_event.text = hmiPtr->_ChuoiBanPhimDangNhap;
      hmiPtr->setText(hmiPtr->_set_event.VPTextDisplayAfterEnter, hmiPtr->_ChuoiBanPhimDangNhap);
      hmiPtr->setPage(hmiPtr->_set_event.pageAfterEnter);
      hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
      return;
    } else if (hmiPtr->_set_event.displayType == HMI_TEXT) {
      if (hmiPtr->_ChuoiBanPhimDangNhap == "" || hmiPtr->_ChuoiBanPhimDangNhap.indexOf(' ') == 0) {
        hmiPtr->setText(_VPAddressKeyboardWarningText, "Enter name");
        return;
      }
      hmiPtr->_set_event.text = hmiPtr->_ChuoiBanPhimDangNhap;
      hmiPtr->setText(hmiPtr->_set_event.VPTextDisplayAfterEnter, hmiPtr->_ChuoiBanPhimDangNhap);
      hmiPtr->setPage(hmiPtr->_set_event.pageAfterEnter);
      hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
      return;
    }
    float value = hmiPtr->_ChuoiBanPhimDangNhap.toFloat();
    if (value < hmiPtr->_set_event.minValue) {
      if (hmiPtr->_set_event.displayType == HMI_FLOAT) {
        hmiPtr->setText(_VPAddressKeyboardWarningText, "Min: " + String(hmiPtr->_set_event.minValue, 1));
      } else if (hmiPtr->_set_event.displayType == HMI_INT) {
        hmiPtr->setText(_VPAddressKeyboardWarningText, "Min: " + String(hmiPtr->_set_event.minValue, 0));
      }
    } else if (value > hmiPtr->_set_event.maxValue) {
      if (hmiPtr->_set_event.displayType == HMI_FLOAT) {
        hmiPtr->setText(_VPAddressKeyboardWarningText, "Max: " + String(hmiPtr->_set_event.maxValue, 1));
      } else if (hmiPtr->_set_event.displayType == HMI_INT) {
        hmiPtr->setText(_VPAddressKeyboardWarningText, "Max: " + String(hmiPtr->_set_event.maxValue, 0));
      }
    } else {
      switch (hmiPtr->_set_event.displayType) {
        case HMI_FLOAT:
          if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') >= 0) {
            hmiPtr->DWIN::setText(hmiPtr->_set_event.VPTextDisplayAfterEnter, "+" + String(value, 1));
          } else {
            hmiPtr->DWIN::setText(hmiPtr->_set_event.VPTextDisplayAfterEnter, String(value, 1));
          }
          break;
        case HMI_INT:
          if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') >= 0) {
            hmiPtr->DWIN::setText(hmiPtr->_set_event.VPTextDisplayAfterEnter, "+" + String(value, 0));
          } else {
            hmiPtr->DWIN::setText(hmiPtr->_set_event.VPTextDisplayAfterEnter, String(value, 0));
          }
          break;
        default:
          break;
      }
      hmiPtr->_set_event.f_value = value;
      hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
      hmiPtr->DWIN::setPage(hmiPtr->_set_event.pageAfterEnter);
      hmiPtr->_ChuoiBanPhimDangNhap = "";
    }
  } else if (((char)lastBytes == '.' && hmiPtr->_ChuoiBanPhimDangNhap.indexOf('.') != -1) || ((char)lastBytes == '.' && hmiPtr->_set_event.displayType != HMI_FLOAT)) {
    return;
  }
  // 0xF2 là mã của phím xóa
  else if (lastBytes == 0xF2) {
    if ((hmiPtr->_ChuoiBanPhimDangNhap == "-" || hmiPtr->_ChuoiBanPhimDangNhap == "+") && (hmiPtr->_set_event.displayType == HMI_FLOAT || hmiPtr->_set_event.displayType == HMI_INT)) {
      return;
    } else {
      hmiPtr->_ChuoiBanPhimDangNhap.remove(hmiPtr->_ChuoiBanPhimDangNhap.length() - 1);
    }
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
  } else if (hmiPtr->_ChuoiBanPhimDangNhap.length() >= hmiPtr->_set_event.textLen) {
    return;
  } else if (hmiPtr->_set_event.displayType == HMI_INT || hmiPtr->_set_event.displayType == HMI_FLOAT) {
    hmiPtr->_ChuoiBanPhimDangNhap += String((char)lastBytes);
    if (hmiPtr->_ChuoiBanPhimDangNhap.toFloat() > hmiPtr->_set_event.maxValue) {
      // hmiPtr->_ChuoiBanPhimDangNhap.remove(hmiPtr->_ChuoiBanPhimDangNhap.length() - 1);
      if (hmiPtr->_set_event.displayType == HMI_INT) {
        if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') == 0) {
          hmiPtr->_ChuoiBanPhimDangNhap = "+" + String(hmiPtr->_set_event.maxValue, 0);
        } else {
          hmiPtr->_ChuoiBanPhimDangNhap = String(hmiPtr->_set_event.maxValue, 0);
        }
      } else if (hmiPtr->_set_event.displayType == HMI_FLOAT) {
        if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') == 0) {
          hmiPtr->_ChuoiBanPhimDangNhap = "+" + String(hmiPtr->_set_event.maxValue, 1);
        } else {
          hmiPtr->_ChuoiBanPhimDangNhap = String(hmiPtr->_set_event.maxValue, 1);
        }
      }
      hmiPtr->setText(_VPAddressKeyboardWarningText, "Max: " + hmiPtr->_ChuoiBanPhimDangNhap);
      ESP_LOGI(TAG, "Out of range.");
    } else if (hmiPtr->_ChuoiBanPhimDangNhap.toFloat() < hmiPtr->_set_event.minValue && hmiPtr->_set_event.minValue <= 0) {
      if (hmiPtr->_set_event.displayType == HMI_INT) {
        if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') == 0) {
          hmiPtr->_ChuoiBanPhimDangNhap = "+" + String(hmiPtr->_set_event.minValue, 0);
        } else {
          hmiPtr->_ChuoiBanPhimDangNhap = String(hmiPtr->_set_event.minValue, 0);
        }
      } else if (hmiPtr->_set_event.displayType == HMI_FLOAT) {
        if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') == 0) {
          hmiPtr->_ChuoiBanPhimDangNhap = "+" + String(hmiPtr->_set_event.minValue, 1);
        } else {
          hmiPtr->_ChuoiBanPhimDangNhap = String(hmiPtr->_set_event.minValue, 1);
        }
      }
      hmiPtr->setText(_VPAddressKeyboardWarningText, "Min: " + hmiPtr->_ChuoiBanPhimDangNhap);
      ESP_LOGI(TAG, "Out of range.");
    }
  }
  // Text
  else {
    if (hmiPtr->_CapslockEnable) {
      Serial.println(String(lastBytes, HEX));
      hmiPtr->_ChuoiBanPhimDangNhap += String((char)(lastBytes >> 8));
    } else {
      hmiPtr->_ChuoiBanPhimDangNhap += String((char)lastBytes);
    }
  }
  if (hmiPtr->_set_event.displayType == HMI_PASSWORD || hmiPtr->_set_event.displayType == HMI_EXTERNAL_PASSWORD) {
    String _strPass = "";
    for (int8_t i = 0; i < hmiPtr->_ChuoiBanPhimDangNhap.length(); i++) {
      _strPass += "*";
    }
    hmiPtr->DWIN::setText(hmiPtr->_set_event.VPTextDisplayWhenInput, _strPass);
  } else {
    hmiPtr->DWIN::setText(hmiPtr->_set_event.VPTextDisplayWhenInput, hmiPtr->_ChuoiBanPhimDangNhap);
  }
  Serial.println(hmiPtr->_ChuoiBanPhimDangNhap);
  return;
}

bool HMI::SoSanhPassWord(String EnteredPassword) {
  if (EnteredPassword.compareTo(this->_ChuoiPassword) == 0 || EnteredPassword.indexOf(userFACTORY_PASSWORD) >= 0) {
    return 1;
  }
  return 0;
}
#pragma endregion



// các hàm theo ứng dụng

void HMI::_LayThoiGianBatQuat_(int32_t lastBytes, void *args) {
  HMI *hmiPtr = (HMI *)args;
  hmiPtr->_hmiGetDataCallback(HMI_GET_FAN, NULL);  // hàm này dùng để gọi qa chương trình chính in các thông tin cần lên màn hình
}

void HMI::_NutCaiNhietDoSetpoint_(int32_t lastBytes, void *args) {
  HMI *hmiPtr = (HMI *)args;
  hmiPtr->_set_event.type = HMI_SET_SETPOINT_TEMP;  // sau khi enter trên màn hình bàn phím nó gọi callBack được đăng ký DangKyHamSetCallback, truyển vào _set_event và callback
  hmiPtr->_set_event.displayType = HMI_FLOAT;       // bàn phím SỐ bấm được dấu chấm nữa và giá trị trả về qua biến f_value
  uint16_t u16CurrentPage = hmiPtr->getPage();
  hmiPtr->_set_event.pageAfterReturn = u16CurrentPage;                                             // enter nó trả về trang được đặt ở đây
  hmiPtr->_set_event.pageAfterEnter = u16CurrentPage;                                              // nút return, khi nhất nút return
  hmiPtr->_set_event.maxValue = 60.0f;                                                             // giá trị MAX
  hmiPtr->_set_event.minValue = 0.0f;                                                              // giá trị min
  hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressSetpointTempText;                         // sau khi enter giá trị được ghi lên VP này
  hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;                         // trong lúc nhập bàn phím nó hiện lên VP này
  hmiPtr->_set_event.textLen = 5;                                                                  // độ dài của ô VP text sau khi nhấn enter
  hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);  // lấy nội dung đang hiển thị, sau đó ghi lên chuỗi đang nhập
  hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);                     //  =>sau đó ghi lên chuỗi đang nhập
  hmiPtr->setText(_VPAddressKeyboardWarningText, "");                                              // xóa chuỗi cảnh báo, khi nhập quá giới hạn nó ghi lên VP này
  hmiPtr->DWIN::setPage(_TempNumericKeypadPage);                                                   // chuyển đến trang bàn phím
}

void HMI::_NutCaiCO2Setpoint_(int32_t lastBytes, void *args) {
  HMI *hmiPtr = (HMI *)args;
  hmiPtr->_set_event.type = HMI_SET_SETPOINT_CO2;  // sau khi enter trên màn hình bàn phím nó gọi callBack được đăng ký DangKyHamSetCallback, truyển vào _set_event và callback
  hmiPtr->_set_event.displayType = HMI_INT;        /// bàn phím SỐ  KHÔNG bấm được dấu chấm nữa và giá trị trả về qua biến u32_value
  uint16_t u16CurrentPage = hmiPtr->getPage();
  hmiPtr->_set_event.pageAfterReturn = u16CurrentPage;
  hmiPtr->_set_event.pageAfterEnter = u16CurrentPage;
  hmiPtr->_set_event.maxValue = 10;
  hmiPtr->_set_event.minValue = 0;
  hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressSetpointCO2Text;
  hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
  hmiPtr->_set_event.textLen = 5;
  hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
  hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
  hmiPtr->setText(_VPAddressKeyboardWarningText, "");
  hmiPtr->DWIN::setPage(_CO2NumericKeypadPage);
}

void HMI::_NutVaoChucNangChonCalib_(int32_t lastBytes, void *args) {
  HMI *hmiPtr = (HMI *)args;
  hmiPtr->_set_event.type = UNDEFINED;
  hmiPtr->_set_event.displayType = HMI_PASSWORD;  // đưa vào màn hình nhập mật khẩu trước khi vào màn hình cài đặt, mật khẩu khai báo ở hàm constructor 
  hmiPtr->_set_event.pageAfterEnter = _HomePage;
  hmiPtr->_set_event.pageAfterReturn = _SettingsPage;
  hmiPtr->_set_event.textLen = 10;
  hmiPtr->_ChuoiBanPhimDangNhap = "";
  hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
  hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressKeyboardInputText;
  hmiPtr->setText(_VPAddressKeyboardInputText, "Password ?");
  hmiPtr->setText(_VPAddressKeyboardWarningText, "");
  hmiPtr->setPage(_KeyboardPage);
}
