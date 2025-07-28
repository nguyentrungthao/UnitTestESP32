#include "11_ScannerPrinter.h"
#include "src/usb_msc_host.h"
#include "src/EspUsbHost.h"
#include "FS.h"

#define TAG "MAIN"

ScannerPrinter xScannerPrinter;

void setup()
{
  Serial.begin(115200);

  xScannerPrinter.beginScanner();
  delay(1000);
  uint8_t u8Try = 100;
  while (u8Try--)
  {
    xScannerPrinter.scannerTask();
    delay(1);
  }

  pinMode(0, INPUT);
}

void loop()
{
  while (digitalRead(0))
  {
    delay(10);
  }
  xScannerPrinter.scannerTask();
  String barcode = xScannerPrinter.getBarcode();
  if (barcode != "")
  {
    Serial.println(barcode);
  }
}