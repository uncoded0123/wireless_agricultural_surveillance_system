#include <LoRa.h>

#define LORA_CS_PIN   18
#define LORA_RST_PIN  14
#define LORA_IRQ_PIN  26
#define BAND    915E6

#define IMAGE_WIDTH 80
#define IMAGE_HEIGHT 60
#define PACKET_SIZE 230

uint8_t imageData[IMAGE_HEIGHT * IMAGE_WIDTH];
bool packetReceived[256] = {false};  // Assuming max 256 packets

void setup() {
  Serial.begin(115200);
  while (!Serial);

  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_IRQ_PIN);
  if (!LoRa.begin(BAND)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
  Serial.println("LoRa Receiver initialized successfully.");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    uint8_t packetIndex = LoRa.read();
    uint8_t packetLength = LoRa.read();

    if (packetLength <= PACKET_SIZE && !packetReceived[packetIndex]) {
      int startByte = packetIndex * PACKET_SIZE;
      LoRa.readBytes(&imageData[startByte], packetLength);
      packetReceived[packetIndex] = true;

      Serial.printf("Received packet %d, length: %d\n", packetIndex, packetLength);

      if (isImageComplete()) {
        printImage();
        resetReceiver();
      }
    }
  }
}

bool isImageComplete() {
  int totalPackets = (IMAGE_WIDTH * IMAGE_HEIGHT + PACKET_SIZE - 1) / PACKET_SIZE;
  for (int i = 0; i < totalPackets; i++) {
    if (!packetReceived[i]) return false;
  }
  return true;
}

void printImage() {
  Serial.println("Received image:");
  for (int y = 0; y < IMAGE_HEIGHT; y++) {
    for (int x = 0; x < IMAGE_WIDTH; x++) {
      Serial.printf("%02X ", imageData[y * IMAGE_WIDTH + x]);
    }
    Serial.println();
  }
}

void resetReceiver() {
  memset(packetReceived, 0, sizeof(packetReceived));
}
