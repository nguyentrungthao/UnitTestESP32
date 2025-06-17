
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

HMI::HMI(HardwareSerial &port, uint8_t receivePin, uint8_t transmitPin, long baud) : DWIN(port, receivePin, transmitPin, baud),
                                                                                     _TenChuongTrinh(""),
                                                                                     _ChiMucChuongTrinhTruocDo(0xffff),
                                                                                     _ChiMucPhanDoanTruocDo(0xffff),
                                                                                     _CapslockEnable(false),
                                                                                     _ChuoiPassword("123456"),
                                                                                     _DiemChonCalib(eNO_POINT)
{
}

void HMI::KhoiTao(uint32_t u32StackDepthReceive, BaseType_t xCoreID)
{
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueSetSetpointTemp, _NutCaiNhietDoSetpoint_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueSetSetpointCO2, _NutCaiCO2Setpoint_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueSetFanSpeed, _NutCaiTocDoQuat_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage103Wakeup, _NutThucDay_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage73_EnterService, _NutEnterPageService_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage107_EnterDiagnostic, _NutEnterPageDiagnostic_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage107_EnterPID, _NutEnterPagePID_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage103_ExitPID, _NutExitPID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetTempKP, _NutSetKpTempPID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetTempKI, _NutSetKiTempPID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetTempKD, _NutSetKdTempPID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetTempKW, _NutSetKwTempPID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetTempImax, _NutSetImaxTempPID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetTempImin, _NutSetIminTempPID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetTempOutmax, _NutSetOutmaxTempPID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetTempOutmin, _NutSetOutminTempPID_, this);

    addButtonEvent(_VPAddressPage103PID, _KeyValueSetCO2KP, _NutSetKpCO2PID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetCO2KI, _NutSetKiCO2PID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetCO2KD, _NutSetKdCO2PID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetCO2KW, _NutSetKwCO2PID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetCO2Imax, _NutSetImaxCO2PID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetCO2Imin, _NutSetIminCO2PID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetCO2Outmax, _NutSetOutmaxCO2PID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetCO2Outmin, _NutSetOutminCO2PID_, this);

    addButtonEvent(_VPAddressPage103PID, _KeyValueSetPerimeterOutmax, _NutSetPerimeterPID_, this);
    addButtonEvent(_VPAddressPage103PID, _KeyValueSetDoorOutmax, _NutSetDoorPID_, this);

    addButtonEvent(_VPAddressSegmentSetpointButton, _AllKeyValue, _NutEditSetpointTrangSegment_, this);
    addButtonEvent(_VPAddressSegmentSetpointCO2Button, _AllKeyValue, _NutEditSetpointCO2TrangSegment_, this);
    addButtonEvent(_VPAddressSegmentFanSpeedButton, _AllKeyValue, _NutEditTocDoQuatTrangSegment_, this);
    addButtonEvent(_VPAddressSegmentTempMinButton, _AllKeyValue, _NutEditTempMinTrangSegment_, this);
    addButtonEvent(_VPAddressSegmentTempMaxButton, _AllKeyValue, _NutEditTempMaxTrangSegment_, this);
    addButtonEvent(_VPAddressSegmentCO2MinButton, _AllKeyValue, _NutEditCO2MinTrangSegment_, this);
    addButtonEvent(_VPAddressSegmentCO2MaxButton, _AllKeyValue, _NutEditCO2MaxTrangSegment_, this);

    addButtonEvent(_VPAddressSegmentFunctionButton, _AllKeyValue, _CacNutThaoTacTrongTrangEditSegment_, this);

    addButtonEvent(_VPAddressSegmentDelayOffButton, _AllKeyValue, _NutEditThoiGianTatTrangSegment_, this);
    addButtonEvent(_VPAddressSetTimeButton, _KeyValueSetDay, _NutVaoTrangNhapNgay_, this);
    addButtonEvent(_VPAddressSetTimeButton, _KeyValueSetMonth, _NutVaoTrangNhapThang_, this);
    addButtonEvent(_VPAddressSetTimeButton, _KeyValueSetYear, _NutVaoTrangNhapNam_, this);
    addButtonEvent(_VPAddressSetTimeButton, _KeyValueSetHour, _NutVaoTrangNhapGio_, this);
    addButtonEvent(_VPAddressSetTimeButton, _KeyValueSetMinute, _NutVaoTrangNhapPhut_, this);
    addButtonEvent(_VPAddressSetTimeButton, _KeyValueEnterSegmentDelayOff, _NutEnterTrongCaiDatThoiGianTatCuaSegment_, this);
    addButtonEvent(_VPAddressSegmentSelectButton, _AllKeyValue, _NutChonSegment_, this);

    // calib
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueCalib, _NutVaoChucNangChonCalib_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueChooseCalibTemp, _NutVaoChucNangCalibNhiet_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueChooseCalibCO2, _NutVaoChucNangCalibCO2_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValueEditCalibTemp, _NutEditCalibTemp_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueEnterCalibTemp, _NutEnterTrangCalibNhiet_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueResetCalibTemp, _NutResetHeSoCalibNhiet_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage78_CalibPerimeter, _NutEditHeSoCalibPerimeter_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage78_CalibDoor, _NutEditHeSoCalibDoor_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValueEditCalibCO2, _NutEditCalibCO2_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueEnterCalibCO2, _NutEnterTrangCalibCO2_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueResetCalibCO2, _NutResetHeSoCalibCO2_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage78_calib1, _NutSetChonDiemCalib1Temp_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage78_calib2, _NutSetChonDiemCalib2Temp_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage78_calib3, _NutSetChonDiemCalib3Temp_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage102_calib1, _NutSetChonDiemCalib1CO2_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage102_calib2, _NutSetChonDiemCalib2CO2_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValuePage102_calib3, _NutSetChonDiemCalib3CO2_, this);

    // RTC
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueSetTimeRTC, _NutCaiDatThoiGianRTC_, this);
    addButtonEvent(_VPAddressSetTimeButton, _KeyValueEnterSetRTC, _NutEnterTrangCaiRTC_, this);

    // alarm
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueAlarm, _NutCaiCanhBaoNhietDo_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueEnterSetAlarm, _NutEnterTrangCaiCanhBaoNhietDo_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueSetAlarmBelow, _NutCaiCanhBaoNhietDoThap_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueSetAlarmAbove, _NutCaiCanhBaoNhietDoCao_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueSetAlarmUnderCO2, _NutCaiCanhBaoCO2Thap_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueSetAlarmOverCO2, _NutCaiCanhBaoCO2Cao_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValueProgram, _NutVaoChucNangProgram_, this);
    addButtonEvent(_VPAddressSelectProgram, _AllKeyValue, _NutChonProgram_, this);
    addButtonEvent(_VPAddressProgramFunction, _AllKeyValue, _CacNutThaoTacTrongTrangProgram_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValueSetDelayOff, _NutCaiThoiGianTatMay_, this);
    addButtonEvent(_VPAddressSetTimeButton, _KeyValueEnterSetDelayOff, _NutEnterCaiThoiGianTatMay_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValueRunButton, _NutRun_, this);
    addButtonEvent(_VPAddressSwitchDelayOff, _AllKeyValue, _NutThayDoiTrangThaiChucNangHenGioTat_, this);

    addButtonEvent(_VPAddressEnterRunProgram, _AllKeyValue, _NutEnterTrongTrangProgram_, this);

    addButtonEvent(_VPAddressEnterProgramLoop, _AllKeyValue, _NutEnterChayProgramVoiChuKyDuocChon_, this);
    addButtonEvent(_VPAddressInfButton, _AllKeyValue, _NutInfTrongTrangChonChuKyChayProgram_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValueSterilization, _NutVaoChucNangTietTrung_, this);
    addButtonEvent(_VPAddressCaiThoiGianTietTrung, _AllKeyValue, _NutCaiThoiGianTietTrung_, this);
    addButtonEvent(_VPAddressSetTimeButton, _KeyValueEnterCaiThoiGianTietTrung, _NutEnterCaiThoiGianTietTrung_, this);
    addButtonEvent(_VPAddressCaiNhietDoTietTrung, _AllKeyValue, _NutCaiNhietDoTietTrung_, this);
    addButtonEvent(_VPAddressNextTrongTrangTietTrung, _AllKeyValue, _NutNextTrongCaiTietTrung_, this);
    addButtonEvent(_VPAddressEnterCaiTietTrung, _AllKeyValue, _NutEnterCaiTietTrung_, this);

    addButtonEvent(_VPAddressNumericKeypad, _AllKeyValue, _XuLyBanPhim_, this);
    addButtonEvent(_VPAddressWarningReturn, _AllKeyValue, _NutTroVeTrongTrangWarning_, this);
    addButtonEvent(_VPAddressResetGraph, _AllKeyValue, _NutXoaDoThi_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValueDataRecord, _NutDataRecord_, this);
    addButtonEvent(_VPAddressCacNutNhan, _KeyValueExport, _NutExportData_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValueUpdate, _NutVaoChucNangUpdate_, this);
    addButtonEvent(_VPAddressCacNutChonPhuongThucUpdate, _AllKeyValue, _CacNutChonPhuongThucUpdate_, this);
    addButtonEvent(_VPAddressCacNutTrangFOTA, _AllKeyValue, _CacNutTrangFOTA_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValueWifi, _NutVaoChucNangWiFi_, this); // truc them
    addButtonEvent(_VPAddressCacNutTrangWiFi, _AllKeyValue, _CacNutTrangWiFi_, this);

    addButtonEvent(_VPAddressCacNutNhan, _KeyValueAdminPassword, _NutVaoChucNangThayDoiAdminPassword_, this); // truc them
    addButtonEvent(_VPAddressCacNutTrangAdminPassword, _AllKeyValue, _CacNutTrangThayDoiAdminPassword_, this);

    DWIN::restartHMI();
    DWIN::begin();
    DWIN::crcEnabled(true);
    DWIN::setupTouchCallBack(&xQueueTouch, 5);
    xTaskCreatePinnedToCore(xHMITouchTask, "HMItouch", u32StackDepthReceive, (void *)this, configMAX_PRIORITIES - 2, &xHMITouchHdl, xCoreID);
}

void HMI::addButtonEvent(uint16_t vpAddr, int32_t lastBytes, HmiButtonEventCB_t ButtonEventCallback, void *args)
{
    HmiEvent event;
    event.vpAddr = vpAddr;
    event.lastBytes = lastBytes;
    event.callBack.buttonEvent = ButtonEventCallback;
    event.eventType = hmiBUTTON;
    event.args = args;
    _eventList.push_back(event);
}

void HMI::xHMITouchTask(void *ptr)
{
    HMI *pHMI = (HMI *)ptr;

    uint32_t notifyNum;
    TouchFrame_t touchRev;
    while (true)
    {
        xQueueReceive(pHMI->xQueueTouch, &touchRev, portMAX_DELAY);
        pHMI->xHandleInHMITask(touchRev);
    }
}

void HMI::xHandleInHMITask(TouchFrame_t xTouchEventData)
{
    Serial.printf("%s nhận %x %x\n", __func__, xTouchEventData.u16VPaddress, xTouchEventData.u16KeyValue);
    EventInfo info;
    info.vpAddr = xTouchEventData.u16VPaddress;
    info.lastBytes = xTouchEventData.u16KeyValue; 
    for (HmiEvent eventElemen : _eventList)
    {
        switch (eventElemen.eventType)
        {
        case hmiBUTTON:
            if (eventElemen.vpAddr == info.vpAddr && eventElemen.lastBytes == 0xffff)
            {
                eventElemen.callBack.buttonEvent(info.lastBytes, eventElemen.args);
            }
            else if (eventElemen.vpAddr == info.vpAddr && eventElemen.lastBytes == info.lastBytes)
            {
                eventElemen.callBack.buttonEvent(info.lastBytes, eventElemen.args);
            }
            break;
        case hmiTEXT:
            if (eventElemen.vpAddr == info.vpAddr)
            {
                eventElemen.callBack.textReceivedEvent("hasn't process TextEvent yet", eventElemen.args); // truyen text nhan duoc
            }
            break;
        default:
            Serial.println("The event has not been registered");
            break;
        }
        delay(1);
    }
}

void HMI::DangKyHamSetCallback(hmiSetData_t function)
{
    _hmiSetDataCallback = function;
}

