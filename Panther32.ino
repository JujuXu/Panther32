#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>

#define CAMERA_MODEL_WROVER_KIT
#include "camera_pins.h"

const char* ssid = "THORGAL";
const char* pswd = "57xwkppgkr";
const char* wsserver = "ws://192.168.4.2:80";

const char* host_ssid = "PANTHER";
const char* host_pswd = "ExecuteOrder66";

const bool ishost = true;

// ping to WS
const int ping = 1000;
long mping;

using namespace websockets;

WebsocketsClient ws;

void onMessageCallback(WebsocketsMessage message) {
    Serial.println(message.data());
}

/*
void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        //Serial.println("Connection established with PantherII WebSocket server !");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        //Serial.println("Connection lost with PantherII Websocket server...");
    }
}*/

void startCameraServer();
void setupLedFlash(int pin);

void setup() {
  Serial.begin(9600);
  //Serial.setDebugOutput(true);

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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  if(ishost) {
    WiFi.softAP(host_ssid,host_pswd);
    /*Serial.print("Hosting Panther32 WiFi connection on address: ");
    Serial.println(WiFi.softAPIP());*/

    startCameraServer();
    /*
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.softAPIP());
    Serial.println("' to connect");*/
  } else {
    //Serial.println("Panther32 starting with non-host WiFi connection.");
    WiFi.begin(ssid,pswd);

    while(WiFi.status() != WL_CONNECTED) {
      delay(500);
      //Serial.print(".");
    }
    /*
    Serial.println("");
    Serial.println("WiFi connected !");
    Serial.print("Address : ");
    Serial.println(WiFi.localIP());*/

    startCameraServer();
    /*
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");*/
  }

  //Serial.println("Connecting to PantherII WebSocket Server...");

  // Setup Callbacks  
  ws.onMessage(onMessageCallback);
  //ws.onEvent(onEventsCallback);
  
  // Connect to server
  bool connected = ws.connect(wsserver);
}

void loop() {
  if(!ws.available()) {
    ws.connect(wsserver);
    delay(500);
    return;
  }
  // TEST FUNCTION, TO BE DELETED
  if(ws.available()) {
    if(Serial.available() > 0) {
      String data = Serial.readStringUntil('\n');

      ws.send(data);
    }

    if(millis() - mping > ping) {
      ws.send("ping");
      mping = millis();
    }

    //Serial.println("Message sent.");

    ws.poll();
  }
}
