#include "esp_camera.h"
#include <WiFi.h>

const char* ssid = "ESP32-CAM-AP";
const char* password = "12345678";

#define IMAGE_WIDTH 80
#define IMAGE_HEIGHT 60

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QQVGA);
  s->set_pixformat(s, PIXFORMAT_GRAYSCALE);

  WiFi.softAP(ssid, password);
  server.begin();
  Serial.println("Camera initialized and WiFi AP started");
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");
    camera_fb_t * fb = esp_camera_fb_get();
    if(fb) {
      uint8_t image[IMAGE_HEIGHT][IMAGE_WIDTH];

      // Sample the larger image to get a smaller one
      for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
          int sourceX = x * fb->width / IMAGE_WIDTH;
          int sourceY = y * fb->height / IMAGE_HEIGHT;
          image[y][x] = fb->buf[sourceY * fb->width + sourceX];
        }
      }

      // Print and send the 2D array
      Serial.println("Image captured. 2D array:");
      for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
          Serial.printf("%3d ", image[y][x]);
          client.write(image[y][x]);
        }
        Serial.println();
      }

      esp_camera_fb_return(fb);
    } else {
      Serial.println("Camera capture failed");
    }
    client.stop();
    Serial.println("Client disconnected");
    delay(5000);  // Wait 5 seconds before next capture
  }
}
