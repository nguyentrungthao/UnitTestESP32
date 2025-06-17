#ifndef _HMI_PARAM_H_
#define _HMI_PARAM_H_

// Danh sach cac trang
#define _EndIntroPage 0x3C              // 60
#define _WarningPage 0x3D               // 61
#define _HomePage 0x46                  // 70
#define _SettingsPage 0x48              // 72
#define _Settings2Page 0x49             // 72
#define _ProgramLoopPage 0x4A           // 74
#define _SterilizationPage 0x4B         // 75
#define _TrangCaiThoiGianTietTrung 0x4C // 76
#define _RTCPage 0x4D                   // 77
#define _CalibTempPage 0x4E             // 78
#define _ProgramPage 0x4F               // 79
#define _AlarmPage 0x50                 // 80
#define _SegmentAdjPage 0x51            // 81
#define _SetSegmentDelayOffPage 0x52    // 82
#define _SetDelayOffPage 0x53           // 83
#define _SegmentViewPage 0x54           // 84
#define _NumericKeypadPage 0x55         // 85
#define _KeyboardPage 0x56              // 86
#define _SleepPage 0x58                 // 88
#define _DataRecordPage 0x5A            // 90
#define _UpdatePage 0x5B                // 91
#define _FOTAPage 0x5C                  // 92
#define _CheckForUpdatePage 0x5D        // 93
#define _WiFiPage 0x5E                  // 94
#define _TempNumericKeypadPage 0x5F     // 95
#define _FanNumericKeypadPage 0x60      // 96
#define _CO2NumericKeypadPage 0x61      // 97
#define _AdminPasswordPage 0x62         // 98
#define _InfoPage 0x64                  // 100
#define _CalibChoosePage 0x65           // 101
#define _CalibCO2Page 0x66              // 102
#define _PIDPage 0x67                   // 103
#define _DiagnosticsPage 0x68           // 104
#define _ConnectionSelectPage 0x69      // 105
#define _ConfirmPage 0x6A               // 106
#define _ServicePage 0x6B               // 107
#define _MAX_PAGE _ServicePage
/* Sử dụng 0xFFFF để làm mã KeyValue cho việc cấu hình hàm addButtonEvent có nghĩa là với bất kỳ
/* giá trị KeyValue nào thì hàm đăng ký cũng được gọi
*/
#define _AllKeyValue 0xFFFF

// Địa chỉ VP dùng cho các nút nhấn Menu và nút nhấn cài đặt giá trị TRỪ nút Enter
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
#define _KeyValuePage78_setPremeter 0x0F
#define _KeyValuePage78_setDoor 0x10

#define _KeyValueEnterSetRTC 0x11
#define _KeyValueEnterSetAlarm 0x12
#define _KeyValueSetAlarmBelow 0x13
#define _KeyValueSetAlarmAbove 0x14

#define _KeyValueRunButton 0x15
#define _KeyValueResetCalibTemp 0x16

#define _KeyValueExport 0x17

#define _KeyValueUpdate 0x18

#define _KeyValueWifi 0x19 // truc them
#define _KeyValueAdminPassword 0x1A
//---
#define _KeyValueSetSetpointCO2 0x1B
#define _KeyValueSwapGraph 0x1C
#define _KeyValueSetAlarmUnderCO2 0x1F
#define _KeyValueSetAlarmOverCO2 0x20
#define _KeyValueEditCalibCO2 0x24
#define _KeyValueResetCalibCO2 0x22
#define _KeyValueChooseCalibCO2 0x1D
#define _KeyValueChooseCalibTemp 0x1E
#define _KeyValueEnterCalibCO2 0x23
#define _KeyValuePage103Wakeup 0x25
#define _KeyValuePage107_EnterPID 0x26
#define _KeyValuePage103_ExitPID 0x27

#define _KeyValuePage78_calib1 0x28
#define _KeyValuePage78_calib2 0x29
#define _KeyValuePage78_calib3 0x2A
#define _KeyValuePage78_CalibPerimeter 0x2B
#define _KeyValuePage78_CalibDoor 0x2C

#define _KeyValuePage102_calib1 0x2D
#define _KeyValuePage102_calib2 0x2E
#define _KeyValuePage102_calib3 0x2F

#define _KeyValuePage107_EnterDiagnostic 0x30
#define _KeyValuePage73_EnterService 0x31

