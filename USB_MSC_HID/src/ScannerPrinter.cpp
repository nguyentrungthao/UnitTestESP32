#include "ScannerPrinter.h"

ScannerPrinter::ScannerPrinter(unsigned long timeoutValue)
    : timeout(timeoutValue), lastCharTime(0), barcodeData("") {}

// Scanner methods
String ScannerPrinter::getBarcode()
{
  if (millis() - lastCharTime > timeout && !barcodeData.isEmpty())
  {
    String temp = barcodeData;
    barcodeData = "";
    return temp;
  }
  return "";
}
void ScannerPrinter::setupClientHandle(usb_host_client_handle_t xClientHandle)
{
  if (xClientHandle)
  {
    this->clientHandle = xClientHandle;
  }
}

void ScannerPrinter::onGone(const usb_host_client_event_msg_t *eventMsg)
{
  this->clientHandle = NULL;
}

void ScannerPrinter::onKeyboard(hid_keyboard_report_t report, hid_keyboard_report_t last_report)
{
  bool shiftPressed = report.modifier & 0x02;

  for (int i = 0; i < 6; ++i)
  {
    uint8_t keycode = report.keycode[i];
    if (keycode != 0)
    {
      char asciiChar = 0;

      if (keycode >= 4 && keycode <= 29)
      {
        asciiChar = (keycode - 4) + 'A';
        if (!shiftPressed)
          asciiChar = tolower(asciiChar);
      }
      else if (keycode >= 30 && keycode <= 39)
      {
        asciiChar = (keycode == 39) ? '0' : (keycode - 29) + '0';
      }

      if (asciiChar != 0)
      {
        barcodeData += asciiChar;
        lastCharTime = millis();
      }
    }
  }
}

void ScannerPrinter::beginScanner()
{
  // pinMode(USB_5V_PIN, OUTPUT);
  // digitalWrite(USB_5V_PIN, HIGH);
  EspUsbHost::begin();
}

void ScannerPrinter::scannerTask()
{
  EspUsbHost::task();
}

// Printer methods
void ScannerPrinter::printBarcode(const String &barcode)
{
  String command = createBarcodeCommand(barcode);
  Serial1.print(command + "PRINT 1\nCLS\n");
}

void ScannerPrinter::printText(const std::vector<String> &lines)
{
  int y = 20;
  for (const auto &line : lines)
  {
    String textCommand = createTextCommand(line, 20, y);
    Serial1.print(textCommand);
    y += 30;
  }
  Serial1.print("PRINT 1\nCLS\n");
}

void ScannerPrinter::printBarcodeAndText(const String &barcode, const std::vector<String> &lines)
{
  String command = createBarcodeCommand(barcode);
  Serial1.print(command);

  int y = 60;
  for (const auto &line : lines)
  {
    String textCommand = createTextCommand(line, 10, y);
    Serial1.print(textCommand);
    y += 20;
  }
  Serial1.print("PRINT 1\nCLS\n");
}

void ScannerPrinter::beginPrinter()
{
  Serial1.begin(9600, SERIAL_8N1, RXD232_PIN, TXD232_PIN);
  String configCommands = "SIZE 40 mm, 30 mm\nGAP 2 mm, 0 mm\nDIRECTION 1\n";
  Serial1.print(configCommands);
}

String ScannerPrinter::createBarcodeCommand(const String &barcode)
{
  // BARCODE X,Y,”code type”,height,human-readable,rotation,narrow,wide,[alignment,]”content”
  return "BARCODE 10, 5,\"128\", 50, 0, 0, 1, 1, \"" + barcode + "\"\n";
}

String ScannerPrinter::createTextCommand(const String &text, int x, int y)
{
  return "TEXT " + String(x) + "," + String(y) + ",\"1\",0,1,1,\"" + text + "\"\n";
}