void HMI::DangKyHamGetCallback(hmiGetData_t function)
{
    _hmiGetDataCallback = function;
}
#pragma region Keyboard
void HMI::_XuLyBanPhim_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    if (lastBytes == 0xF0) // nút back
    {
        hmiPtr->DWIN::setPage(hmiPtr->_set_event.pageAfterReturn);
    }
    else if (lastBytes == 0xF4) // Caps lock
    {
        hmiPtr->_CapslockEnable = !hmiPtr->_CapslockEnable;
        if (hmiPtr->_CapslockEnable)
        {
            hmiPtr->setVPByte(_VPAddressIconLowercaseKeyboard, 1);
        }
        else
        {
            hmiPtr->setVPByte(_VPAddressIconLowercaseKeyboard, 0);
        }
    }
    else if (lastBytes == 0x0D) // Nút Enter
    {
        // Loại bỏ khoảng trắng 2 đầu chuỗi.
        hmiPtr->_ChuoiBanPhimDangNhap.trim();

        if (hmiPtr->_set_event.displayType == HMI_PASSWORD)
        {
            if (hmiPtr->SoSanhPassWord(hmiPtr->_ChuoiBanPhimDangNhap))
            {
                hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
                hmiPtr->setPage(hmiPtr->_set_event.pageAfterEnter);
                hmiPtr->_DemTrangThaiNext++;
            }
            else
            {
                hmiPtr->setText(_VPAddressKeyboardWarningText, "Incorrect");
            }
            return;
        }
        else if (hmiPtr->_set_event.displayType == HMI_EXTERNAL_PASSWORD)
        {
            hmiPtr->_set_event.text = hmiPtr->_ChuoiBanPhimDangNhap;
            hmiPtr->setText(hmiPtr->_set_event.VPTextDisplayAfterEnter, hmiPtr->_ChuoiBanPhimDangNhap);
            hmiPtr->setPage(hmiPtr->_set_event.pageAfterEnter);
            hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
            return;
        }
        else if (hmiPtr->_set_event.displayType == HMI_TEXT)
        {
            if (hmiPtr->_ChuoiBanPhimDangNhap == "" || hmiPtr->_ChuoiBanPhimDangNhap.indexOf(' ') == 0)
            {
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
        if (value < hmiPtr->_set_event.minValue)
        {
            if (hmiPtr->_set_event.displayType == HMI_FLOAT)
            {
                hmiPtr->setText(_VPAddressKeyboardWarningText, "Min: " + String(hmiPtr->_set_event.minValue, 1));
            }
            else if (hmiPtr->_set_event.displayType == HMI_INT)
            {
                hmiPtr->setText(_VPAddressKeyboardWarningText, "Min: " + String(hmiPtr->_set_event.minValue, 0));
            }
        }
        else if (value > hmiPtr->_set_event.maxValue)
        {
            if (hmiPtr->_set_event.displayType == HMI_FLOAT)
            {
                hmiPtr->setText(_VPAddressKeyboardWarningText, "Max: " + String(hmiPtr->_set_event.maxValue, 1));
            }
            else if (hmiPtr->_set_event.displayType == HMI_INT)
            {
                hmiPtr->setText(_VPAddressKeyboardWarningText, "Max: " + String(hmiPtr->_set_event.maxValue, 0));
            }
        }
        else
        {
            switch (hmiPtr->_set_event.displayType)
            {
            case HMI_FLOAT:
                if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') >= 0)
                {
                    hmiPtr->DWIN::setText(hmiPtr->_set_event.VPTextDisplayAfterEnter, "+" + String(value, 1));
                }
                else
                {
                    hmiPtr->DWIN::setText(hmiPtr->_set_event.VPTextDisplayAfterEnter, String(value, 1));
                }
                break;
            case HMI_INT:
                if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') >= 0)
                {
                    hmiPtr->DWIN::setText(hmiPtr->_set_event.VPTextDisplayAfterEnter, "+" + String(value, 0));
                }
                else
                {
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
    }
    else if (((char)lastBytes == '.' && hmiPtr->_ChuoiBanPhimDangNhap.indexOf('.') != -1) || ((char)lastBytes == '.' && hmiPtr->_set_event.displayType != HMI_FLOAT))
    {
        return;
    }
    // 0xF2 là mã của phím xóa
    else if (lastBytes == 0xF2)
    {
        if ((hmiPtr->_ChuoiBanPhimDangNhap == "-" || hmiPtr->_ChuoiBanPhimDangNhap == "+") && (hmiPtr->_set_event.displayType == HMI_FLOAT || hmiPtr->_set_event.displayType == HMI_INT))
        {
            return;
        }
        else
        {
            hmiPtr->_ChuoiBanPhimDangNhap.remove(hmiPtr->_ChuoiBanPhimDangNhap.length() - 1);
        }
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    }
    else if (hmiPtr->_ChuoiBanPhimDangNhap.length() >= hmiPtr->_set_event.textLen)
    {
        return;
    }
    else if (hmiPtr->_set_event.displayType == HMI_INT || hmiPtr->_set_event.displayType == HMI_FLOAT)
    {
        hmiPtr->_ChuoiBanPhimDangNhap += String((char)lastBytes);
        if (hmiPtr->_ChuoiBanPhimDangNhap.toFloat() > hmiPtr->_set_event.maxValue)
        {
            // hmiPtr->_ChuoiBanPhimDangNhap.remove(hmiPtr->_ChuoiBanPhimDangNhap.length() - 1);
            if (hmiPtr->_set_event.displayType == HMI_INT)
            {
                if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') == 0)
                {
                    hmiPtr->_ChuoiBanPhimDangNhap = "+" + String(hmiPtr->_set_event.maxValue, 0);
                }
                else
                {
                    hmiPtr->_ChuoiBanPhimDangNhap = String(hmiPtr->_set_event.maxValue, 0);
                }
            }
            else if (hmiPtr->_set_event.displayType == HMI_FLOAT)
            {
                if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') == 0)
                {
                    hmiPtr->_ChuoiBanPhimDangNhap = "+" + String(hmiPtr->_set_event.maxValue, 1);
                }
                else
                {
                    hmiPtr->_ChuoiBanPhimDangNhap = String(hmiPtr->_set_event.maxValue, 1);
                }
            }
            hmiPtr->setText(_VPAddressKeyboardWarningText, "Max: " + hmiPtr->_ChuoiBanPhimDangNhap);
            ESP_LOGI(TAG, "Out of range.");
        }
        else if (hmiPtr->_ChuoiBanPhimDangNhap.toFloat() < hmiPtr->_set_event.minValue && hmiPtr->_set_event.minValue <= 0)
        {
            if (hmiPtr->_set_event.displayType == HMI_INT)
            {
                if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') == 0)
                {
                    hmiPtr->_ChuoiBanPhimDangNhap = "+" + String(hmiPtr->_set_event.minValue, 0);
                }
                else
                {
                    hmiPtr->_ChuoiBanPhimDangNhap = String(hmiPtr->_set_event.minValue, 0);
                }
            }
            else if (hmiPtr->_set_event.displayType == HMI_FLOAT)
            {
                if (hmiPtr->_ChuoiBanPhimDangNhap.indexOf('+') == 0)
                {
                    hmiPtr->_ChuoiBanPhimDangNhap = "+" + String(hmiPtr->_set_event.minValue, 1);
                }
                else
                {
                    hmiPtr->_ChuoiBanPhimDangNhap = String(hmiPtr->_set_event.minValue, 1);
                }
            }
            hmiPtr->setText(_VPAddressKeyboardWarningText, "Min: " + hmiPtr->_ChuoiBanPhimDangNhap);
            ESP_LOGI(TAG, "Out of range.");
        }
    }
    // Text
    else
    {
        if (hmiPtr->_CapslockEnable)
        {
            Serial.println(String(lastBytes, HEX));
            hmiPtr->_ChuoiBanPhimDangNhap += String((char)(lastBytes >> 8));
        }
        else
        {
            hmiPtr->_ChuoiBanPhimDangNhap += String((char)lastBytes);
        }
    }
    if (hmiPtr->_set_event.displayType == HMI_PASSWORD || hmiPtr->_set_event.displayType == HMI_EXTERNAL_PASSWORD)
    {
        String _strPass = "";
        for (int8_t i = 0; i < hmiPtr->_ChuoiBanPhimDangNhap.length(); i++)
        {
            _strPass += "*";
        }
        hmiPtr->DWIN::setText(hmiPtr->_set_event.VPTextDisplayWhenInput, _strPass);
    }
    else
    {
        hmiPtr->DWIN::setText(hmiPtr->_set_event.VPTextDisplayWhenInput, hmiPtr->_ChuoiBanPhimDangNhap);
    }
    Serial.println(hmiPtr->_ChuoiBanPhimDangNhap);
    return;
}
#pragma endregion

void HMI::_NutCaiNhietDoSetpoint_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = HMI_SET_SETPOINT_TEMP;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    uint16_t u16CurrentPage = hmiPtr->getPage();
    hmiPtr->_set_event.pageAfterReturn = u16CurrentPage;
    hmiPtr->_set_event.pageAfterEnter = u16CurrentPage;
    hmiPtr->_set_event.maxValue = userSETPOINT_TEMP_MAX;
    hmiPtr->_set_event.minValue = userSETPOINT_TEMP_MIN;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressSetpointTempText;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_TempNumericKeypadPage);
}

void HMI::_NutCaiCO2Setpoint_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = HMI_SET_SETPOINT_CO2;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    uint16_t u16CurrentPage = hmiPtr->getPage();
    hmiPtr->_set_event.pageAfterReturn = u16CurrentPage;
    hmiPtr->_set_event.pageAfterEnter = u16CurrentPage;
    hmiPtr->_set_event.maxValue = userSETPOINT_CO2_MAX;
    hmiPtr->_set_event.minValue = userSETPOINT_CO2_MIN;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressSetpointCO2Text;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_CO2NumericKeypadPage);
}

void HMI::_NutCaiTocDoQuat_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = HMI_SET_FAN;
    hmiPtr->_set_event.displayType = HMI_INT;
    uint16_t u16CurrentPage = hmiPtr->getPage();
    hmiPtr->_set_event.pageAfterReturn = u16CurrentPage;
    hmiPtr->_set_event.pageAfterEnter = u16CurrentPage;
    hmiPtr->_set_event.maxValue = 100;
    hmiPtr->_set_event.minValue = 5;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressFanSpeedText;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_FanNumericKeypadPage);
}

void HMI::_NutThucDay_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_EVENT_WAKEUP;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
    hmiPtr->_hmiGetDataCallback(eHMI_EVENT_REFRESH, NULL);
}

void HMI::_NutEnterPageService_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_PASSWORD;
    hmiPtr->_set_event.pageAfterEnter = _ServicePage;
    hmiPtr->_set_event.pageAfterReturn = _Settings2Page;
    hmiPtr->_set_event.textLen = 10;
    hmiPtr->_ChuoiBanPhimDangNhap = "";
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressKeyboardInputText;
    hmiPtr->setText(_VPAddressKeyboardInputText, "Password ?");
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->setPage(_KeyboardPage);
}
void HMI::_NutEnterPageDiagnostic_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.pageAfterReturn = _ServicePage;
    hmiPtr->setPage(_DiagnosticsPage);
}

void HMI::_NutEnterPagePID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PID;
    hmiPtr->_set_event.pageAfterReturn = _ServicePage;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
}
void HMI::_NutExitPID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_EXIT_PID;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
}
void HMI::_NutSetKpTempPID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_KP_TEMP_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103KpTemp;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}

void HMI::_NutSetKiTempPID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_KI_TEMP_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103KiTemp;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}

void HMI::_NutSetKdTempPID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_KD_TEMP_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103KdTemp;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}
void HMI::_NutSetKwTempPID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_KW_TEMP_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103KwTemp;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}
void HMI::_NutSetImaxTempPID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_IMAX_TEMP_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103ImaxTemp;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}
void HMI::_NutSetIminTempPID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_IMIN_TEMP_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = -0.1;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103IminTemp;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}
void HMI::_NutSetOutmaxTempPID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_OUTMAX_TEMP_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103OmaxTemp;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}
void HMI::_NutSetOutminTempPID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_OUTMIN_TEMP_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103OminTemp;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}

void HMI::_NutSetKpCO2PID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_KP_CO2_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103KpCO2;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}

void HMI::_NutSetKiCO2PID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_KI_CO2_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103KiCO2;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}

void HMI::_NutSetKdCO2PID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_KD_CO2_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103KdCO2;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}
void HMI::_NutSetKwCO2PID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_KW_CO2_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103KwCO2;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}
void HMI::_NutSetImaxCO2PID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_IMAX_CO2_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103ImaxCO2;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}
void HMI::_NutSetIminCO2PID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_IMIN_CO2_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = -0.1;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103IminCO2;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}
void HMI::_NutSetOutmaxCO2PID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_OUTMAX_CO2_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103OmaxCO2;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}
void HMI::_NutSetOutminCO2PID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_OUTMIN_CO2_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103OminCO2;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}

void HMI::_NutSetPerimeterPID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_PERIMETER_TEMP;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103PremeterHeater;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}
void HMI::_NutSetDoorPID_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = eHMI_SET_PARAMTER_DOOR_TEMP_PID;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _PIDPage;
    hmiPtr->_set_event.pageAfterEnter = _PIDPage;
    hmiPtr->_set_event.maxValue = 10000;
    hmiPtr->_set_event.minValue = -10000;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage103DoorHeater;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->DWIN::setPage(_NumericKeypadPage);
}

void HMI::_NutCaiThoiGianTatMay_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_hmiGetDataCallback(HMI_GET_DELAYOFF, NULL);
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->setPage(_SetDelayOffPage);
}

void HMI::_NutEnterCaiThoiGianTatMay_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = HMI_SET_DELAYOFF;
    hmiPtr->_set_event.u32_value = hmiPtr->getText(_VPAddressDayText, 2).toInt() * 86400 +
                                   hmiPtr->getText(_VPAddressHourText, 2).toInt() * 3600 +
                                   hmiPtr->getText(_VPAddressMinuteText, 2).toInt() * 60;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
    hmiPtr->setPage(_HomePage);
}

