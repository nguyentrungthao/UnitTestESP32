#ifndef _HMI_H_
#define _HMI_H_

/**
 *@file HMI.h
 * @author HMI của master
 * @brief
 * @version 0.1
 * @date 2025-03-27
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "DWIN_custom.h"
#include "freertos/queue.h"
#include <vector>
#include <TimeLib.h>
#include "userdef.h"
#include "HMIparam.h"
#include <Arduino.h>
#include <ringBuffer.h>
#include <07_Heater.h>
#include <09_concentration.h>

typedef enum
{
    UNDEFINED,
    HMI_SET_RUN_ONOFF,
    HMI_SET_FAN,
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
    HMI_CONNECT_OR_DISCONNECT_WIFI, // truc them
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
typedef enum
{
    HMI_GET_SEGMENT_LIST,
    HMI_GET_NEXT_SEGMENT_LIST,
    HMI_GET_BACK_SEGMENT_LIST,
    HMI_REFRESH_SEGMENT_LIST,
    HMI_GET_SEGMENT_DELAYOFF,
    HMI_GET_RTC,
    HMI_GET_ALARM,
    HMI_GET_PROGRAM_LIST,
    HMI_GET_NEXT_PROGRAM_LIST,
    HMI_GET_BACK_PROGRAM_LIST,
    HMI_GET_DELAYOFF,
    HMI_GET_CALIB_TEMP,
    HMI_GET_CALIB_CO2,
    HMI_CHECK_LIST,
    HMI_GET_SCAN_SSID_WIFI,

    eHMI_EVENT_ICON_NHIET,
    eHMI_EVENT_ICON_CO2,
    eHMI_EVENT_ICON_CUA,
    eHMI_EVENT_ICON_FAN,

    eHMI_EVENT_HIEN_THI_GIA_TRI_CAM_BIEN,
    eHMI_EVENT_HIEN_THI_PID,
    eHMI_EVENT_HIEN_THI_THOI_GIAN,
    eHMI_EVENT_ICON_USB,
    eHMI_EVENT_ICON_WIFI,
    eHMI_EVENT_VE_DO_THI,
    eHMI_EVENT_WARNING,
    eHMI_EVENT_REFRESH,

    eHMI_GET_CALIB_AND_SET_POINT1_TEMP,
    eHMI_GET_CALIB_AND_SET_POINT2_TEMP,
    eHMI_GET_CALIB_AND_SET_POINT3_TEMP,

    eHMI_GET_CALIB_AND_SET_POINT1_CO2,
    eHMI_GET_CALIB_AND_SET_POINT2_CO2,
    eHMI_GET_CALIB_AND_SET_POINT3_CO2,

    eHMI_EVENT_TIMEROUT_OFF,

    HMI_GET_MAX_ENUM
} hmi_get_type_t;

typedef enum
{
    HMI_FLOAT,
    HMI_INT,
    HMI_DATE,
    HMI_TEXT,
    HMI_PASSWORD,
    HMI_EXTERNAL_PASSWORD, // truc them
} hmi_data_type_t;

typedef struct
{
    hmi_set_type_t type;
    union
    {
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

struct DuLieuDoThi_t
{
    int16_t maxValue = 0;
    int16_t minValue = 0;
    int16_t MulY;
    int16_t VDCentral;
    int16_t valueArr[6];
    time_t timeArr[7];
};

typedef std::function<void(const hmi_set_event_t &event)> hmiSetData_t;
typedef std::function<bool(hmi_get_type_t get, void *args)> hmiGetData_t;
// typedef std::function<void(hmi_get_type_t get, int start, int length)> hmiGetDataList_t;

class HMI : public DWIN
{
public:
    HMI(HardwareSerial &port, uint8_t receivePin, uint8_t transmitPin, long baud = DWIN_DEFAULT_BAUD_RATE);
    void KhoiTao(void);
    void DangKyHamSetCallback(hmiSetData_t function);
    void DangKyHamGetCallback(hmiGetData_t function);

    void HienThiDuLieuSegmentTrenHang(uint8_t row, uint8_t index, float setpoint, float setpointCO2, int delayOffDay, int delayOffHour, int delayOffMinute,
                                      int fanSpeed, float tempMin, float tempMax, float CO2Min, float CO2Max);
    void XoaDuLieuHienThiSegmentTrenHang(uint8_t row);

    void HienThiNgay(int ngay);
    void HienThiThang(int thang);
    void HienThiNam(int nam);
    void HienThiGio(int gio);
    void HienThiPhut(int phut);

    void HienThiNhietDoCanhBao(float NhietDuoi, float NhietTren);
    void HienThiCO2CanhBao(float CO2Duoi, float CO2Tren);

    void HienThiTenChuongTrinhTrenHang(uint8_t row, uint8_t index, String name, uint8_t TotalSeg, const char *func = "");
    void XoaDuLieuHienThiTenChuongTrinhTrenHang(uint8_t row);

    void VeDoThi(BaseProgram_t data, time_t time);
    void KhoiTaoDoThi(float value, DuLieuDoThi_t &curvePrameter, uint16_t VPyValueBase, uint16_t SPCurveMain, uint16_t SPCurveZoom, uint16_t PIDCurve);
    void ScaleDoThi(float value, DuLieuDoThi_t &curvePrameter, uint16_t VPyValueBase, uint16_t SPCurveMain, uint16_t SPCurveZoom, uint16_t PIDCurve);
    void ThoiGianDoThi(time_t time);
    void XoaDoThi(BaseProgram_t data);

    void HienThiThongSoPID(uint16_t startVPText, PIDCalcu_t xPIDCalcu, PIDParam_t xPIDParam, float err);
    void HienThiThongSoTrangPID(PIDCalcu_t xPIDCalcuTemp, PIDParam_t xPIDParamTemp, float errTemp, float thoiGianBatCua, float thoiGianBatVanh, PIDCalcu_t xPIDCalcuCO2, PIDParam_t xPIDParamCO2, float errCO2);
    void HienThiNhietDo(float GiaTri);
    void HienThiNhietDo(String text);
    void HienThiCO2(float GiaTri);
    void HienThiCO2(String text);
    void HienThiSetpointTemp(float GiaTri);
    void HienThiSetpointCO2(float GiaTri);
    void HienThiTocDoQuat(int GiaTri);
    void HienThiThoiGianChay(int ngay, int gio, int phut, int giay);
    void HienThiThoiGianChay(String text);
    void HienThiThoiGianRTC(int ngay, int thang, int nam, int gio, int phut, int giay);
    void HienThiIconQuat(bool TrangThai);
    void HienThiIconCua(bool TrangThai);
    void HienThiIconGiaNhiet(bool TrangThai);
    void HienThiIconVanCO2(bool TrangThai);
    void HienThiIconTrangThaiRun(bool TrangThai);
    void HienThiIconOnOffDelayOff(bool TrangThai);
    void HienThiChuongTrinhDangChay(String text);
    void HienThiTenProgramDangEdit(String text);
    void HienThiSegmentDangChay(int SegmentDangChay, int TongSoSegment);
    void HienThiSegmentDangChay(String text);
    void HienThiIconSegment(bool TrangThai);
    void HienThiVongLapChuongTrinhConLai(int GiaTri, int Tong);
    void HienThiVongLapChuongTrinhConLai(String text);
    void HienThiWarning(std::vector<String> warningVector, uint8_t TrangSauKhiReturn);
    void HienThiWarning(String text, uint8_t TrangSauKhiReturn);
    void HienThiIconRemoveWater(bool TrangThai);
    void HienThiIconRemoveSample(bool TrangThai);
    void HienThiIconConfirm1(bool TrangThai);
    void HienThiIconConfirm2(bool TrangThai);
    void Buzzer(int value);
    void HienThiThanhLoading(uint8_t PhanTram);
    void HienThiPhanTramThanhLoading(uint8_t PhanTram);
    void HienThiPhanTramThanhLoading(String text);
    void HienThiThongTinVersion(String text);
    void HienThiThongTinUpdate(String text);
    void HienThiTrangThaiKetNoiWiFi(String text); // truc them
    void HienThiSSIDWiFi(String text);
    void HienThiPasswordWiFi(String text);
    void HienThiListSSIDWifi(std::vector<String> vectorSSID);
    void HienThiIconUSB(bool TrangThai);
    bool SoSanhPassWord(String EnteredPassword);
    void ThayDoiUserAdminPassword(String EnteredPassword);
    void HienThiCheckAdminPasswordState(String text);
    void vHienThiCacDiemCalib(TempCalibStruct_t xCalibTemp, CO2CalibStruct_t xCalibCO2, hmi_get_type_t event);

    CalibPosition_t xGetCalibPoint()
    {
        return _DiemChonCalib;
    }

protected:
    HardwareSerial *_hmiSerial;
    hmiSetData_t _hmiSetDataCallback;
    hmiGetData_t _hmiGetDataCallback;
    SemaphoreHandle_t _lock;

    String _ChuoiBanPhimDangNhap;
    bool _CapslockEnable;
    hmi_set_event_t _set_event;

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

    RingBuffer _bufferThoiGianDoThi = RingBuffer(7, sizeof(time_t));

    void _createHmiListenTask(void *args);
    static void _hmiListenTask(void *args);
    static void _hmiUartEvent(void);

    static void _NutVaoChucNangChonCalib_(int32_t lastBytes, void *args);
    static void _NutVaoChucNangCalibNhiet_(int32_t lastBytes, void *args);
    static void _NutVaoChucNangCalibCO2_(int32_t lastBytes, void *args);
    static void _NutEditCalibTemp_(int32_t lastBytes, void *args);
    static void _NutEditCalibCO2_(int32_t lastBytes, void *args);
    static void _NutEnterTrangCalibNhiet_(int32_t lastBytes, void *args);
    static void _NutEnterTrangCalibCO2_(int32_t lastBytes, void *args);
    static void _NutResetHeSoCalibNhiet_(int32_t lastBytes, void *args);
    static void _NutResetHeSoCalibCO2_(int32_t lastBytes, void *args);
    static void _NutEditHeSoCalibPerimeter_(int32_t lastBytes, void *args);
    static void _NutEditHeSoCalibDoor_(int32_t lastBytes, void *args);
    static void _NutSetChonDiemCalib1Temp_(int32_t lastBytes, void *args);
    static void _NutSetChonDiemCalib2Temp_(int32_t lastBytes, void *args);
    static void _NutSetChonDiemCalib3Temp_(int32_t lastBytes, void *args);
    static void _NutSetChonDiemCalib1CO2_(int32_t lastBytes, void *args);
    static void _NutSetChonDiemCalib2CO2_(int32_t lastBytes, void *args);
    static void _NutSetChonDiemCalib3CO2_(int32_t lastBytes, void *args);

    static void _NutEnterPageService_(int32_t lastBytes, void *args);
    static void _NutEnterPageDiagnostic_(int32_t lastBytes, void *args);

    static void _NutThucDay_(int32_t lastBytes, void *args);
    static void _NutEnterPagePID_(int32_t lastBytes, void *args);
    static void _NutExitPID_(int32_t lastBytes, void *args);
    static void _NutSetKpTempPID_(int32_t lastBytes, void *args);
    static void _NutSetKiTempPID_(int32_t lastBytes, void *args);
    static void _NutSetKdTempPID_(int32_t lastBytes, void *args);
    static void _NutSetKwTempPID_(int32_t lastBytes, void *args);
    static void _NutSetImaxTempPID_(int32_t lastBytes, void *args);
    static void _NutSetIminTempPID_(int32_t lastBytes, void *args);
    static void _NutSetOutmaxTempPID_(int32_t lastBytes, void *args);
    static void _NutSetOutminTempPID_(int32_t lastBytes, void *args);
    static void _NutSetKpCO2PID_(int32_t lastBytes, void *args);
    static void _NutSetKiCO2PID_(int32_t lastBytes, void *args);
    static void _NutSetKdCO2PID_(int32_t lastBytes, void *args);
    static void _NutSetKwCO2PID_(int32_t lastBytes, void *args);
    static void _NutSetImaxCO2PID_(int32_t lastBytes, void *args);
    static void _NutSetIminCO2PID_(int32_t lastBytes, void *args);
    static void _NutSetOutmaxCO2PID_(int32_t lastBytes, void *args);
    static void _NutSetOutminCO2PID_(int32_t lastBytes, void *args);
    static void _NutSetPerimeterPID_(int32_t lastBytes, void *args);
    static void _NutSetDoorPID_(int32_t lastBytes, void *args);

    static void _NutCaiDatThoiGianRTC_(int32_t lastBytes, void *args);
    static void _NutEnterTrangCaiRTC_(int32_t lastBytes, void *args);

    static void _NutCaiCanhBaoNhietDo_(int32_t lastBytes, void *args);
    static void _NutEnterTrangCaiCanhBaoNhietDo_(int32_t lastBytes, void *args);
    static void _NutCaiCanhBaoNhietDoThap_(int32_t lastBytes, void *args);
    static void _NutCaiCanhBaoNhietDoCao_(int32_t lastBytes, void *args);
    static void _NutCaiCanhBaoCO2Thap_(int32_t lastBytes, void *args);
    static void _NutCaiCanhBaoCO2Cao_(int32_t lastBytes, void *args);

    static void _NutVaoChucNangProgram_(int32_t lastBytes, void *args);
    static void _NutChonProgram_(int32_t lastBytes, void *args);
    static void _CacNutThaoTacTrongTrangProgram_(int32_t lastBytes, void *args);
    static void _NutChonSegment_(int32_t lastBytes, void *args);

    static void _NutCaiNhietDoSetpoint_(int32_t lastBytes, void *args);
    static void _NutCaiCO2Setpoint_(int32_t lastBytes, void *args);
    static void _NutCaiTocDoQuat_(int32_t lastBytes, void *args);
    static void _NutCaiThoiGianTatMay_(int32_t lastBytes, void *args);
    static void _NutEnterCaiThoiGianTatMay_(int32_t lastBytes, void *args);

    static void _XuLyBanPhim_(int32_t lastBytes, void *args);

    static void _NutEditSetpointTrangSegment_(int32_t lastBytes, void *args);
    static void _NutEditSetpointCO2TrangSegment_(int32_t lastBytes, void *args);

    static void _NutEditTocDoQuatTrangSegment_(int32_t lastBytes, void *args);
    static void _NutEditTempMinTrangSegment_(int32_t lastBytes, void *args);
    static void _NutEditTempMaxTrangSegment_(int32_t lastBytes, void *args);
    static void _NutEditCO2MinTrangSegment_(int32_t lastBytes, void *args);
    static void _NutEditCO2MaxTrangSegment_(int32_t lastBytes, void *args);

    static void _NutEditThoiGianTatTrangSegment_(int32_t lastBytes, void *args);
    static void _NutEnterTrongCaiDatThoiGianTatCuaSegment_(int32_t lastBytes, void *args);
    static void _CacNutThaoTacTrongTrangEditSegment_(int32_t lastBytes, void *args);

    static void _NutVaoTrangNhapNgay_(int32_t lastBytes, void *args);
    static void _NutVaoTrangNhapThang_(int32_t lastBytes, void *args);
    static void _NutVaoTrangNhapNam_(int32_t lastBytes, void *args);
    static void _NutVaoTrangNhapGio_(int32_t lastBytes, void *args);
    static void _NutVaoTrangNhapPhut_(int32_t lastBytes, void *args);

    static void _NutRun_(int32_t lastBytes, void *args);
    static void _NutThayDoiTrangThaiChucNangHenGioTat_(int32_t lastBytes, void *args);

    static void _NutEnterTrongTrangProgram_(int32_t lastBytes, void *args);
    static void _NutEnterChayProgramVoiChuKyDuocChon_(int32_t lastBytes, void *args);
    static void _NutInfTrongTrangChonChuKyChayProgram_(int32_t lastBytes, void *args);
    static void _NutCaiChuKyChayProgram_(int32_t lastBytes, void *args);

    static void _NutVaoChucNangTietTrung_(int32_t lastBytes, void *args);
    static void _NutCaiThoiGianTietTrung_(int32_t lastBytes, void *args);
    static void _NutCaiNhietDoTietTrung_(int32_t lastBytes, void *args);
    static void _NutNextTrongCaiTietTrung_(int32_t lastBytes, void *args);
    static void _NutEnterCaiTietTrung_(int32_t lastBytes, void *args);
    static void _NutEnterCaiThoiGianTietTrung_(int32_t lastBytes, void *args);
    static void _NutTroVeTrongTrangWarning_(int32_t lastBytes, void *args);
    static void _NutXoaDoThi_(int32_t lastBytes, void *args);

    static void _NutDataRecord_(int32_t lastBytes, void *args);
    static void _NutExportData_(int32_t lastBytes, void *args);

    static void _NutVaoChucNangUpdate_(int32_t lastBytes, void *args);
    static void _CacNutChonPhuongThucUpdate_(int32_t lastBytes, void *args);
    static void _CacNutTrangFOTA_(int32_t lastBytes, void *args);

    static void _ThanhCuonDoThi_(int32_t lastBytes, void *args);

    static void _NutVaoChucNangWiFi_(int32_t lastBytes, void *args); // truc them
    static void _CacNutTrangWiFi_(int32_t lastBytes, void *args);
    static void _NutVaoChucNangThayDoiAdminPassword_(int32_t lastBytes, void *args);
    static void _CacNutTrangThayDoiAdminPassword_(int32_t lastBytes, void *args);
};
#endif