#pragma region VP các nút  Segment

#define _VPAddressSegmentSelectButton 0x5001
#define _VPAddressSegmentSetpointButton 0x5002
#define _VPAddressSegmentDelayOffButton 0x5003
#define _VPAddressSegmentFanSpeedButton 0x5004
#define _VPAddressSegmentSetpointCO2Button 0x5005
#define _VPAddressSegmentTempMinButton 0x5006
#define _VPAddressSegmentTempMaxButton 0x5007
#define _VPAddressSegmentCO2MinButton 0x501E
#define _VPAddressSegmentCO2MaxButton 0x501F

#define _VPAddressSegmentFunctionButton 0x5009
#define _KeyValueSegmentAddButton 1
#define _KeyValueSegmentSubButton 2
#define _KeyValueSegmentUpButton 3
#define _KeyValueSegmentDownButton 4
#define _KeyValueSegmentEditProgramNameButton 5
#define _KeyValueSegmentEnterButton 6

#define _VPAddressSetTimeButton 0x500C
#define _KeyValueSetDay 1
#define _KeyValueSetMonth 2
#define _KeyValueSetYear 3
#define _KeyValueSetHour 4
#define _KeyValueSetMinute 5
#define _KeyValueSetSecond 6
#define _KeyValueEnterSegmentDelayOff 7
#define _KeyValueEnterSetRTC 8
#define _KeyValueEnterSetDelayOff 9
#define _KeyValueEnterCaiThoiGianTietTrung 10

#pragma region VP cac nut trong Program

#define _VPAddressSelectProgram 0x500D
#define _KeyValueProgram1 1
#define _KeyValueProgram2 2
#define _KeyValueProgram3 3
#define _KeyValueProgram4 4
#define _KeyValueProgram5 5
#define _KeyValueProgram6 6

#define _VPAddressProgramFunction 0x500E
#define _KeyValueProgramUp 1
#define _KeyValueProgramDown 2
#define _KeyValueProgramAdd 3
#define _KeyValueProgramDelete 4
#define _KeyValueProgramEdit 5
#define _KeyValueProgramView 6
#define _KeyValueProgramQuick 7

#define _VPAddressSwitchDelayOff 0x500F
#define _VPAddressEnterRunProgram 0x5010
#define _VPAddressEnterProgramLoop 0x5011
#define _VPAddressInfButton 0x5012
#define _VPAddressSetProgramLoop 0x5013

#pragma region VP cac nut trang Tiệt trùng

#define _VPAddressCaiThoiGianTietTrung 0x5014
#define _VPAddressCaiNhietDoTietTrung 0x5015
#define _VPAddressNextTrongTrangTietTrung 0x5016
#define _VPAddressEnterCaiTietTrung 0x5017

#pragma region VP cac nut trang Warning
#define _VPAddressWarningReturn 0x5018

#pragma region VP cac nut reset Graph
#define _VPAddressResetGraph 0x5019
#define _KeyValueResetGraphTemp 0
#define _KeyValueResetGraphCO2 1

#pragma region VP cac nut chuc nang Update
#define _VPAddressCacNutChonPhuongThucUpdate 0x501A
#define _KeyValueNutChonUpdateBangUSB 1
#define _KeyValueNutChonUpdateBangFOTA 2

#define _VPAddressCacNutTrangFOTA 0x501B
#define _KeyValueNutSSIDFOTA 1
#define _KeyValueNutPASSFOTA 2
#define _KeyValueNutUpdateFOTA 3

// truc them
#pragma region VP cac nut trang WiFi
#define _VPAddressCacNutTrangWiFi 0x501C
#define _KeyValueNutWiFiID 1
#define _KeyValueNutPassWifi 2
#define _KeyValueNutKetNoiWiFi 3
#define _KeyValueNutScanWifi 4
#define _KeyValueNutChonSSID1 5
#define _KeyValueNutChonSSID2 6
#define _KeyValueNutChonSSID3 7
#define _KeyValueNutChonSSID4 8

// Truc them
#pragma region VP cac nut trang Cai mau khau admin
#define _VPAddressCacNutTrangAdminPassword 0x501D
#define _KeyValueNutCaiCurrentAdminPassword 1
#define _KeyValueNutCaiNewAdminPassword 2
#define _KeyValueNutcaiConfirmNewAdminPassword 3
#define _KeyValueNutChangeAdminPassword 4