void HMI::_NutEditSetpointTrangSegment_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    ESP_LOGI(TAG, "Select %d\n", lastBytes);
    hmiPtr->_set_event.type = HMI_EDIT_SEG_SETPOINT_TEMP;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _SegmentAdjPage;
    hmiPtr->_set_event.pageAfterEnter = _SegmentAdjPage;
    hmiPtr->_set_event.maxValue = 60;
    hmiPtr->_set_event.minValue = 20;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressSegmentSetpointText1 + lastBytes * 5;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.indexList = lastBytes;
    if (hmiPtr->_hmiGetDataCallback(HMI_CHECK_LIST, (void *)&hmiPtr->_set_event.indexList) == true)
    {
        hmiPtr->DWIN::setPage(_TempNumericKeypadPage);
    }
}
void HMI::_NutEditSetpointCO2TrangSegment_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    ESP_LOGI(TAG, "Select %d\n", lastBytes);
    hmiPtr->_set_event.type = HMI_EDIT_SEG_SETPOINT_CO2;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _SegmentAdjPage;
    hmiPtr->_set_event.pageAfterEnter = _SegmentAdjPage;
    hmiPtr->_set_event.maxValue = 20;
    hmiPtr->_set_event.minValue = 0;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressSegmentSetpointCO2Text1 + lastBytes * 5;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.indexList = lastBytes;
    if (hmiPtr->_hmiGetDataCallback(HMI_CHECK_LIST, (void *)&hmiPtr->_set_event.indexList) == true)
    {
        hmiPtr->DWIN::setPage(_CO2NumericKeypadPage);
    }
}

void HMI::_NutEditTocDoQuatTrangSegment_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    ESP_LOGI(TAG, "Select %d\n", lastBytes);
    hmiPtr->_set_event.type = HMI_EDIT_SEG_FANSPEED;
    hmiPtr->_set_event.displayType = HMI_INT;
    hmiPtr->_set_event.pageAfterReturn = _SegmentAdjPage;
    hmiPtr->_set_event.pageAfterEnter = _SegmentAdjPage;
    hmiPtr->_set_event.maxValue = 100;
    hmiPtr->_set_event.minValue = 5;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressSegmentFanSpeedText1 + lastBytes * 5;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.indexList = lastBytes;
    if (hmiPtr->_hmiGetDataCallback(HMI_CHECK_LIST, (void *)&hmiPtr->_set_event.indexList) == true)
    {
        hmiPtr->DWIN::setPage(_FanNumericKeypadPage);
    }
}

void HMI::_NutEditTempMinTrangSegment_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    ESP_LOGI(TAG, "Select %d\n", lastBytes);
    hmiPtr->_set_event.type = HMI_EDIT_SEG_TEMPMIN;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _SegmentAdjPage;
    hmiPtr->_set_event.pageAfterEnter = _SegmentAdjPage;
    hmiPtr->_set_event.maxValue = -0.1;
    hmiPtr->_set_event.minValue = -10;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressSegmentTempMinText1 + lastBytes * 5;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.indexList = lastBytes;
    if (hmiPtr->_hmiGetDataCallback(HMI_CHECK_LIST, (void *)&hmiPtr->_set_event.indexList) == true)
    {
        hmiPtr->DWIN::setPage(_NumericKeypadPage);
    }
}
void HMI::_NutEditCO2MinTrangSegment_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    ESP_LOGI(TAG, "Select %d\n", lastBytes);
    hmiPtr->_set_event.type = HMI_EDIT_SEG_CO2MIN;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _SegmentAdjPage;
    hmiPtr->_set_event.pageAfterEnter = _SegmentAdjPage;
    hmiPtr->_set_event.maxValue = -0.1;
    hmiPtr->_set_event.minValue = -10;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressSegmentCO2MinText1 + lastBytes * 5;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.indexList = lastBytes;
    if (hmiPtr->_hmiGetDataCallback(HMI_CHECK_LIST, (void *)&hmiPtr->_set_event.indexList) == true)
    {
        hmiPtr->DWIN::setPage(_NumericKeypadPage);
    }
}
void HMI::_NutEditCO2MaxTrangSegment_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    ESP_LOGI(TAG, "Select %d\n", lastBytes);
    hmiPtr->_set_event.type = HMI_EDIT_SEG_CO2MAX;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _SegmentAdjPage;
    hmiPtr->_set_event.pageAfterEnter = _SegmentAdjPage;
    hmiPtr->_set_event.maxValue = 10;
    hmiPtr->_set_event.minValue = 0.1;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressSegmentCO2MaxText1 + lastBytes * 5;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.indexList = lastBytes;
    if (hmiPtr->_hmiGetDataCallback(HMI_CHECK_LIST, (void *)&hmiPtr->_set_event.indexList) == true)
    {
        hmiPtr->DWIN::setPage(_NumericKeypadPage);
    }
}
void HMI::_NutEditTempMaxTrangSegment_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    ESP_LOGI(TAG, "Select %d\n", lastBytes);
    hmiPtr->_set_event.type = HMI_EDIT_SEG_TEMPMAX;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterReturn = _SegmentAdjPage;
    hmiPtr->_set_event.pageAfterEnter = _SegmentAdjPage;
    hmiPtr->_set_event.maxValue = 10;
    hmiPtr->_set_event.minValue = 0.1;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressSegmentTempMaxText1 + lastBytes * 5;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.indexList = lastBytes;
    if (hmiPtr->_hmiGetDataCallback(HMI_CHECK_LIST, (void *)&hmiPtr->_set_event.indexList) == true)
    {
        hmiPtr->DWIN::setPage(_NumericKeypadPage);
    }
}

void HMI::_NutChonSegment_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    ESP_LOGI(TAG, "Select %d\n", lastBytes);
    hmiPtr->_set_event.indexList = lastBytes;
    if (hmiPtr->_hmiGetDataCallback(HMI_CHECK_LIST, (void *)&hmiPtr->_set_event.indexList) == true)
    {
        if (hmiPtr->_ChiMucPhanDoanTruocDo != lastBytes)
        {
            hmiPtr->setVPByte(_VPAddressSegmentIconTick1 + lastBytes, true);
            if (hmiPtr->_ChiMucPhanDoanTruocDo != 0xffff)
            {
                hmiPtr->setVPByte(_VPAddressSegmentIconTick1 + hmiPtr->_ChiMucPhanDoanTruocDo, false);
            }
            hmiPtr->_ChiMucPhanDoanTruocDo = lastBytes;
        }
        else
        {
            hmiPtr->setVPByte(_VPAddressSegmentIconTick1 + lastBytes, false);
            hmiPtr->_ChiMucPhanDoanTruocDo = 0xffff;
        }
    }
}

// void HMI::_SegmentButton_(int32_t lastBytes, void *args)
// {
//     HMI *hmiPtr = (HMI*)args;
//     hmiPtr->_hmiGetDataCallback(HMI_GET_SEGMENT_LIST, hmiPtr);
//     hmiPtr->DWIN::setPage(21);
// }

void HMI::_CacNutThaoTacTrongTrangEditSegment_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    switch (lastBytes)
    {
    case _KeyValueSegmentAddButton:
        hmiPtr->_set_event.type = HMI_ADD_SEG;
        hmiPtr->_set_event.indexList = hmiPtr->_ChiMucPhanDoanTruocDo;
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        hmiPtr->_hmiGetDataCallback(HMI_REFRESH_SEGMENT_LIST, NULL);
        ESP_LOGI(TAG, "%s", __func__);
        break;
    case _KeyValueSegmentSubButton:
        hmiPtr->_set_event.type = HMI_SUB_SEG;
        hmiPtr->_set_event.indexList = hmiPtr->_ChiMucPhanDoanTruocDo;
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        hmiPtr->_hmiGetDataCallback(HMI_REFRESH_SEGMENT_LIST, NULL);
        ESP_LOGI(TAG, "%s", __func__);
        break;
    case _KeyValueSegmentUpButton:
        hmiPtr->_hmiGetDataCallback(HMI_GET_NEXT_SEGMENT_LIST, hmiPtr);
        ESP_LOGI(TAG, "%s", __func__);
        break;
    case _KeyValueSegmentDownButton:
        hmiPtr->_hmiGetDataCallback(HMI_GET_BACK_SEGMENT_LIST, hmiPtr);
        ESP_LOGI(TAG, "%s", __func__);
        break;
    case _KeyValueSegmentEnterButton:
        hmiPtr->_set_event.type = HMI_SAVE_SEG;
        hmiPtr->_set_event.text = hmiPtr->getText(_VPAddressCurrentProgramNameText, 16);
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        hmiPtr->setPage(_ProgramPage);
        hmiPtr->_hmiGetDataCallback(HMI_GET_PROGRAM_LIST, NULL);
        ESP_LOGI(TAG, "%s", __func__);
        break;
    case _KeyValueSegmentEditProgramNameButton:
        hmiPtr->_set_event.type = UNDEFINED;
        hmiPtr->_set_event.displayType = HMI_TEXT;
        hmiPtr->_set_event.pageAfterEnter = _SegmentAdjPage;
        hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter;
        hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressCurrentProgramNameText;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->_set_event.textLen = 15;
        hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(_VPAddressCurrentProgramNameText, 16);
        hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
        hmiPtr->setPage(_KeyboardPage);
        break;
    default:
        break;
    }
    return;
}

void HMI::HienThiDuLieuSegmentTrenHang(uint8_t row, uint8_t index, float setpoint, float setpointCO2, int delayOffDay, int delayOffHour, int delayOffMinute,
                                       int fanSpeed, float tempMin, float tempMax, float CO2Min, float CO2Max)
{
    this->setText(_VPAddressNumSegmentText1 + row * 2, String(index));
    this->setText(_VPAddressSegmentSetpointText1 + row * 5, String(setpoint, 1));
    this->setText(_VPAddressSegmentSetpointCO2Text1 + row * 5, String(setpointCO2, 1));
    char timeString[20];
    sprintf(timeString, "%02dd-%02d:%02d", delayOffDay, delayOffHour, delayOffMinute);
    this->setText(_VPAddressSegmentDelayOffText1 + row * 20, String(timeString));
    this->setText(_VPAddressSegmentFanSpeedText1 + row * 5, String(fanSpeed));
    // this->setText(_VPAddressSegmentAirFlapText1 + row * 5, String(flap));
    this->setText(_VPAddressSegmentTempMinText1 + row * 5, String(tempMin, 1));
    this->setText(_VPAddressSegmentTempMaxText1 + row * 5, String(tempMax, 1));
    this->setText(_VPAddressSegmentCO2MinText1 + row * 5, String(CO2Min, 1));
    this->setText(_VPAddressSegmentCO2MaxText1 + row * 5, String(CO2Max, 1));
    this->setVPByte(_VPAddressSegmentIconTick1 + row, false);
    this->_ChiMucPhanDoanTruocDo = 0xffff;
}

void HMI::XoaDuLieuHienThiSegmentTrenHang(uint8_t row)
{
    this->setText(_VPAddressNumSegmentText1 + row * 2, "");
    this->setText(_VPAddressSegmentSetpointText1 + row * 5, "");
    this->setText(_VPAddressSegmentSetpointCO2Text1 + row * 5, "");
    this->setText(_VPAddressSegmentDelayOffText1 + row * 20, "");
    this->setText(_VPAddressSegmentFanSpeedText1 + row * 5, "");
    // this->setText(_VPAddressSegmentAirFlapText1 + row * 5, "");
    this->setText(_VPAddressSegmentTempMinText1 + row * 5, "");
    this->setText(_VPAddressSegmentTempMaxText1 + row * 5, "");
    this->setText(_VPAddressSegmentCO2MinText1 + row * 5, "");
    this->setText(_VPAddressSegmentCO2MaxText1 + row * 5, "");
    this->setVPByte(_VPAddressSegmentIconTick1 + row, false);
    this->_ChiMucPhanDoanTruocDo = 0xffff;
}

void HMI::_NutEditThoiGianTatTrangSegment_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.indexList = lastBytes;
    hmiPtr->_set_event.type = UNDEFINED;
    if (hmiPtr->_hmiGetDataCallback(HMI_CHECK_LIST, (void *)&hmiPtr->_set_event.indexList) == true)
    {
        hmiPtr->_hmiGetDataCallback(HMI_GET_SEGMENT_DELAYOFF, (void *)&hmiPtr->_set_event.indexList);
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
        hmiPtr->DWIN::setPage(_SetSegmentDelayOffPage);
    }
}

void HMI::HienThiNgay(int ngay)
{
    this->setText(_VPAddressDayText, String(ngay));
}

void HMI::HienThiThang(int thang)
{
    this->setText(_VPAddressMonthText, String(thang));
}

void HMI::HienThiNam(int nam)
{
    this->setText(_VPAddressYearText, String(nam));
}

void HMI::HienThiGio(int gio)
{
    this->setText(_VPAddressHourText, String(gio));
}

void HMI::HienThiPhut(int phut)
{
    this->setText(_VPAddressMinuteText, String(phut));
}

void HMI::_NutVaoTrangNhapNam_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_INT;
    hmiPtr->_set_event.minValue = MIN_INPUT_YEAR_RTC;
    hmiPtr->_set_event.maxValue = MAX_INPUT_YEAR_RTC;
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter = hmiPtr->getPage();
    if (hmiPtr->_set_event.pageAfterEnter != _RTCPage)
    {
        return;
    }
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressYearText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 4);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 4;
    hmiPtr->setPage(_NumericKeypadPage);
}

void HMI::_NutVaoTrangNhapThang_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_INT;
    hmiPtr->_set_event.minValue = MIN_INPUT_MONTH_RTC;
    hmiPtr->_set_event.maxValue = MAX_INPUT_MONTH_RTC;
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter = hmiPtr->getPage();
    if (hmiPtr->_set_event.pageAfterEnter != _RTCPage)
    {
        return;
    }
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressMonthText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 2);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 2;
    hmiPtr->setPage(_NumericKeypadPage);
}

