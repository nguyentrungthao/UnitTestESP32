#include "EEPROM.h"

#define EEPROM_PARAMETER_CO2_ADDRESS 50
#define EEPROM_PARAMETER_TEMP_ADDRESS 0
#define EEPROM_CONFIRM_DATA_STRING "LABone"

EEPROMClass xControlParamater("CtrParam");

typedef struct {
  float Kp;
  float Ki;
  float Kd;
  float Kw;
  float WindupMax;
  float WindupMin;
  float OutMax;
  float OutMin;
}PIDParam_t;

typedef struct  {
  PIDParam_t xPID;
  uint16_t Perimter;
  uint16_t Door;
  char pcConfim[8];
}ControlParamaterTEMP ;

typedef struct  {
  PIDParam_t xPID;
  char pcConfim[8];
}ControlParamaterCO2;
void printControlParams(const ControlParamaterTEMP& tempParam, const ControlParamaterCO2& co2Param) {
  Serial.println("===== ControlParamaterTEMP =====");
  Serial.println("  PID Parameters:");
  Serial.print("    Kp: ");
  Serial.println(tempParam.xPID.Kp);
  Serial.print("    Ki: ");
  Serial.println(tempParam.xPID.Ki);
  Serial.print("    Kd: ");
  Serial.println(tempParam.xPID.Kd);
  Serial.print("    Kw: ");
  Serial.println(tempParam.xPID.Kw);
  Serial.print("    OutMax: ");
  Serial.println(tempParam.xPID.OutMax);
  Serial.print("    OutMin: ");
  Serial.println(tempParam.xPID.OutMin);
  Serial.print("    WindupMax: ");
  Serial.println(tempParam.xPID.WindupMax);
  Serial.print("    WindupMin: ");
  Serial.println(tempParam.xPID.WindupMin);
  Serial.print("  Perimter: ");
  Serial.println(tempParam.Perimter);
  Serial.print("  Door: ");
  Serial.println(tempParam.Door);
  Serial.print("  Confirm String: ");
  Serial.println(tempParam.pcConfim);
  Serial.println();

  Serial.println("===== ControlParamaterCO2 =====");
  Serial.println("  PID Parameters:");
  Serial.print("    Kp: ");
  Serial.println(co2Param.xPID.Kp);
  Serial.print("    Ki: ");
  Serial.println(co2Param.xPID.Ki);
  Serial.print("    Kd: ");
  Serial.println(co2Param.xPID.Kd);
  Serial.print("    Kw: ");
  Serial.println(co2Param.xPID.Kw);
  Serial.print("    OutMax: ");
  Serial.println(co2Param.xPID.OutMax);
  Serial.print("    OutMin: ");
  Serial.println(co2Param.xPID.OutMin);
  Serial.print("    WindupMax: ");
  Serial.println(co2Param.xPID.WindupMax);
  Serial.print("    WindupMin: ");
  Serial.println(co2Param.xPID.WindupMin);
  Serial.print("  Confirm String: ");
  Serial.println(co2Param.pcConfim);
  Serial.println();
}

void setEEPROM() {
  if (!xControlParamater.begin(100)) {
    Serial.println("Failed to initialize xControlParamater");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }

  ControlParamaterTEMP xTempParam = {
    .xPID = {
      .Kp = 5.0f,
      .Ki = 0.01f,
      .Kd = 0.0f,
      .Kw = 0.1f,
      .WindupMax = 5.0f,
      .WindupMin = 0.0f,
      .OutMax = 17.0f,
      .OutMin = 0.0f,
    },
    .Perimter = 220,
    .Door = 170,
    .pcConfim = EEPROM_CONFIRM_DATA_STRING
  };
  ControlParamaterCO2 xCO2Param = {
    .xPID = {
      .Kp = 1000.0f,
      .Ki = 0.01f,
      .Kd = 0.0f,
      .Kw = 0.0f,
      .WindupMax = 1000.0f,
      .WindupMin = 0.0f,
      .OutMax = 4000.0f,
      .OutMin = 0.0f,
    },
    .pcConfim = EEPROM_CONFIRM_DATA_STRING
  };

  printControlParams(xTempParam, xCO2Param);
  xControlParamater.writeBytes(EEPROM_PARAMETER_TEMP_ADDRESS, (uint8_t*)(&xTempParam), sizeof(xTempParam));
  xControlParamater.commit();
  delay(10);
  xControlParamater.writeBytes(EEPROM_PARAMETER_CO2_ADDRESS, (uint8_t*)(&xCO2Param), sizeof(xCO2Param));
  xControlParamater.commit();
  delay(10);
  Serial.println("------------------------------------\n");
}

void getEEPROM() {
  ControlParamaterCO2 xCO2Param = {};
  ControlParamaterTEMP xTempParam = {};
  printControlParams(xTempParam, xCO2Param);
  Serial.println("------------------------------------\n");

  xControlParamater.readBytes(EEPROM_PARAMETER_TEMP_ADDRESS, (uint8_t*)(&xTempParam), sizeof(xTempParam));
  xControlParamater.readBytes(EEPROM_PARAMETER_CO2_ADDRESS, (uint8_t*)(&xCO2Param), sizeof(xCO2Param));
  printControlParams(xTempParam, xCO2Param);
}

void setup() {
  Serial.begin(115200);
  setEEPROM();
  getEEPROM();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
}