#pragma region NumericKeypad
#define _VPAddressNumericKeypad 0x5300

#define _VPAddressPage103PID 0x5400
#define _KeyValueSetTempKP 0x0
#define _KeyValueSetTempKI 0x1
#define _KeyValueSetTempKD 0x2
#define _KeyValueSetTempKW 0x3
#define _KeyValueSetTempImax 0x5
#define _KeyValueSetTempImin 0x6
#define _KeyValueSetTempOutmax 0x7
#define _KeyValueSetTempOutmin 0x8

#define _KeyValueSetCO2KP 0x11
#define _KeyValueSetCO2KI 0x12
#define _KeyValueSetCO2KD 0x13
#define _KeyValueSetCO2KW 0x14
#define _KeyValueSetCO2Imax 0x16
#define _KeyValueSetCO2Imin 0x17
#define _KeyValueSetCO2Outmax 0x18
#define _KeyValueSetCO2Outmin 0x19

#define _KeyValueSetPerimeterOutmax 0x1A
#define _KeyValueSetDoorOutmax 0x1B

#define _VPAddressHardWareTesting 0x5500
#define _KeyValueOnOffRelay 0x01
#define _KeyValueTriac1 0x02
#define _KeyValueTriac2 0x03
#define _KeyValueTriac3 0x04
#define _KeyValueTriac4 0x05
#define _KeyValueActiveHeater 0x06

#define _KeyValueEditSpanSenSorCO2 0x10
#define _KeyValueEditZeroSenSorCO2 0x11
#define _KeyValueFactorySenSorCO2 0x12
#define _KeyValueEditValveCO2 0x13
#define _KeyValueActiveCO2 0x14

#define _KeyValueFormatSDCard 0x20

#define _KeyValueUSBHostBackup 0x30

#define _KeyValueServo 0x40

#define _KeyValueSetSSID

#define _VPAddressAction 0x5900

#pragma region ICON VP Address
#define _VPAddressIconRun 0x6000
#define _VPAddressIconQuat 0x6001
#define _VPAddressIconCua 0x6002
#define _VPAddressIconGiaNhiet 0x6003
#define _VPAddressIconSwitchDelayOff 0x6004
#define _VPAddressIconSegment 0x6005
#define _VPAddressIconTickRemoveWater 0x6006
#define _VPAddressIconTickRemoveSample 0x6007
#define _VPAddressIconTickConfirm1 0x6008
#define _VPAddressIconTickConfirm2 0x6009
#define _VPAddressIconLoading 0x600A
#define _VPAddressIonConnect 0x600B
#define _VPAddressIconWiFi 0x600C
#define _VPAddressIconLowercaseKeyboard 0x600D
#define _VPAddressIconUSB 0x600E
#define _VPAddressIconVanCO2 0x600F
#define _VPAddressIconWarningTemp 0x6010
#define _VPAddressIconWarningCO2 0x6011

#pragma endregion

#pragma region Địa chỉ TEXT

#define _VPAddressSetpointTempText 0x8000 // Length text 5
#define _VPAddressFanSpeedText 0x8005     // Length text 5
#define _VPAddressDelayOffText 0x800A     // Length text 15
#define _VPAddressTemperatureText 0x8019  // Length text 5

#define _VPAddressDayText 0x8020    // length text 2
#define _VPAddressMonthText 0x8022  // length text 2
#define _VPAddressYearText 0x8024   // length text 4
#define _VPAddressHourText 0x8028   // length text 2
#define _VPAddressMinuteText 0x802A // length text 2

#define _VPAddressStdTempText 0x8091 // 20 ký tự
#define _VPAddresPgae78_PremeterText 0x8105
#define _VPAddresPgae78_DoorText 0x8145
#define _VPAddressAlarmBelowTempText 0x810A
#define _VPAddressAlarmAboveTempText 0x810F
#define _VPAddressProgramNumText 0x8120
#define _VPAddressSegmentNumText 0x8130
#define _VPAddressPage78_infoTemp 0x8140 // 20 ký tự
#define _VPAddressPage102_infoCO2 0x8154 // 20 ký tự