void HMI::_NutVaoTrangNhapNgay_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_INT;
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter = hmiPtr->getPage();
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressDayText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 2);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 2;
    if (hmiPtr->_set_event.pageAfterEnter == _RTCPage)
    {
        hmiPtr->_set_event.minValue = MIN_INPUT_DAY_RTC;
        hmiPtr->_set_event.maxValue = MAX_INPUT_DAY_RTC;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->setPage(_NumericKeypadPage);
    }
    else if (hmiPtr->_set_event.pageAfterEnter == _SetDelayOffPage || hmiPtr->_set_event.pageAfterEnter == _SetSegmentDelayOffPage)
    {
        hmiPtr->_set_event.minValue = MIN_INPUT_DAY_DELAYOFF;
        hmiPtr->_set_event.maxValue = MAX_INPUT_DAY_DELAYOFF;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressDayText;
        // hmiPtr->setPage(150);
    }
    else
    {
        return;
    }
}

void HMI::_NutVaoTrangNhapGio_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_INT;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressHourText;
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter = hmiPtr->getPage();
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 2);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 2;
    if (hmiPtr->_set_event.pageAfterEnter == _RTCPage)
    {
        hmiPtr->_set_event.minValue = MIN_INPUT_HOUR_RTC;
        hmiPtr->_set_event.maxValue = MAX_INPUT_HOUR_RTC;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->setPage(_NumericKeypadPage);
    }
    else if (hmiPtr->_set_event.pageAfterEnter == _TrangCaiThoiGianTietTrung)
    {
        hmiPtr->_set_event.minValue = MIN_INPUT_HOUR_STERILIZATION_DELAYOFF;
        hmiPtr->_set_event.maxValue = MAX_INPUT_HOUR_STERILIZATION_DELAYOFF;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressHourText;
    }
    else if (hmiPtr->_set_event.pageAfterEnter == _SetDelayOffPage || hmiPtr->_set_event.pageAfterEnter == _SetSegmentDelayOffPage)
    {
        hmiPtr->_set_event.minValue = MIN_INPUT_HOUR_DELAYOFF;
        hmiPtr->_set_event.maxValue = MAX_INPUT_HOUR_DELAYOFF;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressHourText;
    }
    else
    {
        return;
    }
}

void HMI::_NutVaoTrangNhapPhut_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_INT;
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter = hmiPtr->getPage();
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressMinuteText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 2);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 2;
    if (hmiPtr->_set_event.pageAfterEnter == _RTCPage)
    {
        hmiPtr->_set_event.minValue = MIN_INPUT_MINUTE_RTC;
        hmiPtr->_set_event.maxValue = MAX_INPUT_MINUTE_RTC;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->setPage(_NumericKeypadPage);
    }
    else if (hmiPtr->_set_event.pageAfterEnter == _TrangCaiThoiGianTietTrung)
    {
        hmiPtr->_set_event.minValue = MIN_INPUT_MINUTE_STERILIZATION_DELAYOFF;
        hmiPtr->_set_event.maxValue = MAX_INPUT_MINUTE_STERILIZATION_DELAYOFF;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressMinuteText;
    }
    else if (hmiPtr->_set_event.pageAfterEnter == _SetDelayOffPage || hmiPtr->_set_event.pageAfterEnter == _SetSegmentDelayOffPage)
    {
        hmiPtr->_set_event.minValue = MIN_INPUT_MINUTE_DELAYOFF;
        hmiPtr->_set_event.maxValue = MAX_INPUT_MINUTE_DELAYOFF;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressMinuteText;
    }
    else
    {
        return;
    }
}

void HMI::_NutEnterTrongCaiDatThoiGianTatCuaSegment_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;

    int Day = hmiPtr->getText(_VPAddressDayText, 2).toInt();
    if (Day > MAX_INPUT_DAY_DELAYOFF)
    {
        return;
    }
    hmiPtr->_set_event.type = HMI_EDIT_SEG_DELAYOFF_DAY;
    hmiPtr->_set_event.u32_value = Day;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

    int Hour = hmiPtr->getText(_VPAddressHourText, 2).toInt();
    if (Hour > MAX_INPUT_HOUR_DELAYOFF)
    {
        return;
    }
    hmiPtr->_set_event.type = HMI_EDIT_SEG_DELAYOFF_HOUR;
    hmiPtr->_set_event.u32_value = Hour;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

    int Minute = hmiPtr->getText(_VPAddressMinuteText, 2).toInt();
    if (Minute > MAX_INPUT_MINUTE_DELAYOFF)
    {
        return;
    }
    hmiPtr->_set_event.type = HMI_EDIT_SEG_DELAYOFF_MINUTE;
    hmiPtr->_set_event.u32_value = Minute;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

    char strTime[15];
    sprintf(strTime, "%02dd-%02d:%02d", Day, Hour, Minute);
    hmiPtr->setText(_VPAddressSegmentDelayOffText1 + hmiPtr->_set_event.indexList * 20, strTime);

    hmiPtr->setPage(_SegmentAdjPage);
}

void HMI::_NutVaoChucNangChonCalib_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_PASSWORD;
    hmiPtr->_set_event.pageAfterEnter = _CalibChoosePage;
    hmiPtr->_set_event.pageAfterReturn = _SettingsPage;
    hmiPtr->_set_event.textLen = 10;
    hmiPtr->_ChuoiBanPhimDangNhap = "";
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressKeyboardInputText;
    hmiPtr->setText(_VPAddressKeyboardInputText, "Password ?");
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->setPage(_KeyboardPage);
}

void HMI::_NutVaoChucNangCalibNhiet_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_DiemChonCalib = eNO_POINT;
    hmiPtr->_hmiGetDataCallback(HMI_GET_CALIB_TEMP, NULL);
    hmiPtr->setText(_VPAddressStdTempText, "");
    hmiPtr->setText(_VPAddressPage78_infoTemp, "");
    hmiPtr->setTextColor(_SPAddressPage78_point1Temp, 0x03, _BlueColor);
    hmiPtr->setTextColor(_SPAddressPage78_point2Temp, 0x03, _BlueColor);
    hmiPtr->setTextColor(_SPAddressPage78_point3Temp, 0x03, _BlueColor);
    hmiPtr->setPage(_CalibTempPage);
}

void HMI::_NutVaoChucNangCalibCO2_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_DiemChonCalib = eNO_POINT;
    hmiPtr->_hmiGetDataCallback(HMI_GET_CALIB_CO2, NULL);
    hmiPtr->setText(_VPAddressStdCO2Text, "");
    hmiPtr->setText(_VPAddressPage102_infoCO2, "");
    hmiPtr->setTextColor(_SPAddressPage102_point1CO2, 0x03, _BlueColor);
    hmiPtr->setTextColor(_SPAddressPage102_point2CO2, 0x03, _BlueColor);
    hmiPtr->setTextColor(_SPAddressPage102_point3CO2, 0x03, _BlueColor);
    hmiPtr->setPage(_CalibCO2Page);
}

void HMI::_NutResetHeSoCalibNhiet_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = HMI_RESET_CALIB_NHIET;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
    hmiPtr->_hmiGetDataCallback(HMI_GET_CALIB_TEMP, NULL);
}
void HMI::_NutResetHeSoCalibCO2_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = HMI_RESET_CALIB_CO2;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
    hmiPtr->_hmiGetDataCallback(HMI_GET_CALIB_CO2, NULL);
}

void HMI::_NutSetChonDiemCalib1Temp_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_hmiGetDataCallback(eHMI_GET_CALIB_AND_SET_POINT1_TEMP, NULL);
    hmiPtr->_DiemChonCalib = ePOINT1;
}
void HMI::_NutSetChonDiemCalib2Temp_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_hmiGetDataCallback(eHMI_GET_CALIB_AND_SET_POINT2_TEMP, NULL);
    hmiPtr->_DiemChonCalib = ePOINT2;
}
void HMI::_NutSetChonDiemCalib3Temp_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_hmiGetDataCallback(eHMI_GET_CALIB_AND_SET_POINT3_TEMP, NULL);
    hmiPtr->_DiemChonCalib = ePOINT3;
}

void HMI::_NutSetChonDiemCalib1CO2_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_hmiGetDataCallback(eHMI_GET_CALIB_AND_SET_POINT1_CO2, NULL);
    hmiPtr->_DiemChonCalib = ePOINT1;
}
void HMI::_NutSetChonDiemCalib2CO2_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_hmiGetDataCallback(eHMI_GET_CALIB_AND_SET_POINT2_CO2, NULL);
    hmiPtr->_DiemChonCalib = ePOINT2;
}

void HMI::_NutSetChonDiemCalib3CO2_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_hmiGetDataCallback(eHMI_GET_CALIB_AND_SET_POINT3_CO2, NULL);
    hmiPtr->_DiemChonCalib = ePOINT3;
}

void HMI::vHienThiCacDiemCalib(TempCalibStruct_t xCalibTemp, CO2CalibStruct_t xCalibCO2, hmi_get_type_t event)
{

    if (eHMI_GET_CALIB_AND_SET_POINT1_TEMP <= event && event <= eHMI_GET_CALIB_AND_SET_POINT3_TEMP)
    {
        this->setText(_VPAddressPage78_setpointCalib1, String(xCalibTemp.point1.Setpoint, 1));
        this->setText(_VPAddressPage78_setpointCalib2, String(xCalibTemp.point2.Setpoint, 1));
        this->setText(_VPAddressPage78_setpointCalib3, String(xCalibTemp.point3.Setpoint, 1));
        this->setTextColor(_SPAddressPage78_point1Temp, 0x03, _BlueColor);
        this->setTextColor(_SPAddressPage78_point2Temp, 0x03, _BlueColor);
        this->setTextColor(_SPAddressPage78_point3Temp, 0x03, _BlueColor);
        this->setTextColor(_SPAddressPage78_point1Temp + (event - eHMI_GET_CALIB_AND_SET_POINT1_TEMP) * 0x10, 0x03, _RedColor);
    }
    else if (eHMI_GET_CALIB_AND_SET_POINT1_CO2 <= event && event <= eHMI_GET_CALIB_AND_SET_POINT3_CO2)
    {
        this->setText(_VPAddressPage102_setpointCalib1, String(xCalibCO2.point1.Setpoint, 1));
        this->setText(_VPAddressPage102_setpointCalib2, String(xCalibCO2.point2.Setpoint, 1));
        this->setText(_VPAddressPage102_setpointCalib3, String(xCalibCO2.point3.Setpoint, 1));
        this->setTextColor(_SPAddressPage102_point1CO2, 0x03, _BlueColor);
        this->setTextColor(_SPAddressPage102_point2CO2, 0x03, _BlueColor);
        this->setTextColor(_SPAddressPage102_point3CO2, 0x03, _BlueColor);
        this->setTextColor(_SPAddressPage102_point1CO2 + (event - eHMI_GET_CALIB_AND_SET_POINT1_CO2) * 0x10, 0x03, _RedColor);
    }
}

void HMI::_NutEditHeSoCalibPerimeter_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_INT;
    hmiPtr->_set_event.pageAfterEnter = hmiPtr->getPage();
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter;
    hmiPtr->_set_event.maxValue = 100;
    hmiPtr->_set_event.minValue = 0;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage78_Peremeter;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->setPage(_NumericKeypadPage);
}
void HMI::_NutEditHeSoCalibDoor_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_INT;
    hmiPtr->_set_event.pageAfterEnter = hmiPtr->getPage();
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterReturn;
    hmiPtr->_set_event.maxValue = 100;
    hmiPtr->_set_event.minValue = 0;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressPage78_Door;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->setPage(_NumericKeypadPage);
}

void HMI::_NutEditCalibTemp_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    if (hmiPtr->_DiemChonCalib == eNO_POINT)
    {
        hmiPtr->setText(_VPAddressStdTempText, "no Point");
        return;
    }
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterEnter = _CalibTempPage;
    hmiPtr->_set_event.pageAfterReturn = _CalibTempPage;
    hmiPtr->_set_event.maxValue = 900;
    hmiPtr->_set_event.minValue = 0;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressStdTempText;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    String Text = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    if (isdigit(Text.charAt(0)) == false)
    { // ký tự đầu không phải chuỗi thì xóa
        Text = "";
    }
    hmiPtr->_ChuoiBanPhimDangNhap = Text;
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->setPage(_NumericKeypadPage);
}
void HMI::_NutEditCalibCO2_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    if (hmiPtr->_DiemChonCalib == eNO_POINT)
    {
        hmiPtr->setText(_VPAddressStdCO2Text, "no Point");
        return;
    }
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.pageAfterEnter = _CalibCO2Page;
    hmiPtr->_set_event.pageAfterReturn = _CalibCO2Page;
    hmiPtr->_set_event.maxValue = 25;
    hmiPtr->_set_event.minValue = 0;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressStdCO2Text;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    String Text = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    if (isdigit(Text.charAt(0)) == false)
    { // ký tự đầu không phải chuỗi thì xóa
        Text = "";
    }
    hmiPtr->_ChuoiBanPhimDangNhap = Text;
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->setPage(_NumericKeypadPage);
}

