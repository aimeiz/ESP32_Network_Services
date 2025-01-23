//Simple websockets server sending stream of photos from ESP32CAM camera.
//Use AI-Thinker ESP-32 board with default settings.
#include <credentials.h>  // consist of MYSSID and MYPASSWORD
//If You don't have library with Your credentials just set before include ESP32_N4etwork_Services.h:
//#define MYSSID "Your wifissid"
//#define MYPASSWORD "Your wifi password"
#include "esp_camera.h"
#define ENABLE_WEBSOCKETS
#include <ESP32_Network_Services.h>
#define FRAMES_PER_SECOND 15
void setupCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.pin_xclk = 0;
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    config.pin_d7 = 35;
    config.pin_d6 = 34;
    config.pin_d5 = 39;
    config.pin_d4 = 36;
    config.pin_d3 = 21;
    config.pin_d2 = 19;
    config.pin_d1 = 18;
    config.pin_d0 = 5;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_pclk = 22;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;

    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Camera init failed!");
        return;
    }
    Serial.println("Camera initialized");
}

void sendPhotoOverWebSocket() {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }

    webSocket.broadcastBIN(fb->buf, fb->len);
    esp_camera_fb_return(fb);
    Serial.println("Photo sent");
}

void setup() {
    Serial.begin(115200);
    startNetworkServices();
    setupCamera();
}

void loop() {
    sendPhotoOverWebSocket();
    delay(1000 / FRAMES_PER_SECOND); // Avoid overwhelming the camera
}