#define _VPAddressProgramNameText1 0x8200
#define _VPAddressProgramNameText2 _VPAddressProgramNameText1 + 20 // 8214
#define _VPAddressProgramNameText3 _VPAddressProgramNameText2 + 20 // 8228
#define _VPAddressProgramNameText4 _VPAddressProgramNameText3 + 20 // 823C
#define _VPAddressProgramNameText5 _VPAddressProgramNameText4 + 20 // 8250
#define _VPAddressProgramNameText6 _VPAddressProgramNameText5 + 20 // 8278

#define _VPAddressGraphYValueText1 0x8300
#define _VPAddressGraphYValueText2 0x8305
#define _VPAddressGraphYValueText3 0x830A
#define _VPAddressGraphYValueText4 0x830F
#define _VPAddressGraphYValueText5 0x8314
#define _VPAddressGraphYValueText6 0x8319

#define _VPAddressGraphXValueText1 0x8320
#define _VPAddressGraphXValueText2 0x832A
#define _VPAddressGraphXValueText3 0x8334
#define _VPAddressGraphXValueText4 0x833E
#define _VPAddressGraphXValueText5 0x8348
#define _VPAddressGraphXValueText6 0x8352
#define _VPAddressGraphXValueText7 0x835C
#define _VPAddressGraphXValueText8 0x8366
#define _VPAddressGraphXValueText9 0x8370

#define _VPAddressSegmentIconTick1 0x8990
#define _VPAddressSegmentIconTick2 0x8991
#define _VPAddressSegmentIconTick3 0x8992
#define _VPAddressSegmentIconTick4 0x8993
#define _VPAddressSegmentIconTick5 0x8994

#define _VPAddressNumSegmentText1 0x9000 // Có 5 ô, mỗi ô tối đa 2 ký tự
#define _VPAddressNumSegmentText2 _VPAddressNumSegmentText1 + 2
#define _VPAddressNumSegmentText3 _VPAddressNumSegmentText2 + 2
#define _VPAddressNumSegmentText4 _VPAddressNumSegmentText3 + 2
#define _VPAddressNumSegmentText5 _VPAddressNumSegmentText4 + 2

#define _VPAddressSegmentSetpointText1 0x900A // Có 5 ô, mỗi ô tối đa 5 ký tự
#define _VPAddressSegmentSetpointText2 _VPAddressSegmentSetpointText1 + 5
#define _VPAddressSegmentSetpointText3 _VPAddressSegmentSetpointText2 + 5
#define _VPAddressSegmentSetpointText4 _VPAddressSegmentSetpointText3 + 5
#define _VPAddressSegmentSetpointText5 _VPAddressSegmentSetpointText4 + 5

#define _VPAddressSegmentDelayOffText1 0x9023 // Có 5 ô, mỗi ô tối đa 20 ký tự
#define _VPAddressSegmentDelayOffText2 _VPAddressSegmentDelayOffText1 + 20
#define _VPAddressSegmentDelayOffText3 _VPAddressSegmentDelayOffText2 + 20
#define _VPAddressSegmentDelayOffText4 _VPAddressSegmentDelayOffText3 + 20
#define _VPAddressSegmentDelayOffText5 _VPAddressSegmentDelayOffText4 + 20

#define _VPAddressSegmentFanSpeedText1 0x9087 // Có 5 ô, mỗi ô tối đa 5 ký tự
#define _VPAddressSegmentFanSpeedText2 _VPAddressSegmentFanSpeedText1 + 5
#define _VPAddressSegmentFanSpeedText3 _VPAddressSegmentFanSpeedText2 + 5
#define _VPAddressSegmentFanSpeedText4 _VPAddressSegmentFanSpeedText3 + 5
#define _VPAddressSegmentFanSpeedText5 _VPAddressSegmentFanSpeedText4 + 5

#define _VPAddressSegmentTempMinText1 0x90B9 // Có 5 ô, mỗi ô tối đa 5 ký tự
#define _VPAddressSegmentTempMinText2 _VPAddressSegmentTempMinText1 + 5
#define _VPAddressSegmentTempMinText3 _VPAddressSegmentTempMinText2 + 5
#define _VPAddressSegmentTempMinText4 _VPAddressSegmentTempMinText3 + 5
#define _VPAddressSegmentTempMinText5 _VPAddressSegmentTempMinText4 + 5