void HMI::_NutEnterTrangCalibNhiet_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->setTextColor(_SPAddressPage78_point1Temp, 0x03, _BlueColor);
    hmiPtr->setTextColor(_SPAddressPage78_point2Temp, 0x03, _BlueColor);
    hmiPtr->setTextColor(_SPAddressPage78_point3Temp, 0x03, _BlueColor);

    String giaTriChuan = hmiPtr->getText(_VPAddressStdTempText, 6);
    if (giaTriChuan != "" && isdigit(giaTriChuan.charAt(0)))
    {
        hmiPtr->_set_event.type = HMI_SET_CALIB_NHIET;
        hmiPtr->_set_event.displayType = HMI_FLOAT;
        hmiPtr->setText(_VPAddressStdTempText, "");
        hmiPtr->_set_event.f_value = giaTriChuan.toFloat();
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
    }

    hmiPtr->_set_event.type = eHMI_SET_PERIMETER;
    hmiPtr->_set_event.displayType = HMI_INT;
    hmiPtr->_set_event.f_value = hmiPtr->getText(_VPAddressPage78_Peremeter, 6).toInt();
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

    hmiPtr->_set_event.type = eHMI_SET_DOOR;
    hmiPtr->_set_event.displayType = HMI_INT;
    hmiPtr->_set_event.f_value = hmiPtr->getText(_VPAddressPage78_Door, 6).toInt();
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

    hmiPtr->_hmiGetDataCallback(HMI_GET_CALIB_TEMP, NULL);
    hmiPtr->_DiemChonCalib = eNO_POINT;
}

void HMI::_NutEnterTrangCalibCO2_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->setTextColor(_SPAddressPage102_point1CO2, 0x03, _BlueColor);
    hmiPtr->setTextColor(_SPAddressPage102_point2CO2, 0x03, _BlueColor);
    hmiPtr->setTextColor(_SPAddressPage102_point3CO2, 0x03, _BlueColor);

    String giaTriChuan = hmiPtr->getText(_VPAddressStdCO2Text, 6);
    if (giaTriChuan != "" && isdigit(giaTriChuan.charAt(0)))
    {
        hmiPtr->_set_event.type = HMI_SET_CALIB_CO2;
        hmiPtr->_set_event.displayType = HMI_FLOAT;
        hmiPtr->setText(_VPAddressStdCO2Text, "");
        hmiPtr->_set_event.f_value = giaTriChuan.toFloat();
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        hmiPtr->_hmiGetDataCallback(HMI_GET_CALIB_CO2, NULL);
    }
    hmiPtr->_DiemChonCalib = eNO_POINT;
}

void HMI::_NutCaiDatThoiGianRTC_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_hmiGetDataCallback(HMI_GET_RTC, NULL); // Yêu cầu main gửi thời gian RTC lên HMI
    hmiPtr->setPage(_RTCPage);
}

void HMI::_NutEnterTrangCaiRTC_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    int Day = hmiPtr->getText(_VPAddressDayText, 2).toInt();
    int Month = hmiPtr->getText(_VPAddressMonthText, 2).toInt();
    int Year = hmiPtr->getText(_VPAddressYearText, 4).toInt();
    int Hour = hmiPtr->getText(_VPAddressHourText, 2).toInt();
    int Minute = hmiPtr->getText(_VPAddressMinuteText, 2).toInt();
    DateTime dt(Year, Month, Day, Hour, Minute, 0);
    hmiPtr->_set_event.type = HMI_SET_RTC;
    hmiPtr->_set_event.u32_value = dt.unixtime();
    Serial.printf("Unix time: %d\n", dt.unixtime());
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
    hmiPtr->setPage(_SettingsPage);
}

void HMI::_NutCaiCanhBaoNhietDo_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_hmiGetDataCallback(HMI_GET_ALARM, NULL); // Yêu cầu main gửi thông số Alarm đang lưu

    hmiPtr->setPage(_AlarmPage);
}

void HMI::HienThiNhietDoCanhBao(float NhietDuoi, float NhietTren)
{
    if (String(NhietDuoi).indexOf('-') >= 0)
    {
        this->setText(_VPAddressAlarmBelowTempText, String(NhietDuoi, 1));
    }
    else
    {
        this->setText(_VPAddressAlarmBelowTempText, "-" + String(NhietDuoi, 1));
    }
    this->setText(_VPAddressAlarmAboveTempText, "+" + String(NhietTren, 1));
}
void HMI::HienThiCO2CanhBao(float CO2Duoi, float CO2Tren)
{
    if (String(CO2Duoi).indexOf('-') >= 0)
    {
        this->setText(_VPAddressAlarmBelowCO2Text, String(CO2Duoi, 1));
    }
    else
    {
        this->setText(_VPAddressAlarmBelowCO2Text, "-" + String(CO2Duoi, 1));
    }
    this->setText(_VPAddressAlarmAboveCO2Text, "+" + String(CO2Tren, 1));
}

void HMI::_NutCaiCanhBaoNhietDoThap_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.minValue = -10;
    hmiPtr->_set_event.maxValue = -0.1;
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter = _AlarmPage;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressAlarmBelowTempText;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    // Serial.println("Below" + hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->setPage(_NumericKeypadPage);
}

void HMI::_NutCaiCanhBaoNhietDoCao_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.minValue = 0.1;
    hmiPtr->_set_event.maxValue = 10;
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter = _AlarmPage;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressAlarmAboveTempText;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    // Serial.println("Above" + hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->setPage(_NumericKeypadPage);
}

void HMI::_NutCaiCanhBaoCO2Thap_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.minValue = -10;
    hmiPtr->_set_event.maxValue = -0.1;
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter = _AlarmPage;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressAlarmBelowCO2Text;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    // Serial.println("Below" + hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->setPage(_NumericKeypadPage);
}

void HMI::_NutCaiCanhBaoCO2Cao_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.minValue = 0.1;
    hmiPtr->_set_event.maxValue = 10;
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter = _AlarmPage;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressAlarmAboveCO2Text;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(hmiPtr->_set_event.VPTextDisplayAfterEnter, 6);
    // Serial.println("Above" + hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->setPage(_NumericKeypadPage);
}

void HMI::_NutEnterTrangCaiCanhBaoNhietDo_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;

    float value = hmiPtr->getText(_VPAddressAlarmBelowTempText, 5).toFloat();
    hmiPtr->_set_event.type = HMI_SET_ALARM_TEMP_BELOW;
    hmiPtr->_set_event.f_value = value;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

    value = hmiPtr->getText(_VPAddressAlarmAboveTempText, 5).toFloat();
    hmiPtr->_set_event.type = HMI_SET_ALARM_TEMP_ABOVE;
    hmiPtr->_set_event.f_value = value;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

    value = hmiPtr->getText(_VPAddressAlarmAboveCO2Text, 5).toFloat();
    hmiPtr->_set_event.type = HMI_SET_ALARM_CO2_ABOVE;
    hmiPtr->_set_event.f_value = value;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

    value = hmiPtr->getText(_VPAddressAlarmBelowCO2Text, 5).toFloat();
    hmiPtr->_set_event.type = HMI_SET_ALARM_CO2_BELOW;
    hmiPtr->_set_event.f_value = value;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

    hmiPtr->setPage(_SettingsPage);
}

void HMI::HienThiTenChuongTrinhTrenHang(uint8_t row, uint8_t index, String name, uint8_t TotalSeg, const char *func)
{
    Serial.printf("%s\n", func);
    this->setText(_VPAddressNumProgramText1 + row * 2, String(index));
    this->setText(_VPAddressProgramNameText1 + row * 20, name);
    this->setTextColor(_SPAddressProgramNameText1 + row * 0x0010, 0x03, _BlueColor);
    this->setText(_VPAddressTotalNumOfSegmentsText1 + row * 2, String(TotalSeg));
    this->_TenChuongTrinh = "";
    this->_TenChuongTrinhTruocDo = "";
}

void HMI::XoaDuLieuHienThiTenChuongTrinhTrenHang(uint8_t row)
{
    this->setText(_VPAddressNumProgramText1 + row * 2, "");
    this->setText(_VPAddressProgramNameText1 + row * 20, "");
    this->setText(_VPAddressTotalNumOfSegmentsText1 + row * 2, "");
}

void HMI::_NutVaoChucNangProgram_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_hmiGetDataCallback(HMI_GET_PROGRAM_LIST, NULL);
    hmiPtr->_TenChuongTrinh = "";
    hmiPtr->_TenChuongTrinhTruocDo = "";
    hmiPtr->setText(_VPAddressCurrentProgramNameText, hmiPtr->_TenChuongTrinh);
    hmiPtr->setPage(_ProgramPage);
}

void HMI::_NutChonProgram_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;

    hmiPtr->_TenChuongTrinh = hmiPtr->getText(_VPAddressProgramNameText1 + lastBytes * 20, 20);
    if (hmiPtr->_TenChuongTrinhTruocDo != hmiPtr->_TenChuongTrinh)
    {
        hmiPtr->setTextColor(_SPAddressProgramNameText1 + lastBytes * 0x0010, 0x03, _RedColor);
        if (hmiPtr->_ChiMucChuongTrinhTruocDo != 0xFFFF && lastBytes != hmiPtr->_ChiMucChuongTrinhTruocDo)
        {
            hmiPtr->setTextColor(_SPAddressProgramNameText1 + hmiPtr->_ChiMucChuongTrinhTruocDo * 0x0010, 0x03, _BlueColor);
        }
        hmiPtr->_ChiMucChuongTrinhTruocDo = lastBytes;
        hmiPtr->_TenChuongTrinhTruocDo = hmiPtr->_TenChuongTrinh;
    }
    else
    {
        hmiPtr->_TenChuongTrinh = "";
        hmiPtr->_TenChuongTrinhTruocDo = "";
        hmiPtr->setTextColor(_SPAddressProgramNameText1 + hmiPtr->_ChiMucChuongTrinhTruocDo * 0x0010, 0x03, _BlueColor);
        hmiPtr->_ChiMucChuongTrinhTruocDo = 0xFFFF;
    }
    hmiPtr->setText(_VPAddressCurrentProgramNameText, hmiPtr->_TenChuongTrinh);
    Serial.println(hmiPtr->_TenChuongTrinh);
}

void HMI::_CacNutThaoTacTrongTrangProgram_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    Serial.printf("Key value: %d\n", lastBytes);
    switch (lastBytes)
    {
    case _KeyValueProgramUp:
        hmiPtr->_hmiGetDataCallback(HMI_GET_NEXT_PROGRAM_LIST, NULL);

        break;
    case _KeyValueProgramDown:
        hmiPtr->_hmiGetDataCallback(HMI_GET_BACK_PROGRAM_LIST, NULL);

        break;
    case _KeyValueProgramAdd:
        hmiPtr->_set_event.type = HMI_ADD_PROGRAM;
        hmiPtr->_set_event.displayType = HMI_TEXT;
        hmiPtr->_set_event.textLen = 15;
        hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressCurrentProgramNameText;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->_set_event.pageAfterReturn = _ProgramPage;
        hmiPtr->_set_event.pageAfterEnter = _SegmentAdjPage;
        hmiPtr->_ChuoiBanPhimDangNhap = "";
        hmiPtr->setText(_VPAddressKeyboardInputText, "");
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
        // hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        // hmiPtr->_hmiGetDataCallback(HMI_GET_PROGRAM_LIST, NULL);
        hmiPtr->DWIN::setPage(_KeyboardPage);
        hmiPtr->_TenChuongTrinh = "";
        break;
    case _KeyValueProgramDelete:
        hmiPtr->_set_event.type = HMI_DELETE_PROGRAM;
        if (hmiPtr->_TenChuongTrinh == "")
        {
            break;
        }
        hmiPtr->_set_event.text = hmiPtr->_TenChuongTrinh;
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        hmiPtr->_hmiGetDataCallback(HMI_GET_PROGRAM_LIST, NULL);
        hmiPtr->_TenChuongTrinh = "";
        break;
    case _KeyValueProgramEdit:
        if (hmiPtr->_TenChuongTrinh != "")
        {
            hmiPtr->_set_event.type = HMI_SELECT_PROGRAM;
            hmiPtr->_set_event.text = hmiPtr->_TenChuongTrinh;
            hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
            hmiPtr->_hmiGetDataCallback(HMI_GET_SEGMENT_LIST, NULL);
            hmiPtr->setPage(_SegmentAdjPage);
        }
        break;
    case _KeyValueProgramView:
        if (hmiPtr->_TenChuongTrinh != "")
        {
            hmiPtr->_set_event.type = HMI_SELECT_PROGRAM;
            hmiPtr->_set_event.text = hmiPtr->_TenChuongTrinh;
            hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
            hmiPtr->_hmiGetDataCallback(HMI_GET_SEGMENT_LIST, NULL);
            hmiPtr->setPage(_SegmentViewPage);
        }
        break;
    case _KeyValueProgramQuick:
        hmiPtr->_set_event.type = HMI_RUN_eQUICK_MODE;
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        hmiPtr->setPage(_HomePage);
        break;
    default:
        break;
    }
}

void HMI::_NutThayDoiTrangThaiChucNangHenGioTat_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = HMI_SET_DELAYOFF_ONOFF;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
}

