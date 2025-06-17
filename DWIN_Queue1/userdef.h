#ifndef _UserDef_H_
#define _UserDef_H_

// #include <Arduino.h>
// #include "08_PID.h"

// các giá trị cài đặt theo tủ
#define PATH_CALIB_DATA "/CalibData.data"
#define PATH_BASEPROGRAM_DATA "/BaseProgram.data"
#define PATH_RECORD "/record.dat"
#define PATH_LOG "/log"
#define PATH_CONTROL_ALGORITHRM "/controlAlgorithrm.data"

#define userFACTORY_PASSWORD "LABone2025"

#define userEEPROM_PARAMETER_CO2_ADDRESS 50u // byte
#define userEEPROM_PARAMETER_TEMP_ADDRESS 0u // byte
#define userEEPROM_CONFIRM_DATA_STRING "LABone"

#define userSETPOINT_TEMP_MAX 60.0f
#define userSETPOINT_TEMP_MIN 20.0f

#define userSETPOINT_CO2_MAX 20.0f
#define userSETPOINT_CO2_MIN 0.0f

#define userBASE_SEGMENT_DEFAUT() \
    {                             \
        .setPointTemp = 37.0,     \
        .tempMax = 0.3,           \
        .tempMin = -0.3,          \
        .setPointCO2 = 5.0,       \
        .CO2Max = 0.3,            \
        .CO2Min = -0.3,           \
        .fanSpeed = 50,           \
        .delayOffDay = 0,         \
        .delayOffHour = 1,        \
        .delayOffMinute = 0}

#define userBASE_PROGRAM_DEFAUT() \
    {                             \
        .programData = {          \
            .setPointTemp = 37.0, \
            .tempMax = 0.3,       \
            .tempMin = -0.3,      \
            .setPointCO2 = 5.0,   \
            .CO2Max = 0.3,        \
            .CO2Min = -0.3,       \
            .fanSpeed = 50,       \
            .delayOffDay = 0,     \
            .delayOffHour = 0,    \
            .delayOffMinute = 0}, \
        .temperature = 0,         \
        .CO2 = 0,                 \
        .machineState = false,    \
        .delayOffState = false}

#define userTEMP_DEFAUT_CONTROL_PARAMETER() \
    {                                       \
        .xPID = { \ 
                .Kp = 15.0f,                \
                  .Ki = 0.1f,               \
                  .Kd = 500.0f,             \
                  .Kw = 0.12f,              \
                  .WindupMax = 15.0f,       \
                  .WindupMin = -0.001f,     \
                  .OutMax = 17.0f,          \
                  .OutMin = 0.0f,           \
        },                                  \
        .pcConfim = userEEPROM_CONFIRM_DATA_STRING}

#define userCO2_DEFAUT_CONTROL_PARAMETER() \
    {                                      \
        .xPID = {                          \
            .Kp = 2500.0f,                 \
            .Ki = 0.01f,                   \
            .Kd = 0.0f,                    \
            .Kw = 0.05f,                   \
            .WindupMax = 1000.0f,          \
            .WindupMin = -0.001f,          \
            .OutMax = 5000.0f,             \
            .OutMin = -0.0f,               \
        },                                 \
        .pcConfim = userEEPROM_CONFIRM_DATA_STRING}

#define userCO2_DEFAURT_CALIB_PARAMETER() \
    {                                     \
        .point1 = {5.0f, 0},              \
        .point2 = {10.0f, 0},             \
        .point3 = {20.0f, 0},             \
    }
#define userTEMP_DEFAURT_CALIB_PARAMETER() \
    {                                      \
        .point1 = {37.0f, 0},              \
        .point2 = {42.0f, 0},              \
        .point3 = {55.0f, 0},              \
    }

#define userDEFAULY_CALIB_PRAMETER() {                \
    .xCalibCO2 = userCO2_DEFAURT_CALIB_PARAMETER(),   \
    .xCalibTemp = userTEMP_DEFAURT_CALIB_PARAMETER(), \
    .i16Perimeter = 21u,                              \
    .i16Door = 15}

// các kiểu dữ liệu
typedef enum
{
    eOFF_MODE = 0,
    eQUICK_MODE,
    ePROGRAM_MODE,
    eSTERILIZATION_MODE,
    eSLEEP_MODE,
    eTESTING_MODE,
    eMAX_RUN_MODE,
} RunMode_t;

