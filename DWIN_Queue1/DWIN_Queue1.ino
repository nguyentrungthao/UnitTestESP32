#include <AnhLABV01HardWare.h>
#include <FifoBuffer.h>
#include <vector>
#include <esp_task_wdt.h>
#include "HMI.h"

HMI _hmi(Serial2, HMI_RX_PIN, HMI_TX_PIN);

void hmiSetEvent(const hmi_set_event_t &event){
  Serial.printf("%s %d\n",__func__, event.type);
}
bool hmiGetEvent(hmi_get_type_t event, void *args){
  Serial.printf("%s %d\n",__func__, event);
}

void setup()
{
  // put your setup code here, to run once:
  esp_task_wdt_deinit();
  Serial.begin(115200);
  _hmi.KhoiTao();
  _hmi.crcEnabled(true);
  _hmi.DangKyHamSetCallback(hmiSetEvent);
  _hmi.DangKyHamGetCallback(hmiGetEvent);
}
void loop()
{
  Serial.printf("--------------------------------------\n");
  _hmi.setPage(70);
  Serial.println(_hmi.getPage());
  _hmi.setText(_VPAddressTemperatureText, "0");
  _hmi.setText(_VPAddressCO2Text, "500");
  _hmi.setText(_VPAddressSetpointTempText, "0");
  _hmi.setText(_VPAddressSetpointCO2Text, "500");
  _hmi.setText(_VPAddressGraphYValueText1, "0");
  _hmi.setText(_VPAddressGraphYValueText2, "500");
  _hmi.setText(_VPAddressGraphYValueText3, "0");
  _hmi.setText(_VPAddressGraphYValueText4, "500");
  _hmi.setText(_VPAddressGraphYValueText5, "0");
  _hmi.setText(_VPAddressGraphYValueText6, "500");
  _hmi.setText(_VPAddressGraphY_R_ValueText1, "0");
  _hmi.setText(_VPAddressGraphY_R_ValueText2, "500");
  _hmi.setText(_VPAddressGraphY_R_ValueText3, "0");
  _hmi.setText(_VPAddressGraphY_R_ValueText4, "500");
  _hmi.setText(_VPAddressGraphY_R_ValueText5, "0");
  _hmi.setText(_VPAddressGraphY_R_ValueText6, "500");
  delay(1000);
}