void HMI::ThoiGianDoThi(time_t time)
{
    static uint8_t count = 0;
    if (count > 120)
    {
        count = 0;
    }
    if (count++ > 0)
    {
        return;
    }

    _bufferThoiGianDoThi.enqueue(&time);

    uint8_t u8BufferSize = _bufferThoiGianDoThi.getBufferSize();
    time_t arrTime[u8BufferSize] = {};
    _bufferThoiGianDoThi.peekAllBuffer(arrTime, u8BufferSize);

    for (uint8_t i = 0; i < u8BufferSize; i++)
    {
        char timeText[10] = {};
        if (arrTime[i] > 0)
        {
            sprintf(timeText, "%02u:%02u", hour(arrTime[i]), minute(arrTime[i]));
        }
        setText(_VPAddressGraphXValueText1 + 10 * i, timeText);
    }
}

void HMI::KhoiTaoDoThi(float value, DuLieuDoThi_t &curvePrameter, uint16_t VPyValueBase, uint16_t SPCurveMain, uint16_t SPCurveZoom, uint16_t PIDCurve)
{

    curvePrameter.minValue = (uint16_t)(value) * 10;
    if (curvePrameter.minValue < 0)
        curvePrameter.minValue = 0;
    curvePrameter.valueArr[0] = curvePrameter.minValue;
    for (uint8_t i = 1; i < 6; i++)
    {
        curvePrameter.valueArr[i] = curvePrameter.valueArr[0] + 10 * i;
    }
    curvePrameter.maxValue = curvePrameter.valueArr[5];
    curvePrameter.VDCentral = (curvePrameter.valueArr[5] + curvePrameter.valueArr[0]) / 2;
    curvePrameter.MulY = 240 * 256 / (curvePrameter.valueArr[5] - curvePrameter.valueArr[0]);
    for (uint8_t i = 0; i < 6; i++)
    {
        setText(VPyValueBase + i * 5, String(curvePrameter.valueArr[i] / 10.0f, 1));
    }
    setGraphVDCentral(SPCurveMain, curvePrameter.VDCentral);
    setGraphMulY(SPCurveMain, curvePrameter.MulY);
    setGraphVDCentral(SPCurveZoom, curvePrameter.VDCentral);
    setGraphMulY(SPCurveZoom, curvePrameter.MulY);
    setGraphVDCentral(PIDCurve, curvePrameter.VDCentral);
    setGraphMulY(PIDCurve, curvePrameter.MulY);
}
void HMI::ScaleDoThi(float value, DuLieuDoThi_t &curvePrameter, uint16_t VPyValueBase, uint16_t SPCurveMain, uint16_t SPCurveZoom, uint16_t PIDCurve)
{
    if (curvePrameter.maxValue == curvePrameter.minValue)
    {
        KhoiTaoDoThi(value, curvePrameter, VPyValueBase, SPCurveMain, SPCurveZoom, PIDCurve);
    }

    int16_t u16Value = (uint16_t)(value * 10);
    if (curvePrameter.minValue >= u16Value)
    {
        curvePrameter.valueArr[0] -= 10;
        if (curvePrameter.valueArr[0] < 0)
            curvePrameter.valueArr[0] = 0;
        curvePrameter.minValue = curvePrameter.valueArr[0];
        int16_t step = (curvePrameter.maxValue - curvePrameter.minValue) / 5;
        if (step < 0)
            step = 0;
        for (uint8_t i = 1; i < 6; i++)
        {
            curvePrameter.valueArr[i] = curvePrameter.valueArr[0] + step * i;
        }
        curvePrameter.maxValue = curvePrameter.valueArr[5];

        curvePrameter.MulY = 240 * 256 / (curvePrameter.valueArr[5] - curvePrameter.valueArr[0]);
        curvePrameter.VDCentral = (curvePrameter.valueArr[5] + curvePrameter.valueArr[0]) / 2;
        for (uint8_t i = 0; i < 6; i++)
        {
            setText(VPyValueBase + i * 5, String(curvePrameter.valueArr[i] / 10.0f, 1));
        }
        setGraphVDCentral(SPCurveMain, curvePrameter.VDCentral);
        setGraphMulY(SPCurveMain, curvePrameter.MulY);
        setGraphVDCentral(SPCurveZoom, curvePrameter.VDCentral);
        setGraphMulY(SPCurveZoom, curvePrameter.MulY);
        setGraphVDCentral(PIDCurve, curvePrameter.VDCentral);
        setGraphMulY(PIDCurve, curvePrameter.MulY);
    }
    else if (curvePrameter.maxValue <= u16Value)
    {
        curvePrameter.maxValue = (uint16_t)(value) * 10 + 10;
        curvePrameter.valueArr[5] = curvePrameter.maxValue;
        int16_t step = (curvePrameter.maxValue - curvePrameter.minValue) / 5;
        if (step < 0)
            step = 0;
        curvePrameter.valueArr[0] = curvePrameter.valueArr[5] - step * 5;
        curvePrameter.minValue = curvePrameter.valueArr[0];
        for (uint8_t i = 1; i < 6; i++)
        {
            curvePrameter.valueArr[i] = curvePrameter.valueArr[0] + step * i;
        }
        curvePrameter.MulY = 240 * 256 / (curvePrameter.valueArr[5] - curvePrameter.valueArr[0]);
        curvePrameter.VDCentral = (curvePrameter.valueArr[5] + curvePrameter.valueArr[0]) / 2;
        for (uint8_t i = 0; i < 6; i++)
        {
            setText(VPyValueBase + i * 5, String(curvePrameter.valueArr[i] / 10.0f, 1));
        }
        setGraphVDCentral(SPCurveMain, curvePrameter.VDCentral);
        setGraphMulY(SPCurveMain, curvePrameter.MulY);
        setGraphVDCentral(SPCurveZoom, curvePrameter.VDCentral);
        setGraphMulY(SPCurveZoom, curvePrameter.MulY);
        setGraphVDCentral(PIDCurve, curvePrameter.VDCentral);
        setGraphMulY(PIDCurve, curvePrameter.MulY);
    }
}

void HMI::VeDoThi(BaseProgram_t data, time_t time)
{

    ScaleDoThi(data.temperature, _DuLieuDoThiNhietDo, _VPAddressGraphYValueText1, _SPAddressSmallGraph1, _SPAddressLargeGraph, _SPAddressPage103PIDGraph);
    ScaleDoThi(data.CO2, _DuLieuDoThiCO2, _VPAddressGraphY_R_ValueText1, _SPAddressSmallGraphCO2, _SPAddressLargeGraphCO2, _SPAddressPage103PIDGraphCO2);
    ThoiGianDoThi(time);

    int16_t temp = (uint16_t)(data.temperature * 10);
    int16_t CO2 = (uint16_t)(data.CO2 * 10);
    uint16_t arrayy[] = {0x0310, 0x5AA5, 0x0200, 0x0001, temp, 0x0101, CO2};
    // Serial.printf("%u %u\t min % u, max % u, VD % u, Mul % u\tmin % u, max % u, VD % u, Mul % u\t\n", temp, CO2,
    //     _DuLieuDoThiNhietDo.minValue, _DuLieuDoThiNhietDo.maxValue, _DuLieuDoThiNhietDo.VDCentral, _DuLieuDoThiNhietDo.MulY,
    //     _DuLieuDoThiCO2.minValue, _DuLieuDoThiCO2.maxValue, _DuLieuDoThiCO2.VDCentral, _DuLieuDoThiCO2.MulY);
    //! sendIntArray(0x82, arrayy, 14);
}

void HMI::XoaDoThi(BaseProgram_t data)
{
    resetGraph(0);
    resetGraph(1);

    KhoiTaoDoThi(data.CO2, _DuLieuDoThiCO2, _VPAddressGraphY_R_ValueText1, _SPAddressSmallGraphCO2, _SPAddressLargeGraphCO2, _SPAddressPage103PIDGraphCO2);
    KhoiTaoDoThi(data.temperature, _DuLieuDoThiNhietDo, _VPAddressGraphYValueText1, _SPAddressSmallGraph1, _SPAddressLargeGraph, _SPAddressPage103PIDGraph);
}

void HMI::HienThiThongSoPID(uint16_t startVPText, PIDCalcu_t xPIDCalcu, PIDParam_t xPIDParam, float err)
{
    setText(startVPText, String(xPIDParam.Kp, xPIDParam.Kp < 1000 ? 3 : 0));
    setText(startVPText + 5, String(xPIDParam.Ki, xPIDParam.Ki < 1000 ? 3 : 0));
    setText(startVPText + 10, String(xPIDParam.Kd, xPIDParam.Kd < 1000 ? 3 : 0));
    setText(startVPText + 15, String(xPIDParam.Kw, xPIDParam.Kw < 1000 ? 3 : 0));
    setText(startVPText + 20, String(xPIDCalcu.Output, xPIDCalcu.Output < 1000 ? 3 : 0));
    setText(startVPText + 25, String(xPIDParam.WindupMax, xPIDParam.WindupMax < 1000 ? 3 : 0));
    setText(startVPText + 30, String(xPIDParam.WindupMin, xPIDParam.WindupMin < 1000 ? 3 : 0));
    setText(startVPText + 35, String(xPIDParam.OutMax, xPIDParam.OutMax < 1000 ? 3 : 0));
    setText(startVPText + 40, String(xPIDParam.OutMin, xPIDParam.OutMin < 1000 ? 3 : 0));

    setText(startVPText + 45, String(xPIDCalcu.PTerm, xPIDCalcu.PTerm < 1000 ? 3 : 0));
    setText(startVPText + 50, String(xPIDCalcu.ITerm, xPIDCalcu.ITerm < 1000 ? 3 : 0));
    setText(startVPText + 55, String(xPIDCalcu.DTerm, xPIDCalcu.DTerm < 1000 ? 3 : 0));
    setText(startVPText + 60, String(xPIDCalcu.feedBackWindup, xPIDCalcu.feedBackWindup < 1000 ? 3 : 0));
    setText(startVPText + 75, String(err, 3));
}
void HMI::HienThiThongSoTrangPID(PIDCalcu_t xPIDCalcuTemp, PIDParam_t xPIDParamTemp, float errTemp, float thoiGianBatCua, float thoiGianBatVanh, PIDCalcu_t xPIDCalcuCO2, PIDParam_t xPIDParamCO2, float errCO2)
{
    HienThiThongSoPID(_VPAddressPage103KpTemp, xPIDCalcuTemp, xPIDParamTemp, errTemp);
    HienThiThongSoPID(_VPAddressPage103KpCO2, xPIDCalcuCO2, xPIDParamCO2, errCO2);
    setText(_VPAddressPage103PremeterHeater, String(thoiGianBatVanh));
    setText(_VPAddressPage103DoorHeater, String(thoiGianBatCua));
}

void HMI::HienThiNhietDo(float GiaTri)
{
    setText(_VPAddressTemperatureText, String(GiaTri, 1));
}

void HMI::HienThiNhietDo(String text)
{
    setText(_VPAddressTemperatureText, text);
}

void HMI::HienThiCO2(float GiaTri)
{
    setText(_VPAddressCO2Text, String(GiaTri, 1));
}

void HMI::HienThiCO2(String text)
{
    setText(_VPAddressCO2Text, text);
}
void HMI::HienThiSetpointTemp(float GiaTri)
{
    setText(_VPAddressSetpointTempText, String(GiaTri, 1));
}
void HMI::HienThiSetpointCO2(float GiaTri)
{
    setText(_VPAddressSetpointCO2Text, String(GiaTri, 1));
}

void HMI::HienThiTocDoQuat(int GiaTri)
{
    setText(_VPAddressFanSpeedText, String(GiaTri));
}

void HMI::HienThiThoiGianChay(int ngay, int gio, int phut, int giay)
{
    char text[15];
    sprintf(text, "%02ud-%02u:%02u:%02u", ngay, gio, phut, giay);
    setText(_VPAddressDelayOffText, text);
}

void HMI::HienThiThoiGianChay(String text)
{
    setText(_VPAddressDelayOffText, text);
}

void HMI::HienThiThoiGianRTC(int ngay, int thang, int nam, int gio, int phut, int giay)
{
    setRTCSOFT(nam, thang, ngay, 1, gio, phut, giay);
}

void HMI::HienThiIconQuat(bool TrangThai)
{
    setVPByte(_VPAddressIconQuat, TrangThai);
}

void HMI::HienThiIconCua(bool TrangThai)
{
    setVPByte(_VPAddressIconCua, TrangThai);
}

void HMI::HienThiIconGiaNhiet(bool TrangThai)
{
    setVPByte(_VPAddressIconGiaNhiet, TrangThai);
}

void HMI::HienThiIconVanCO2(bool TrangThai)
{
    setVPByte(_VPAddressIconVanCO2, TrangThai);
}

void HMI::HienThiIconTrangThaiRun(bool TrangThai)
{
    setVPByte(_VPAddressIconRun, TrangThai);
}
void HMI::HienThiIconOnOffDelayOff(bool TrangThai)
{
    setVPByte(_VPAddressIconSwitchDelayOff, TrangThai);
}
void HMI::HienThiChuongTrinhDangChay(String text)
{
    setText(_VPAddressProgramNumText, text);
}

void HMI::HienThiSegmentDangChay(int SegmentDangChay, int TongSoSegment)
{
    setText(_VPAddressSegmentNumText, String(SegmentDangChay) + "/" + String(TongSoSegment));
}

void HMI::HienThiSegmentDangChay(String text)
{
    setText(_VPAddressSegmentNumText, text);
}

