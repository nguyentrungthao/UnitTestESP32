#ifndef _HMI_H_
#define _HMI_H_

/**
 * @file DWIN.h
 * @author Trung Thao (nguyentrungthao1412@gmail.com)
 * @brief tầng UI, quản lý chức năng của các nút nhấn 
 * @version 0.1
 * @date 2025-07-01
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "DWIN.h"
#include "freertos/queue.h"
#include <vector>
#include <TimeLib.h>
#include "userdef.h"
#include "HMIparam.h"
#include <Arduino.h>

typedef enum {
  UNDEFINED,

  HMI_SET_SETPOINT_TEMP,
  HMI_SET_SETPOINT_CO2,
  HMI_SET_CALIB_NHIET,
  HMI_SET_CALIB_CO2,
  HMI_RESET_CALIB_NHIET,
  HMI_RESET_CALIB_CO2,
  eHMI_SET_PERIMETER,
  eHMI_SET_DOOR,
  HMI_SET_DELAYOFF,
  HMI_SET_DELAYOFF_ONOFF,
  HMI_SET_TEMPMAX,
  HMI_SET_TEMPMIN,
  HMI_SET_RTC,
  HMI_SET_ALARM_TEMP_BELOW,
  HMI_SET_ALARM_TEMP_ABOVE,
  HMI_SET_ALARM_CO2_BELOW,
  HMI_SET_ALARM_CO2_ABOVE,
  HMI_SET_TEXT,
  HMI_EDIT_SEG_SETPOINT_TEMP,
  HMI_EDIT_SEG_SETPOINT_CO2,
  HMI_EDIT_SEG_FANSPEED,
  HMI_EDIT_SEG_AIRFLAP,
  HMI_EDIT_SEG_TEMPMIN,
  HMI_EDIT_SEG_TEMPMAX,
  HMI_EDIT_SEG_CO2MIN,
  HMI_EDIT_SEG_CO2MAX,
  HMI_EDIT_SEG_DELAYOFF_DAY,
  HMI_EDIT_SEG_DELAYOFF_HOUR,
  HMI_EDIT_SEG_DELAYOFF_MINUTE,
  HMI_SELECT_SEGMENT,
  HMI_ADD_SEG,
  HMI_SUB_SEG,
  HMI_EDIT_SEG_DAY,
  HMI_EDIT_SEG_HOUR,
  HMI_EDIT_SEG_MINUTE,
  HMI_SAVE_SEG,
  HMI_ADD_PROGRAM,
  HMI_DELETE_PROGRAM,
  HMI_SELECT_PROGRAM,
  HMI_SELECT_PROGRAMNAME,
  HMI_RUN_ePROGRAM_MODE,
  HMI_RUN_eQUICK_MODE,
  HMI_RESET_STER,
  HMI_SET_STER_TEMP,
  HMI_SET_STER_TIME,
  HMI_SET_ICON1,
  HMI_SET_ICON2,
  HMI_EXPORT_DATA,
  HMI_FIRMWARE_USB,
  HMI_FIRMWARE_FOTA,
  HMI_SET_SSID,
  HMI_SET_PASSWORD,
  HMI_SET_SCROLLCHART,
  HMI_CONNECT_OR_DISCONNECT_WIFI,  // truc them
  HMI_CHANGE_ADMIN_PASSWORD,
  eHMI_SET_EVENT_WAKEUP,
  eHMI_SET_PID,
  eHMI_EXIT_PID,
  eHMI_SET_PARAMTER_KP_TEMP_PID,
  eHMI_SET_PARAMTER_KI_TEMP_PID,
  eHMI_SET_PARAMTER_KD_TEMP_PID,
  eHMI_SET_PARAMTER_KW_TEMP_PID,
  eHMI_SET_PARAMTER_IMAX_TEMP_PID,
  eHMI_SET_PARAMTER_IMIN_TEMP_PID,
  eHMI_SET_PARAMTER_OUTMAX_TEMP_PID,
  eHMI_SET_PARAMTER_OUTMIN_TEMP_PID,
  eHMI_SET_PARAMTER_PERIMETER_TEMP,
  eHMI_SET_PARAMTER_DOOR_TEMP_PID,
  eHMI_SET_PARAMTER_KP_CO2_PID,
  eHMI_SET_PARAMTER_KI_CO2_PID,
  eHMI_SET_PARAMTER_KD_CO2_PID,
  eHMI_SET_PARAMTER_KW_CO2_PID,
  eHMI_SET_PARAMTER_IMAX_CO2_PID,
  eHMI_SET_PARAMTER_IMIN_CO2_PID,
  eHMI_SET_PARAMTER_OUTMAX_CO2_PID,
  eHMI_SET_PARAMTER_OUTMIN_CO2_PID,

  HMI_SET_MAX_ENUM
} hmi_set_type_t;
typedef enum {
  HMI_GET_FAN,
  HMI_GET_MAX_ENUM
} hmi_get_type_t;

typedef enum {
  HMI_FLOAT,
  HMI_INT,
  HMI_DATE,
  HMI_TEXT,
  HMI_PASSWORD,
  HMI_EXTERNAL_PASSWORD,  // truc them
} hmi_data_type_t;

typedef struct
{
  hmi_set_type_t type;
  union {
    float f_value;
    int32_t i32_value;
    uint32_t u32_value;
  };
  float maxValue;
  float minValue;
  uint16_t pageAfterEnter;
  uint16_t pageAfterReturn;
  uint16_t VPTextDisplayAfterEnter;
  uint16_t VPTextDisplayWhenInput;
  hmi_data_type_t displayType;
  uint8_t indexList;
  String text;
  int8_t textLen;
} hmi_set_event_t;

struct DuLieuDoThi_t {
  int16_t maxValue = 0;
  int16_t minValue = 0;
  int16_t MulY;
  int16_t VDCentral;
  int16_t valueArr[6];
  time_t timeArr[7];
};

typedef std::function<void(const hmi_set_event_t &event)> hmiSetData_t;
typedef std::function<bool(hmi_get_type_t get, void *args)> hmiGetData_t;

class HMI : public DWIN {
public:
  HMI(HardwareSerial &port, uint8_t receivePin, uint8_t transmitPin, long baud = DWIN_DEFAULT_BAUD_RATE);
  void KhoiTao(uint32_t u32StackDepthReceive = 5120, BaseType_t xCoreID = tskNO_AFFINITY);
  void DangKyHamSetCallback(hmiSetData_t function);
  void DangKyHamGetCallback(hmiGetData_t function);

  void HienThiNgay(int ngay);
  void HienThiThang(int thang);
  void HienThiNam(int nam);
  void HienThiGio(int gio);
  void HienThiPhut(int phut);
  bool SoSanhPassWord(String EnteredPassword);
protected:
  String _TenChuongTrinh;
  String _TenChuongTrinhTruocDo;
  // int _currentProgram;
  int32_t _ChiMucChuongTrinhTruocDo;

  int32_t _ChiMucPhanDoanTruocDo;

  uint32_t _GioTietTrung;
  uint32_t _PhutTietTrung;
  float _NhietDoTietTrung;
  int8_t _DemTrangThaiNext;
  CalibPosition_t _DiemChonCalib;
  String _ChuoiPassword;
  uint8_t _TrangSauKhiNhanReturnTrenWarning;

  DuLieuDoThi_t _DuLieuDoThiNhietDo;
  DuLieuDoThi_t _DuLieuDoThiCO2;


  static void _NutCaiNhietDoSetpoint_(int32_t lastBytes, void *args);
  static void _NutCaiCO2Setpoint_(int32_t lastBytes, void *args);
  static void _LayThoiGianBatQuat_(int32_t lastBytes, void *args);
  static void _NutVaoChucNangChonCalib_(int32_t lastBytes, void *args);



  // các biến và hàm hệ thống quản lý
  static void _XuLyBanPhim_(int32_t lastBytes, void *args);

  hmiSetData_t _hmiSetDataCallback;
  hmiGetData_t _hmiGetDataCallback;

  String _ChuoiBanPhimDangNhap;
  bool _CapslockEnable;
  hmi_set_event_t _set_event;

  typedef void (*HmiButtonEventCB_t)(int32_t lastBytes, void *args);
  typedef void (*HmiTextReceivedEventCB_t)(String text, void *args);
  typedef enum {
    hmiBUTTON,
    hmiTEXT,
  } eventType_t;

  class EventInfo {
  public:
    uint16_t vpAddr;
    int32_t lastBytes;
    eventType_t eventType;
    String data;
    void *args;
  };

  class HmiEvent : public EventInfo {
  public:
    union {
      HmiButtonEventCB_t buttonEvent;
      HmiTextReceivedEventCB_t textReceivedEvent;
    } callBack;
  };
  std::vector<HmiEvent> _eventList;
  std::vector<uint8_t> _rawData;
  TaskHandle_t xHMITouchHdl;
  QueueHandle_t xQueueTouch;
  void addButtonEvent(uint16_t vpAddr, int32_t lastBytes, HmiButtonEventCB_t ButtonEventCallback, void *args);
  static void xHMITouchTask(void *ptr);
  void xHandleInHMITask(TouchFrame_t xTouchEventData);
};
#endif
