#ifndef SCANNERPRINTER_H
#define SCANNERPRINTER_H

#include <Arduino.h>
#include <vector>
#include "src/EspUsbHost.h"

#define RXD232_PIN 1
#define TXD232_PIN 2

class ScannerPrinter : public EspUsbHost
{
public:
  ScannerPrinter(unsigned long timeoutValue = 500);

  // Scanner methods
  void setupClientHandle(usb_host_client_handle_t xClientHandle);
  void onGone(const usb_host_client_event_msg_t *eventMsg);
  String getBarcode();
  void onKeyboard(hid_keyboard_report_t report, hid_keyboard_report_t last_report);
  void beginScanner();
  void scannerTask();

  // Printer methods
  void printBarcode(const String &barcode);
  void printText(const std::vector<String> &lines);
  void printBarcodeAndText(const String &barcode, const std::vector<String> &lines);
  void beginPrinter();

private:
  // Scanner
  unsigned long timeout;
  unsigned long lastCharTime;
  String barcodeData;

  // Printer
  String createBarcodeCommand(const String &barcode);
  String createTextCommand(const String &text, int x, int y);
};

#endif // SCANNERPRINTER_H
