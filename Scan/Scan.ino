#include "11_ScannerPrinter.h"

ScannerPrinter xScannerPrinter;

void setup() {
  xScannerPrinter.beginScanner();
}

void loop() {
  xScannerPrinter.scannerTask();
  String barcode = xScannerPrinter.getBarcode();
  Serial.println(barcode);
}