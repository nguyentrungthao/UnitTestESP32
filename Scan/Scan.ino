#include "11_ScannerPrinter.h"
#include "usb_msc_host.h"
#include "EspUsbHost.h"
#include "FS.h"


ScannerPrinter xScannerPrinter;


void setup() {
  xScannerPrinter.beginScanner();
}

void loop() {
  xScannerPrinter.scannerTask();
  String barcode = xScannerPrinter.getBarcode();
  Serial.println(barcode);
}