#define _VPAddressSegmentTempMaxText1 0x90D2 // Có 5 ô, mỗi ô tối đa 5 ký tự
#define _VPAddressSegmentTempMaxText2 _VPAddressSegmentTempMaxText1 + 5
#define _VPAddressSegmentTempMaxText3 _VPAddressSegmentTempMaxText2 + 5
#define _VPAddressSegmentTempMaxText4 _VPAddressSegmentTempMaxText3 + 5
#define _VPAddressSegmentTempMaxText5 _VPAddressSegmentTempMaxText4 + 5

#define _VPAddressNumProgramText1 0x90F7
#define _VPAddressNumProgramText2 _VPAddressNumProgramText1 + 2
#define _VPAddressNumProgramText3 _VPAddressNumProgramText2 + 2
#define _VPAddressNumProgramText4 _VPAddressNumProgramText3 + 2
#define _VPAddressNumProgramText5 _VPAddressNumProgramText4 + 2
#define _VPAddressNumProgramText6 _VPAddressNumProgramText5 + 2

#define _VPAddressTotalNumOfSegmentsText1 0x9103
#define _VPAddressTotalNumOfSegmentsText2 _VPAddressTotalNumOfSegmentsText1 + 2
#define _VPAddressTotalNumOfSegmentsText3 _VPAddressTotalNumOfSegmentsText2 + 2
#define _VPAddressTotalNumOfSegmentsText4 _VPAddressTotalNumOfSegmentsText3 + 2
#define _VPAddressTotalNumOfSegmentsText5 _VPAddressTotalNumOfSegmentsText4 + 2
#define _VPAddressTotalNumOfSegmentsText6 _VPAddressTotalNumOfSegmentsText5 + 2

#define _VPAddressKeyboardWarningText 0xA010

#define _VPAddressCurrentProgramNameText 0xA052 // 20 ky tu
#define _VPAddressProgramLoopText 0xA072        // 3 ký tự
#define _VPAddressCurrentProgramLoopText 0xA075 // 10 ký tự

#define _VPAddressTrangCaiThoiGianTietTrung 0xA07F // 15 ký tự
#define _VPAddressTextNhietDoTietTrung 0xA094      // 5 ký tự

#define _VPAddressTextPhanTramThanhLoading 0xA09A // 5 ky tu

#define _VPAddressTextCheckForUpdates 0xA09F  // 30 ký tự
#define _VPAddressTextSSIDWifi 0xA0BD         // 30
#define _VPAddressTextPASSWifi 0xA0DB         // 15
#define _VPAddressTextWiFiConnectState 0xA0EA // 15 truc them

#define _VPAddressKeyboardInputText 0xA100 // 30 ky tu

#define _VPAddressTextCurrentAdminPassword 0xA11E    // 10 ky tu truc them
#define _VPAddressTextNewAdminPassword 0xA128        // 10 ky tu
#define _VPAddressTextConfirmNewAdminPassword 0xA132 // 10 ky tu
#define _VPAddressTextPasswordCheckState 0xA13C      // 40 ky tu truc them

#define _VPAddressTextVersion 0xAFF0 // 15 ky tu
//---
#define _VPAddressCO2Text 0xAFFF           // 5 ky tu
#define _VPAddressSetpointCO2Text 0xB005   // 5 ký tự
#define _VPAddressAlarmBelowCO2Text 0xB00A // 5 ký tự
#define _VPAddressAlarmAboveCO2Text 0xB00F // 5 ký tự

#define _VPAddressSegmentSetpointCO2Text1 0xB014
#define _VPAddressSegmentSetpointCO2Text2 (_VPAddressSegmentSetpointCO2Text1 + 5)
#define _VPAddressSegmentSetpointCO2Text3 (_VPAddressSegmentSetpointCO2Text2 + 5)
#define _VPAddressSegmentSetpointCO2Text4 (_VPAddressSegmentSetpointCO2Text3 + 5)
#define _VPAddressSegmentSetpointCO2Text5 (_VPAddressSegmentSetpointCO2Text4 + 5)

