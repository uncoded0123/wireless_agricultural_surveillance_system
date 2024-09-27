#include <WiFi.h>
#include <LoRa.h>

const char* ssid = "ESP32-CAM-AP";
const char* password = "12345678";

#define LORA_CS_PIN   18
#define LORA_RST_PIN  14
#define LORA_IRQ_PIN  26
#define BAND    915E6

#define IMAGE_WIDTH 80
#define IMAGE_HEIGHT 60
#define PACKET_SIZE 230

uint8_t imageData[IMAGE_HEIGHT * IMAGE_WIDTH];

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize LoRa
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_IRQ_PIN);
  if (!LoRa.begin(BAND)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
  Serial.println("LoRa initialized successfully.");
}

void loop() {
  if (receiveImageOverWiFi()) {
    transmitImageOverLoRa();
  }
  delay(5000);
}

bool receiveImageOverWiFi() {
  WiFiClient client;
  if (!client.connect(WiFi.gatewayIP(), 80)) {
    Serial.println("Failed to connect to server");
    return false;
  }

  Serial.println("Connected to server. Receiving image...");
  int bytesRead = client.readBytes(imageData, IMAGE_WIDTH * IMAGE_HEIGHT);
  client.stop();

  if (bytesRead != IMAGE_WIDTH * IMAGE_HEIGHT) {
    Serial.println("Failed to receive complete image");
    return false;
  }

  Serial.println("Image received successfully");
  return true;
}

void transmitImageOverLoRa() {
  int totalPackets = (IMAGE_WIDTH * IMAGE_HEIGHT + PACKET_SIZE - 1) / PACKET_SIZE;
  
  Serial.println("Transmitting image over LoRa...");
  
  for (int packetIndex = 0; packetIndex < totalPackets; packetIndex++) {
    int startByte = packetIndex * PACKET_SIZE;
    int endByte = min((packetIndex + 1) * PACKET_SIZE, IMAGE_WIDTH * IMAGE_HEIGHT);
    int packetLength = endByte - startByte;

    LoRa.beginPacket();
    LoRa.write(packetIndex);
    LoRa.write(packetLength);
    LoRa.write(&imageData[startByte], packetLength);
    LoRa.endPacket();

    Serial.printf("Sent packet %d/%d, length: %d\n", packetIndex + 1, totalPackets, packetLength);
    delay(50);  // Short delay between packets
  }

  Serial.println("Image transmission complete");
}
