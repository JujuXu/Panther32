// This code has been created on the example code from ArduinoIDE "CameraWebServer".

// import libraries and external files
#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>

// define camera pins
#define CAMERA_MODEL_WROVER_KIT
// import camera pins
#include "camera_pins.h"

// set up constants for WiFi connection and websocket server address
// ssid & pswd to connect to an external WiFi
const char* ssid = "THORGAL";
const char* pswd = "57xwkppgkr";

// host_ssid & host_pswd for the hosted wifi
const char* host_ssid = "PANTHER";
const char* host_pswd = "ExecuteOrder66";
const char* wsserver = "ws://192.168.4.2:80";
//const char* wsserver = "ws://192.168.1.160:80";

// choose whether the ESP32 is a WiFi host or not
const bool ishost = true;

// delay in ms for sending a ping message to the websocket server. It ensures a JavaApp-side connection reset for the websocket
// if ping isn't received by the JavaAPP, it resets connection
const int ping = 1000;
// same for sending messages
const int msg = 250;

// time in ms for sending ping message and for sending data received by the ATMEGA328P-PU chip
// these variables are for saving millis()
long mping, mdata;

// using websocket library
using namespace websockets;

// define websocket object
WebsocketsClient ws;

// websocket event
// if a message is received by the websocket server, it is sent to the ATMEGA328P-PU chip with serial communication (rx/tx)
void onMessageCallback(WebsocketsMessage message) {
    Serial.println(message.data());
}

// function to start camera web server
void startCameraServer();
void setupLedFlash(int pin);

void setup() {
  // starting serial comm
  Serial.begin(9600);

  // check if the ESP32 has to host a WiFi connection or to connect to an external one
  if(ishost) {
    WiFi.softAP(host_ssid,host_pswd); // create an access point
  } else {
    WiFi.begin(ssid,pswd); // connect to a wifi connection

    while(WiFi.status() != WL_CONNECTED) {
      delay(500);
    }

    /*Serial.println("");
    Serial.print("Address : ");
    Serial.println(WiFi.localIP());*/
  }

  // Setup Callbacks  
  ws.onMessage(onMessageCallback);
  
  // Connect to server
  bool connected = ws.connect(wsserver);

  // camera settings settup
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
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      config.jpeg_quality = 20;
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
    s->set_framesize(s, FRAMESIZE_HD);
  }

  startCameraServer();
}

void loop() {  
  // check if websocket server is available
  if(ws.available()) {
    // check if bytes are available on the serial comm
    // and if it's time to send data
    if(Serial.available() > 0  && millis()-mdata >= msg) {
      mdata = millis(); // overwrite last millis
      String data = Serial.readStringUntil('\n'); // read last string on serial comm

      ws.send(data); // send string received to the websocket server
    }

    // check if it's time to send a ping message
    if(millis() - mping > ping) {
      ws.send("ping"); // send "ping" message
      mping = millis(); // overwrite last millis

      /*ws.send("PS_FRONT="+String(random(0,50)));
      ws.send("PS_LEFT="+String(random(0,50)));
      ws.send("PS_RIGHT="+String(random(0,50)));

      ws.send("ACC_X="+String(random(0,50)));
      ws.send("ACC_Y="+String(random(0,50)));
      ws.send("ACC_Z="+String(random(0,50)));

      ws.send("CURR_A="+String(random(0,50)));
      ws.send("CURR_B="+String(random(0,50)));*/
    }

    ws.poll(); // poll the websocket connection
  } else {
    
    ws.connect(wsserver); // try to reconnect to websocket server
    delay(500); // wait time
    return;

  }
}