enum CalibPosition_t
{
    ePOINT1 = 0,
    ePOINT2 = 1,
    ePOINT3 = 2,
    eNO_POINT = 3,
    eMAX_POINT,
};

typedef struct PIDCalcu_t
{
  float PTerm;
  float ITerm;
  float DTerm;
  float feedBackWindup;
  float Output;
};
typedef struct PIDParam_t
{
  float Kp;
  float Ki;
  float Kd;
  float Kw;
  float WindupMax;
  float WindupMin;
  float OutMax;
  float OutMin;
};

typedef struct
{
    float setPointTemp;
    float tempMax;
    float tempMin;
    float setPointCO2;
    float CO2Max;
    float CO2Min;
    int8_t fanSpeed;
    int8_t delayOffDay;
    int8_t delayOffHour;
    int8_t delayOffMinute;
} Program_t;

typedef struct
{
    Program_t programData;
    float temperature;
    float CO2;
    bool machineState;
    bool delayOffState;
} BaseProgram_t;

typedef struct
{
    float Setpoint;
    float value;
} CalibData_t;

union CalibStruct_t
{
    struct
    {
        CalibData_t point1;
        CalibData_t point2;
        CalibData_t point3;
    };
    CalibData_t pxArrPoint[3];
};

typedef CalibStruct_t CO2CalibStruct_t;
typedef CalibStruct_t TempCalibStruct_t;

struct ParameterSaveInSDCard_t
{
    CO2CalibStruct_t xCalibCO2;
    TempCalibStruct_t xCalibTemp;
    int16_t i16Perimeter;
    int16_t i16Door;
};

struct ParamterSensorlineOutput_t
{
    float a;
    float b;
};

typedef struct
{
    float setpointTemp;
    float valueTemp;
    float setpointCO2;
    float valueCO2;
    uint8_t fan;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} RecordData_t;

struct RecordRunningProgram_t
{
    BaseProgram_t xBaseProgram;
    uint32_t u32CurrentProgramLoop;
    uint32_t u32CurrentTotalProgramLoop;
    time_t xTimeRemaning;
    char pcName[16];
    uint8_t u8CurrentSegment;
};

typedef struct
{
    char ssid[30];
    char password[30];
    bool state;
} WiFiConfig_t;

// Program structure definition
struct PIDData
{
    float Setpoint;
    float Temperature;
    float Kp;
    float Ki;
    float Kd;
    float PTerm;
    float ITerm;
    float DTerm;
    float Output;
    int Triac;
    int IMax;
    int UMax;
    int TGondinh;
    int TGgianhiet;
};

struct FrameDataQueue_t
{
    int32_t event;
    void *pvData;
};

struct FrameStaticDataQueue_t
{
    int32_t event;
    const char *pcSource;
    union
    {
        uint8_t pu8Data[8];
        double fValue;
        int32_t i32Value;
        uint32_t u32Value;
    };
};

typedef enum
{
    MAIN_UPDATE_USB,
    MAIN_UPDATE_FOTA,
} MethodUpdates_t;

typedef enum
{
    eEVENT_CONNECT_WIFI,
    eEVENT_DISCONNECT_WIFI,
    eEVENT_CONNECT_WIFI_SUCCESS,
    eEVENT_LOST_WIFI_CONNECTION,
} WifiEvent_t;

// các macro
#define _CHECK_AND_WARNING_PAGE(condition, message, returnPage) \
    do                                                          \
    {                                                           \
        if (condition)                                          \
        {                                                       \
            _dwin.HienThiWarning(message, returnPage);          \
            _dwin.Buzzer(160);                                  \
        }                                                       \
    } while (0)

#define userCHECK_FLAG(flagGroup, flag) ((((flagGroup) & (flag))) != 0)
#define userSET_FLAG(flagGroup, flag) (flagGroup) |= (flag)
#define userRESET_FLAG(flagGroup, flag) (flagGroup) &= ~(flag)

// cấu hình các task trong freeRTOS
#define userTASK_PRIORITY_DWIN
#define userTASK_PRIORITY_HEATER_LOOP
#define userTASK_PRIORITY_HEATER_ACDET

#endif
