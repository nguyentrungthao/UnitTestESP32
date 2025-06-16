#include <SPI.h>
#include <EthernetESP32.h>
#include <HTTPClient.h>  // Cho phép khai báo kiểu HTTPClient

EthernetClient client;
HTTPClient http;

#define URL_POSTDuLieuTuBoardLenServer "https://App.IoTVision.vn/api/LABone_KingIncuCO2_DuLieu"

#define ETH_MOSI_PIN 41
#define ETH_MISO_PIN 40
#define ETH_SCK_PIN 39
#define ETH_INT_PIN 42
#define ETH_CS_PIN 38

String ID = "1234567890AB";

uint8_t mac[] = {
  0x12, 0x34, 0x56, 0x78, 0x90, 0xAB
};

IPAddress ip(192, 168, 137, 6);

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  // Khởi tạo SPI và Ethernet
  SPI.begin(ETH_SCK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN, ETH_CS_PIN);
  Ethernet.init(ETH_CS_PIN);
  Ethernet.begin(mac, ip);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found. Sorry, can't run without hardware.");
    while (true) {
      delay(1);
    }
  }

  while (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    delay(500);
  }

  Serial.println("Ethernet connected.");
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());

  // Tạo dữ liệu POST
  String DuLieu = "175;500;27.9;32.5;32.49;130;80.6;21:34:13 30/07/2023";  // <- Bị thiếu dấu ; trong code cũ
  String data = "{\"ID\":\"" + ID + "\",\"S\":\"" + DuLieu + "\"}";

  // Bắt đầu gửi POST
  http.begin(client, (char*)URL_POSTDuLieuTuBoardLenServer);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(data);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("Response: ");
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void loop() {
  // Nothing here
}
