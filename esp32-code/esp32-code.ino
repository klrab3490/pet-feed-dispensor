#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "camera_pins.h"

#define CAMERA_MODEL_AI_THINKER

const char* ssid = "Galaxy M111142"; // Enter your Wifi Name
const char* password = "1234567890"; // Enter your Wifi Password
const char* websocket_server_host = "192.168.58.28";  // Enter your Lap IP Address IPv4 (change 192.168.58.28)
const uint16_t websocket_server_port = 8885; 

using namespace websockets;
WebsocketsClient client;

void onEventsCallback(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("Connection Opened");
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Connection Closed");
    ESP.restart();
  } else if (event == WebsocketsEvent::GotPing) {
    Serial.println("Got a Ping!");
  } else if (event == WebsocketsEvent::GotPong) {
    Serial.println("Got a Pong!");
  }
}

void setup() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 40;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.onEvent(onEventsCallback);
  Serial.begin(115200);

  while (!client.connect(websocket_server_host, websocket_server_port, "/")) {
    delay(500);
  }
}

void loop() {
  client.poll();
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    return;
  }

  if (fb->format != PIXFORMAT_JPEG) {
    return;
  }

  client.sendBinary((const char*)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}