void HMI::_NutRun_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    // static bool state = 0;
    // if(state == false)
    // {
    //     hmiPtr->_set_event.type = HMI_SET_RUN_ONOFF;
    //     hmiPtr->HienThiIconTrangThaiRun(true);
    //     state = true;
    // } else {
    //     hmiPtr->_set_event.type = HMI_SET_RUN_OFF;
    //     hmiPtr->HienThiIconTrangThaiRun(false);
    //     state = false;
    // }
    hmiPtr->_set_event.type = HMI_SET_RUN_ONOFF;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
}

void HMI::Buzzer(int value)
{
    beepHMI(value);
}

void HMI::HienThiIconSegment(bool TrangThai)
{
    setVPByte(_VPAddressIconSegment, TrangThai);
}

void HMI::HienThiIconRemoveWater(bool TrangThai)
{
    setVPByte(_VPAddressIconTickRemoveWater, TrangThai);
}

void HMI::HienThiIconRemoveSample(bool TrangThai)
{
    setVPByte(_VPAddressIconTickRemoveSample, TrangThai);
}

void HMI::HienThiIconConfirm1(bool TrangThai)
{
    setVPByte(_VPAddressIconTickConfirm1, TrangThai);
}

void HMI::HienThiIconConfirm2(bool TrangThai)
{
    setVPByte(_VPAddressIconTickConfirm2, TrangThai);
}

void HMI::HienThiIconUSB(bool TrangThai)
{
    setVPByte(_VPAddressIconUSB, TrangThai);
}

void HMI::HienThiVongLapChuongTrinhConLai(int GiaTri, int Tong)
{
    setText(_VPAddressCurrentProgramLoopText, String(GiaTri) + "/" + String(Tong));
}

// truc them
void HMI::HienThiCheckAdminPasswordState(String text)
{
    setText(_VPAddressTextPasswordCheckState, text);
}

void HMI::HienThiVongLapChuongTrinhConLai(String text)
{
    setText(_VPAddressCurrentProgramLoopText, text);
}

void HMI::HienThiWarning(std::vector<String> warningVector, uint8_t TrangSauKhiReturn)
{
    _TrangSauKhiNhanReturnTrenWarning = TrangSauKhiReturn;
    constexpr uint32_t step = _VPAddressWarningText1 - _VPAddressWarningText0;
    uint8_t i = 0;
    int8_t i8Try = 5;
    _set_event.pageAfterReturn = 0;
    uint8_t checkPage = getPage();
    while (i8Try--)
    {
        _set_event.pageAfterReturn = getPage();
        if (checkPage == _set_event.pageAfterReturn)
        {
            break;
        }
        checkPage == _set_event.pageAfterReturn;
    }
    if (checkPage == _set_event.pageAfterReturn)
    {
        _set_event.pageAfterReturn = _HomePage;
    }

    for (; i < warningVector.size(); i++)
    {
        setText(_VPAddressWarningText0 + i * step, warningVector.at(i));
    }
    for (; i < 5; i++)
    {
        setText(_VPAddressWarningText0 + i * step, "");
    }
    setPage(_WarningPage);
}
void HMI::HienThiWarning(String text, uint8_t TrangSauKhiReturn)
{
    _TrangSauKhiNhanReturnTrenWarning = TrangSauKhiReturn;
    setText(_VPAddressWarningText0, text);
    setPage(_WarningPage);
}

void HMI::HienThiTenProgramDangEdit(String text)
{
    setText(_VPAddressCurrentProgramNameText, text);
}

void HMI::HienThiThanhLoading(uint8_t PhanTram)
{
    static uint8_t PhanTramHienThiTruocDo = 100;
    if (PhanTramHienThiTruocDo != PhanTram / 2)
    {
        PhanTramHienThiTruocDo = PhanTram / 2;
        setVPByte(_VPAddressIconLoading, PhanTramHienThiTruocDo);
    }
}

void HMI::HienThiPhanTramThanhLoading(uint8_t PhanTram)
{
    static uint8_t PhanTramHienThiTruocDo = 100;
    if (PhanTramHienThiTruocDo != PhanTram)
    {
        PhanTramHienThiTruocDo = PhanTram;
        setText(_VPAddressTextPhanTramThanhLoading, String(PhanTramHienThiTruocDo) + "%");
    }
}

void HMI::HienThiPhanTramThanhLoading(String text)
{
    setText(_VPAddressTextPhanTramThanhLoading, text);
}

void HMI::HienThiThongTinVersion(String text)
{
    setText(_VPAddressTextVersion, text);
}

void HMI::HienThiThongTinUpdate(String text)
{
    setText(_VPAddressTextCheckForUpdates, text);
    setPage(_CheckForUpdatePage);
}

void HMI::HienThiTrangThaiKetNoiWiFi(String text)
{
    setText(_VPAddressTextWiFiConnectState, text);
}

void HMI::HienThiSSIDWiFi(String text)
{
    setText(_VPAddressTextSSIDWifi, text);
}
void HMI::HienThiListSSIDWifi(std::vector<String> vectorSSID)
{
    for (uint16_t i = 0; i < 4; i++)
    {
        Serial.println("SSID: " + vectorSSID.at(i));
        setText(_VPAddressPage94SSID1 + 30 * i, vectorSSID.at(i));
    }
}

void HMI::HienThiPasswordWiFi(String text)
{
    setText(_VPAddressTextPASSWifi, text);
}

void HMI::_NutEnterTrongTrangProgram_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    if (hmiPtr->_TenChuongTrinh != "")
    {
        hmiPtr->_set_event.type = UNDEFINED;
        hmiPtr->_set_event.displayType = HMI_INT;
        hmiPtr->_set_event.maxValue = 255;
        hmiPtr->_set_event.minValue = 1;
        hmiPtr->_set_event.textLen = 3;
        hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressProgramLoopText;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressProgramLoopText;
        hmiPtr->_set_event.pageAfterReturn = _ProgramPage;
        hmiPtr->_set_event.pageAfterEnter = _ProgramPage;
        hmiPtr->_ChuoiBanPhimDangNhap = "1";
        hmiPtr->setText(_VPAddressProgramLoopText, "1");
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
        hmiPtr->setPage(_ProgramLoopPage);
    }
}

void HMI::_NutEnterChayProgramVoiChuKyDuocChon_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    {
        String strLoop = hmiPtr->getText(_VPAddressProgramLoopText, 3);
        if (strLoop == "Inf")
        {
            hmiPtr->_set_event.f_value = 0;
        }
        else
        {
            hmiPtr->_set_event.f_value = strLoop.toInt();
        }
    }
    hmiPtr->_set_event.type = HMI_RUN_ePROGRAM_MODE;
    hmiPtr->_set_event.text = hmiPtr->_TenChuongTrinh;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
}

void HMI::_NutInfTrongTrangChonChuKyChayProgram_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->setText(_VPAddressProgramLoopText, "Inf");
}

// void HMI::_NutCaiChuKyChayProgram_(int32_t lastBytes, void* args)
// {
//     HMI* hmiPtr = (HMI*)args;
//     hmiPtr->_set_event.type = UNDEFINED;
//     hmiPtr->_set_event.displayType = HMI_INT;
//     hmiPtr->_set_event.maxValue = 255;
//     hmiPtr->_set_event.minValue = 1;
//     hmiPtr->_set_event.textLen = 3;
//     hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressProgramLoopText;
//     hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
//     hmiPtr->_set_event.pageAfterReturn = _ProgramLoopPage;
//     hmiPtr->_set_event.pageAfterEnter = _ProgramLoopPage;
//     hmiPtr->_ChuoiBanPhimDangNhap = "1";
//     hmiPtr->setText(_VPAddressKeyboardInputText, "1");
//     hmiPtr->setText(_VPAddressKeyboardWarningText, "");
//     hmiPtr->setPage(_NumericKeypadPage);
// }

void HMI::_NutVaoChucNangTietTrung_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = HMI_RESET_STER;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
    hmiPtr->_DemTrangThaiNext = 0;
    hmiPtr->_GioTietTrung = 2;
    hmiPtr->_PhutTietTrung = 0;
    hmiPtr->_NhietDoTietTrung = 180;
    if (hmiPtr->_PhutTietTrung < 10)
    {
        hmiPtr->setText(_VPAddressTrangCaiThoiGianTietTrung, "0" + String(hmiPtr->_GioTietTrung) + ":" + "0" + String(hmiPtr->_PhutTietTrung));
    }
    else
    {
        hmiPtr->setText(_VPAddressTrangCaiThoiGianTietTrung, "0" + String(hmiPtr->_GioTietTrung) + ":" + String(hmiPtr->_PhutTietTrung));
    }
    hmiPtr->setText(_VPAddressTextNhietDoTietTrung, String(hmiPtr->_NhietDoTietTrung, 0));
    // hmiPtr->setText(_VPAddressTextNhietDoTietTrung, "180");
    hmiPtr->setVPByte(_VPAddressIconTickRemoveWater, false);
    hmiPtr->setVPByte(_VPAddressIconTickRemoveSample, false);
    hmiPtr->setVPByte(_VPAddressIconTickConfirm1, false);
    hmiPtr->setVPByte(_VPAddressIconTickConfirm2, false);
    hmiPtr->setPage(_SterilizationPage);
}

void HMI::_NutCaiThoiGianTietTrung_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    String strHour;
    String strMinute;
    if (hmiPtr->_GioTietTrung < 10)
    {
        strHour = "0" + String(hmiPtr->_GioTietTrung);
    }
    if (hmiPtr->_PhutTietTrung < 10)
    {
        strMinute = "0" + String(hmiPtr->_PhutTietTrung);
    }
    hmiPtr->setText(_VPAddressHourText, String(hmiPtr->_GioTietTrung));
    hmiPtr->setText(_VPAddressMinuteText, String(hmiPtr->_PhutTietTrung));
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->setPage(_TrangCaiThoiGianTietTrung);
}

void HMI::_NutCaiNhietDoTietTrung_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_FLOAT;
    hmiPtr->_set_event.maxValue = 180;
    hmiPtr->_set_event.minValue = 20;
    hmiPtr->_set_event.pageAfterReturn = hmiPtr->_set_event.pageAfterEnter = _SterilizationPage;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressTextNhietDoTietTrung;
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.textLen = 5;
    hmiPtr->_ChuoiBanPhimDangNhap = hmiPtr->getText(_VPAddressTextNhietDoTietTrung, 5);
    hmiPtr->setText(_VPAddressKeyboardInputText, hmiPtr->_ChuoiBanPhimDangNhap);
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->setPage(_NumericKeypadPage);
}

void HMI::_NutNextTrongCaiTietTrung_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    if (hmiPtr->_DemTrangThaiNext < 2)
    {
        hmiPtr->setVPByte(_VPAddressIconTickRemoveWater + hmiPtr->_DemTrangThaiNext, 1);
        hmiPtr->_DemTrangThaiNext++;
    }
    else
    {
        hmiPtr->_ChuoiBanPhimDangNhap = "";
        hmiPtr->_set_event.displayType = HMI_PASSWORD;
        if (hmiPtr->_DemTrangThaiNext == 2)
        {
            hmiPtr->_set_event.textLen = 6;
            hmiPtr->_set_event.type = HMI_SET_ICON1;
            hmiPtr->_set_event.pageAfterEnter = _SterilizationPage;
            hmiPtr->_set_event.pageAfterReturn = _SterilizationPage;
            hmiPtr->setPage(_KeyboardPage);
        }
        else if (hmiPtr->_DemTrangThaiNext == 3)
        {
            hmiPtr->_set_event.textLen = 6;
            hmiPtr->_set_event.type = HMI_SET_ICON2;
            hmiPtr->_set_event.pageAfterEnter = _SterilizationPage;
            hmiPtr->_set_event.pageAfterReturn = _SterilizationPage;
            hmiPtr->setPage(_KeyboardPage);
        }
        hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressKeyboardInputText;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->setText(_VPAddressKeyboardInputText, "Password ?");
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    }
}

void HMI::_NutEnterCaiTietTrung_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    if (hmiPtr->_DemTrangThaiNext == 4)
    {
        hmiPtr->_set_event.type = HMI_SET_STER_TEMP;
        hmiPtr->_set_event.f_value = hmiPtr->getText(_VPAddressTextNhietDoTietTrung, 5).toFloat();
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

        hmiPtr->_set_event.type = HMI_SET_STER_TIME;
        if (hmiPtr->_GioTietTrung >= 2 && hmiPtr->_PhutTietTrung > 0)
        {
            hmiPtr->_GioTietTrung = 2;
            hmiPtr->_PhutTietTrung = 0;
        }
        hmiPtr->_set_event.u32_value = hmiPtr->_GioTietTrung * 3600 + hmiPtr->_PhutTietTrung * 60;
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        hmiPtr->setPage(_HomePage);
    }
}

void HMI::_NutEnterCaiThoiGianTietTrung_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    String strHour, strMinute;
    hmiPtr->_GioTietTrung = hmiPtr->getText(_VPAddressHourText, 2).toInt();
    hmiPtr->_PhutTietTrung = hmiPtr->getText(_VPAddressMinuteText, 2).toInt();
    if (hmiPtr->_GioTietTrung >= 2 && hmiPtr->_PhutTietTrung >= 0)
    {
        hmiPtr->_GioTietTrung = 2;
        hmiPtr->_PhutTietTrung = 0;
    }
    if (hmiPtr->_GioTietTrung < 10)
    {
        strHour = "0" + String(hmiPtr->_GioTietTrung);
    }
    else
    {
        strHour = String(hmiPtr->_GioTietTrung);
    }
    if (hmiPtr->_PhutTietTrung < 10)
    {
        strMinute = "0" + String(hmiPtr->_PhutTietTrung);
    }
    else
    {
        strMinute = String(hmiPtr->_PhutTietTrung);
    }

    hmiPtr->setText(_VPAddressTrangCaiThoiGianTietTrung, strHour + ":" + strMinute);
    hmiPtr->setPage(_SterilizationPage);
}

