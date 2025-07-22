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
}

void loop()
{
  xScannerPrinter.scannerTask();
  String barcode = xScannerPrinter.getBarcode();
  if(barcode != ""){
    Serial.println(barcode);
  }
}