#define _VPAddressSegmentCO2MinText1 0xB02D
#define _VPAddressSegmentCO2MinText2 (_VPAddressSegmentCO2MinText1 + 5)
#define _VPAddressSegmentCO2MinText3 (_VPAddressSegmentCO2MinText2 + 5)
#define _VPAddressSegmentCO2MinText4 (_VPAddressSegmentCO2MinText3 + 5)
#define _VPAddressSegmentCO2MinText5 (_VPAddressSegmentCO2MinText4 + 5)

#define _VPAddressSegmentCO2MaxText1 0xB046
#define _VPAddressSegmentCO2MaxText2 (_VPAddressSegmentCO2MaxText1 + 5)
#define _VPAddressSegmentCO2MaxText3 (_VPAddressSegmentCO2MaxText2 + 5)
#define _VPAddressSegmentCO2MaxText4 (_VPAddressSegmentCO2MaxText3 + 5)
#define _VPAddressSegmentCO2MaxText5 (_VPAddressSegmentCO2MaxText4 + 5)

#define _VPAddressStdCO2Text 0xB05F // 10 ký tự

#define _VPAddressGraphY_R_ValueText1 0xB069
#define _VPAddressGraphY_R_ValueText2 0xB06E
#define _VPAddressGraphY_R_ValueText3 0xB073
#define _VPAddressGraphY_R_ValueText4 0xB078
#define _VPAddressGraphY_R_ValueText5 0xB07D
#define _VPAddressGraphY_R_ValueText6 0xB082

#define _VPAddressWarningText0 0xB0B4 // 50 ky tu
#define _VPAddressWarningText1 0xB0E6 // 50 ky tu
#define _VPAddressWarningText2 0xB118 // 50 ky tu
#define _VPAddressWarningText3 0xB14A // 50 ky tu
#define _VPAddressWarningText4 0xB17C // 50 ky tu

#define _VPAddressCalibCO2TextInfor 0xB1AE // 20 ký tự

#define _VPAddressPage94SSID1 0xB1C2 // 30 ký tư
#define _VPAddressPage94SSID2 0xB1E0
#define _VPAddressPage94SSID3 0xB1FE
#define _VPAddressPage94SSID4 0xB21C

#define _VPAddressPage70historyEvent 0xB23A // 50 ký tự

#define _VPAddressPage103KpTemp 0xB26C
#define _VPAddressPage103KiTemp 0xB271    // 0xB26C + 5
#define _VPAddressPage103KdTemp 0xB276    // 0xB271 + 5
#define _VPAddressPage103KwTemp 0xB27B    // 0xB276 + 5
#define _VPAddressPage103OUTTemp 0xB280   // 0xB27B + 5
#define _VPAddressPage103ImaxTemp 0xB285  // 0xB280 + 5
#define _VPAddressPage103IminTemp 0xB28A  // 0xB285 + 5
#define _VPAddressPage103OmaxTemp 0xB28F  // 0xB28A + 5
#define _VPAddressPage103OminTemp 0xB294  // 0xB28F + 5
#define _VPAddressPage103PTemp 0xB299     // 0xB294 + 5
#define _VPAddressPage10ITemp 0xB29E      // 0xB299 + 5
#define _VPAddressPage103DTemp 0xB2A3     // 0xB29E + 5
#define _VPAddressPage103WTemp 0xB2A8     // 0xB2A3 + 5
#define _VPAddressPage103StartTemp 0xB2AD // 0xB2A8 + 5
#define _VPAddressPage103endTemp 0xB2B2   // 0xB2AD + 5
#define _VPAddressPage103ErrTemp 0xB2B7   // 0xB2B2 + 5

#define _VPAddressPage103KpCO2 0xB2C1    // 0xB2BC + 5
#define _VPAddressPage103KiCO2 0xB2C6    // 0xB2C1 + 5
#define _VPAddressPage103KdCO2 0xB2CB    // 0xB2C6 + 5
#define _VPAddressPage103KwCO2 0xB2D0    // 0xB2CB + 5
#define _VPAddressPage103OUTCO2 0xB2D5   // 0xB2D0 + 5
#define _VPAddressPage103ImaxCO2 0xB2DA  // 0xB2D5 + 5
#define _VPAddressPage103IminCO2 0xB2DF  // 0xB2DA + 5
#define _VPAddressPage103OmaxCO2 0xB2E4  // 0xB2DF + 5
#define _VPAddressPage103OminCO2 0xB2E9  // 0xB2E4 + 5
#define _VPAddressPage103PCO2 0xB2EE     // 0xB2E9 + 5
#define _VPAddressPage10ICO2 0xB2F3      // 0xB2EE + 5
#define _VPAddressPage103DCO2 0xB2F8     // 0xB2F3 + 5
#define _VPAddressPage103WCO2 0xB2FD     // 0xB2F8 + 5
#define _VPAddressPage103StartCO2 0xB302 // 0xB2FD + 5
#define _VPAddressPage103endCO2 0xB307   // 0xB302 + 5
#define _VPAddressPage103ErrC02 0xB30C   // 0xB307 + 5