void HMI::_NutTroVeTrongTrangWarning_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->setPage(hmiPtr->_TrangSauKhiNhanReturnTrenWarning);
}

void HMI::_NutXoaDoThi_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    switch (lastBytes)
    {
    case _KeyValueResetGraphTemp:
        hmiPtr->resetGraph(0);
        break;
    case _KeyValueResetGraphCO2:
        hmiPtr->resetGraph(1);
        break;
    default:
        break;
    }
}

void HMI::_NutDataRecord_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->HienThiThanhLoading(0);
    hmiPtr->HienThiPhanTramThanhLoading(0);
    hmiPtr->setPage(_DataRecordPage);
}

void HMI::_NutExportData_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = HMI_EXPORT_DATA;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
    hmiPtr->setPage(_DataRecordPage);
}

void HMI::_NutVaoChucNangUpdate_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_PASSWORD;
    hmiPtr->_set_event.pageAfterEnter = _UpdatePage;
    hmiPtr->_set_event.pageAfterReturn = _SettingsPage;
    hmiPtr->_set_event.textLen = 10;
    hmiPtr->_ChuoiBanPhimDangNhap = "";
    hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
    hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressKeyboardInputText;
    hmiPtr->setText(_VPAddressKeyboardInputText, "Password ?");
    hmiPtr->setText(_VPAddressKeyboardWarningText, "");
    hmiPtr->setPage(_KeyboardPage);
}

void HMI::_CacNutChonPhuongThucUpdate_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    switch (lastBytes)
    {
    case _KeyValueNutChonUpdateBangUSB:
        hmiPtr->_set_event.type = HMI_FIRMWARE_USB;
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        break;
    case _KeyValueNutChonUpdateBangFOTA:
        if (WiFi.status() == WL_CONNECTED)
        {
            // Gửi yêu cầu Cập nhật firmware OTA
            hmiPtr->_set_event.type = HMI_FIRMWARE_FOTA;
            hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
            break;
        }
        hmiPtr->setPage(_FOTAPage);
        break;
    default:
        break;
    }
}

void HMI::_CacNutTrangFOTA_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    switch (lastBytes)
    {
    case _KeyValueNutSSIDFOTA:
        hmiPtr->_set_event.type = UNDEFINED;
        hmiPtr->_set_event.displayType = HMI_TEXT;
        hmiPtr->_set_event.textLen = 30;
        hmiPtr->_set_event.pageAfterReturn = _FOTAPage;
        hmiPtr->_set_event.pageAfterEnter = _FOTAPage;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressTextSSIDWifi;
        hmiPtr->_ChuoiBanPhimDangNhap = "";
        hmiPtr->setText(_VPAddressKeyboardInputText, "");
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
        hmiPtr->setPage(_KeyboardPage);
        break;
    case _KeyValueNutPASSFOTA:
        hmiPtr->_set_event.type = UNDEFINED;
        hmiPtr->_set_event.displayType = HMI_TEXT;
        hmiPtr->_set_event.textLen = 30;
        hmiPtr->_set_event.pageAfterReturn = _FOTAPage;
        hmiPtr->_set_event.pageAfterEnter = _FOTAPage;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressTextPASSWifi;
        hmiPtr->_ChuoiBanPhimDangNhap = "";
        hmiPtr->setText(_VPAddressKeyboardInputText, "");
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
        hmiPtr->setPage(_KeyboardPage);
        break;
    case _KeyValueNutUpdateFOTA:
        // Lấy SSID từ HMI
        hmiPtr->_set_event.type = HMI_SET_SSID;
        hmiPtr->_set_event.text = hmiPtr->getText(_VPAddressTextSSIDWifi, 30);
        if (hmiPtr->_set_event.text.compareTo("") == 0)
        {
            break;
        }
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

        // Lấy PASS từ HMI
        hmiPtr->_set_event.type = HMI_SET_PASSWORD;
        hmiPtr->_set_event.text = hmiPtr->getText(_VPAddressTextPASSWifi, 15);
        if (hmiPtr->_set_event.text.compareTo("") == 0)
        {
            break;
        }
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

        // Connect wifi
        hmiPtr->_set_event.type = HMI_CONNECT_OR_DISCONNECT_WIFI;
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

        // Gửi yêu cầu Cập nhật firmware OTA
        hmiPtr->_set_event.type = HMI_FIRMWARE_FOTA;
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

        break;
    default:
        break;
    }
}

void HMI::_ThanhCuonDoThi_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = HMI_SET_SCROLLCHART;
    hmiPtr->_set_event.f_value = (float)lastBytes;
    hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
}

void HMI::_NutVaoChucNangWiFi_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_TEXT;

    hmiPtr->setPage(_WiFiPage);
}

void HMI::_CacNutTrangWiFi_(int32_t lastBytes, void *arg)
{
    HMI *hmiPtr = (HMI *)arg;
    switch (lastBytes)
    {
    case _KeyValueNutWiFiID:
        hmiPtr->_set_event.type = UNDEFINED;
        hmiPtr->_set_event.displayType = HMI_TEXT;
        hmiPtr->_set_event.textLen = 30;
        hmiPtr->_set_event.pageAfterReturn = _WiFiPage;
        hmiPtr->_set_event.pageAfterEnter = _WiFiPage;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressTextSSIDWifi;
        hmiPtr->_ChuoiBanPhimDangNhap = "";
        hmiPtr->setText(_VPAddressKeyboardInputText, "");
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
        hmiPtr->setPage(_KeyboardPage);
        break;
    case _KeyValueNutPassWifi:
        hmiPtr->_set_event.type = UNDEFINED;
        // hmiPtr->_set_event.displayType = HMI_EXTERNAL_PASSWORD;
        hmiPtr->_set_event.displayType = HMI_TEXT;
        hmiPtr->_set_event.textLen = 30;
        hmiPtr->_set_event.pageAfterReturn = _WiFiPage;
        hmiPtr->_set_event.pageAfterEnter = _WiFiPage;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressTextPASSWifi;
        hmiPtr->_ChuoiBanPhimDangNhap = "";
        hmiPtr->setText(_VPAddressKeyboardInputText, "");
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
        hmiPtr->setPage(_KeyboardPage);
        break;

        break;
    case _KeyValueNutKetNoiWiFi:
        // Lấy SSID từ HMI
        hmiPtr->_set_event.type = HMI_SET_SSID;
        hmiPtr->_set_event.text = hmiPtr->getText(_VPAddressTextSSIDWifi, 30);
        if (hmiPtr->_set_event.text.compareTo("") == 0)
        {
            break;
        }
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

        // Lấy PASS từ HMI
        hmiPtr->_set_event.type = HMI_SET_PASSWORD;
        hmiPtr->_set_event.text = hmiPtr->getText(_VPAddressTextPASSWifi, 15);
        if (hmiPtr->_set_event.text.compareTo("") == 0)
        {
            break;
        }
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);

        // Connect wifi
        hmiPtr->_set_event.type = HMI_CONNECT_OR_DISCONNECT_WIFI;
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        break;
    case _KeyValueNutScanWifi:
        // hmiPtr->_set_event.type = HMI_GET_SCAN_SSID_WIFI;
        hmiPtr->_hmiGetDataCallback(HMI_GET_SCAN_SSID_WIFI, NULL);
        break;
    case _KeyValueNutChonSSID1:
    case _KeyValueNutChonSSID2:
    case _KeyValueNutChonSSID3:
    case _KeyValueNutChonSSID4:
    {
        String string = hmiPtr->getText(_VPAddressPage94SSID1 + (lastBytes - _KeyValueNutChonSSID1) * 30, 30);
        if (!string.isEmpty())
            hmiPtr->setText(_VPAddressTextSSIDWifi, string);
    }
    break;
    default:
        break;
    }
}

void HMI::_NutVaoChucNangThayDoiAdminPassword_(int32_t lastBytes, void *args) // truc them
{
    HMI *hmiPtr = (HMI *)args;
    hmiPtr->_set_event.type = UNDEFINED;
    hmiPtr->_set_event.displayType = HMI_TEXT;
    hmiPtr->_set_event.pageAfterEnter = _SettingsPage;
    hmiPtr->_set_event.pageAfterReturn = _SettingsPage;
    hmiPtr->setPage(_AdminPasswordPage);
}
void HMI::_CacNutTrangThayDoiAdminPassword_(int32_t lastBytes, void *args)
{
    HMI *hmiPtr = (HMI *)args;
    switch (lastBytes)
    {
    case _KeyValueNutCaiCurrentAdminPassword:
        hmiPtr->_set_event.type = UNDEFINED;
        hmiPtr->_set_event.displayType = HMI_TEXT;
        hmiPtr->_set_event.textLen = 10;
        hmiPtr->_set_event.pageAfterReturn = _AdminPasswordPage;
        hmiPtr->_set_event.pageAfterEnter = _AdminPasswordPage;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressTextCurrentAdminPassword;
        hmiPtr->_ChuoiBanPhimDangNhap = "";
        hmiPtr->setText(_VPAddressKeyboardInputText, "");
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
        hmiPtr->setPage(_KeyboardPage);
        break;
        break;
    case _KeyValueNutCaiNewAdminPassword:
        hmiPtr->_set_event.type = UNDEFINED;
        hmiPtr->_set_event.displayType = HMI_TEXT;
        hmiPtr->_set_event.textLen = 10;
        hmiPtr->_set_event.pageAfterReturn = _AdminPasswordPage;
        hmiPtr->_set_event.pageAfterEnter = _AdminPasswordPage;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressTextNewAdminPassword;
        hmiPtr->_ChuoiBanPhimDangNhap = "";
        hmiPtr->setText(_VPAddressKeyboardInputText, "");
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
        hmiPtr->setPage(_KeyboardPage);
        break;
    case _KeyValueNutcaiConfirmNewAdminPassword:
        hmiPtr->_set_event.type = UNDEFINED;
        hmiPtr->_set_event.displayType = HMI_TEXT;
        hmiPtr->_set_event.textLen = 10;
        hmiPtr->_set_event.pageAfterReturn = _AdminPasswordPage;
        hmiPtr->_set_event.pageAfterEnter = _AdminPasswordPage;
        hmiPtr->_set_event.VPTextDisplayWhenInput = _VPAddressKeyboardInputText;
        hmiPtr->_set_event.VPTextDisplayAfterEnter = _VPAddressTextConfirmNewAdminPassword;
        hmiPtr->_ChuoiBanPhimDangNhap = "";
        hmiPtr->setText(_VPAddressKeyboardInputText, "");
        hmiPtr->setText(_VPAddressKeyboardWarningText, "");
        hmiPtr->setPage(_KeyboardPage);
        break;

    case _KeyValueNutChangeAdminPassword:
        // Lấy old admin pass từ HMI
        hmiPtr->HienThiCheckAdminPasswordState("");
        hmiPtr->_set_event.text = hmiPtr->getText(_VPAddressTextCurrentAdminPassword, 10);
        if (hmiPtr->_set_event.text.compareTo("") == 0)
        {
            hmiPtr->HienThiCheckAdminPasswordState("Missing current password");
            break;
        }
        if (hmiPtr->SoSanhPassWord(hmiPtr->_set_event.text) == 0)
        {
            hmiPtr->HienThiCheckAdminPasswordState("Incorrect password");
            break;
        }
        else
        {
            hmiPtr->HienThiCheckAdminPasswordState("Success");
        }
        // Lấy new admin pass từ HMI
        hmiPtr->_set_event.text = hmiPtr->getText(_VPAddressTextNewAdminPassword, 10);
        if (hmiPtr->_set_event.text.compareTo("") == 0)
        {
            hmiPtr->HienThiCheckAdminPasswordState("Missing new password");
            break;
        }
        hmiPtr->_set_event.text = hmiPtr->getText(_VPAddressTextConfirmNewAdminPassword, 10);
        if (hmiPtr->_set_event.text.compareTo("") == 0)
        {
            hmiPtr->HienThiCheckAdminPasswordState("Please confirm your new password");
            break;
        }
        if (hmiPtr->getText(_VPAddressTextNewAdminPassword, 10).compareTo(hmiPtr->getText(_VPAddressTextConfirmNewAdminPassword, 10)) != 0)
        {
            hmiPtr->HienThiCheckAdminPasswordState("Incorrect confirmed password");
            break;
        }
        // Change pass
        hmiPtr->ThayDoiUserAdminPassword(hmiPtr->_set_event.text);
        hmiPtr->_set_event.type = HMI_CHANGE_ADMIN_PASSWORD;
        hmiPtr->_hmiSetDataCallback(hmiPtr->_set_event);
        break;
    default:
        break;

        break;
    }
}
bool HMI::SoSanhPassWord(String EnteredPassword)
{
    if (EnteredPassword.compareTo(this->_ChuoiPassword) == 0 || EnteredPassword.indexOf(userFACTORY_PASSWORD) >= 0)
    {
        return 1;
    }
    return 0;
}
void HMI::ThayDoiUserAdminPassword(String EnteredPassword)
{
    this->_ChuoiPassword = EnteredPassword;
}