#define _VPAddressPage103PremeterHeater 0xB311 // 5
#define _VPAddressPage103DoorHeater 0xB316     // 5

#define _VPAddressPage78_setpointCalib1 0xB31B
#define _VPAddressPage78_setpointCalib2 0xB320
#define _VPAddressPage78_setpointCalib3 0xB325
#define _VPAddressPage78_OffsetCalib1 0xB32A
#define _VPAddressPage78_OffsetCalib2 0xB32F
#define _VPAddressPage78_OffsetCalib3 0xB334
#define _VPAddressPage78_Peremeter 0xB339
#define _VPAddressPage78_Door 0xB33E

#define _VPAddressPage102_setpointCalib1 0xB343
#define _VPAddressPage102_setpointCalib2 0xB348
#define _VPAddressPage102_setpointCalib3 0xB34D
#define _VPAddressPage102_OffsetCalib1 0xB352
#define _VPAddressPage102_OffsetCalib2 0xB357
#define _VPAddressPage102_OffsetCalib3 0xB35C

#define _VPAddressPage104_TempSen1 0xB400 // 10 ký tự
#define _VPAddressPage104_TempSen2 0xB40A // 10 ký tự
#define _VPAddressPage104_TempSen3 0xB414 // 10 ký tự
#define _VPAddressPage104_ACFreq 0xB41E   // 3 ký tự
#define _VPAddressPage104_Relay 0xB421    // 5 ký tự
#define _VPAddressPage104_Triac1 0xB426   // 4 ký tự
#define _VPAddressPage104_Triac2 0xB42A   // 4 ký tự
#define _VPAddressPage104_Triac3 0xB42E   // 4 ký tự
#define _VPAddressPage104_Triac4 0xB432   // 4 ký tự

#define _VPAddressPage104_CO2Sen 0xB436  // 10 ký tự
#define _VPAddressPage104_Span 0xB440    // 4 ký tự
#define _VPAddressPage104_Zero 0xB444    // 4 ký tự
#define _VPAddressPage104_factory 0xB448 // 4 ký tự
#define _VPAddressPage104_valve 0xB44C   // 4 ký tự

#define _VPAddressPage104_detect 0xB450       // 6 ký tự
#define _VPAddressPage104_size 0xB456         // 6 ký tự
#define _VPAddressPage104_free 0xB45C         // 6 ký tự
#define _VPAddressPage104_formatStatus 0xB463 // 4 ký tự

#define _VPAddressPage104_RetartReason 0xB467 // 10 ký tự

#pragma region SP Address
#define _SPAddressProgramNameText1 0xC000
#define _SPAddressProgramNameText2 0xC010
#define _SPAddressProgramNameText3 0xC020
#define _SPAddressProgramNameText4 0xC030
#define _SPAddressProgramNameText5 0xC040
#define _SPAddressProgramNameText5 0xC050

#define _SPAddressSmallGraph1 0xC150
#define _SPAddressLargeGraph 0xC160
#define _SPAddressPage103PIDGraph 0xC170

#define _SPAddressSmallGraphCO2 0xC180
#define _SPAddressLargeGraphCO2 0xC190
#define _SPAddressPage103PIDGraphCO2 0xC200

//
#define _SPAddressPage78_point1Temp 0xF000
#define _SPAddressPage78_point2Temp 0xF010
#define _SPAddressPage78_point3Temp 0xF020
#define _SPAddressPage102_point1CO2 0xF030
#define _SPAddressPage102_point2CO2 0xF040
#define _SPAddressPage102_point3CO2 0xF050

#pragma region VP control brightness
#define _VPAddressBrightness 0xC500

#pragma region Mã màu

#define _RedColor 0xF800
#define _BlueColor 0x0014

#endif