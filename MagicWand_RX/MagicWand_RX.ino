// NeoPixel simple sketch (c) 2013 Shae Erisson, adapted to tinyNeoPixel library by Spence Konde 2019.
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library


#include <Arduino.h>

#include <ecc608.h>
#include <led_ctrl.h>
#include <log.h>
#include <lte.h>
#include <mqtt_client.h>
#include <sequans_controller.h>

#include "tinyNeoPixel_Static.h"

#define MQTT_SUB_TOPIC "magicwand"

#define MQTT_THING_NAME    "wand2"
#define MQTT_BROKER        "6de6ef5108aa44d390ca2e675d9d6aad.s1.eu.hivemq.cloud"
#define MQTT_PORT          8883
#define MQTT_USE_TLS       true
#define MQTT_KEEPALIVE     60
#define MQTT_USE_ECC       false
#define HIVEMQ_USERNAME "magicwand_rx"
#define HIVEMQ_PASSWORD "Rossisboss3"

#define LOG Serial3

void TLS_config(void);
void connect_lte(void);
void connect_mqtt(void);

// functions to react to motion
void state_off();
void state_one();
void state_two();

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            PIN_PE1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      32

// Since this is for the static version of the library, we need to supply the pixel array
// This saves space by eliminating use of malloc() and free(), and makes the RAM used for
// the frame buffer show up when the sketch is compiled.

byte pixels[NUMPIXELS * 3];

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.

tinyNeoPixel leds = tinyNeoPixel(NUMPIXELS, PIN, NEO_GRB, pixels);

#define DELAY_TIME 1  // delay time in ms (between lighting individual LEDs)
#define LONG_DELAY_TIME 3000  // 3 sec between colour changes


// states of FSM
typedef enum  {
  OFF = 0,
  ONE = 1,
  TWO = 2
}APP_STATES;

typedef struct  {
  APP_STATES state;
}APP_DATA_t;

APP_DATA_t state_mach;  // create instance of FSM


void test(char* topic, uint16_t message_length, int32_t message_id){
  Log.info("I am a function");
}

void setup() {
  // Setup MQTT
  Log.begin(115200);
  Log.setLogLevelStr("info");
  LedCtrl.begin();
  LedCtrl.startupCycle();

  Log.info("Starting initialization of MQTT with username and password");
  TLS_config();
  connect_lte();
  connect_mqtt();
  Log.info("Connected to MQTT");

//  if(!MqttClient.subscribe(MQTT_SUB_TOPIC)) {
//      Log.error("Could not subscribe to topic");
//  } else {
//      Log.info("Subscribed to topic");
//  }
//  MqttClient.onReceive(test);
  
  pinMode(PIN, OUTPUT);
  // with tinyNeoPixel_Static, you need to set pinMode yourself. This means you can eliminate pinMode()
  // and replace with direct port writes to save a couple hundred bytes in sketch size (note that this
  // savings is only present when you eliminate *all* references to pinMode).
  // leds.begin() not needed on tinyNeoPixel

  state_mach.state = OFF; // start in OFF state
}

void state_off(void){
      for (int i=0; i < NUMPIXELS; i++) {
        leds.setPixelColor(i, leds.Color(0, 0, 0));
        leds.show();
        delay(DELAY_TIME);
      }
      state_mach.state = OFF;
      delay(LONG_DELAY_TIME);
}

void state_one(void){
        for (int i=0; i < NUMPIXELS; i++) {
        leds.setPixelColor(i, leds.Color(10, 0, 0));
        leds.show();
        delay(DELAY_TIME);
      }
      state_mach.state = OFF;
      delay(LONG_DELAY_TIME);
}

void state_two(void){
        for (int i=0; i < NUMPIXELS; i++) {
        leds.setPixelColor(i, leds.Color(0, 10, 0));
        leds.show();
        delay(DELAY_TIME);
      }
      state_mach.state = OFF;
      delay(LONG_DELAY_TIME);
}

void loop() {
    // Recieve message
    String message = MqttClient.readMessage(MQTT_SUB_TOPIC);
    if (message != "") {
        Log.infof("Got new message: %s\r\n", message.c_str());
        if(message == "Plus"){
          state_mach.state = ONE;
          Log.infof("Moving to state ONE");
        } else if(message == "Circle"){
          state_mach.state = TWO;
          Log.infof("Moving to state TWO");
        }
        message = "";
    }
  // Act on message
  switch(state_mach.state)  {
    case OFF: // off functions
      state_off();
      break;
    case ONE:  
      state_one();
    case TWO:
      state_two();
      break;
  }
}


void TLS_config(void)
{

 
    SequansController.begin();
    Log.info(">> Setting up security profile for MQTT TLS without ECC");

    const char* command = "AT+SQNSPCFG=2,2,\"\",0,1";
    
    SequansController.writeBytes((uint8_t*)command, strlen(command), true);

    if (!SequansController.waitForURC("SQNSPCFG", NULL, 0, 4000)) {
        Log.infof(">> Error: Failed to set security profile\r\n");
        return;
    }
    Log.info(">> TLS config complete");
    SequansController.end();

}

void connect_lte(void)
{

    
    Log.info(">> LTE connect");

    // Establish LTE connection
    if (!Lte.begin()) 
    {
        Log.error(">> LTE connect fail\r\n");
        // Halt here
        while (1) {}
    }

    
} // end connect lte

void connect_mqtt(void)
{
    int connect_attempt = 0;
  // Connect to the MGtt broker
    MqttClient.begin(MQTT_THING_NAME,MQTT_BROKER,MQTT_PORT,MQTT_USE_TLS,MQTT_KEEPALIVE,MQTT_USE_ECC,HIVEMQ_USERNAME,
                         HIVEMQ_PASSWORD);
    

    Log.infof(">> broker connect");
    while (!MqttClient.isConnected() && connect_attempt < 10) 
      {
            Log.rawf(".");
            // LedCtrl.toggle(Led::CON);
            delay(1000);
            connect_attempt++;
          
        }
        
    if(connect_attempt < 10)
      {
          Log.rawf(">> broker connected\n");
          MqttClient.subscribe(MQTT_SUB_TOPIC);
          delay(2000);
          /*
          MqttClient.end();
          Log.rawf("hanging up\r\n");
          Lte.end();
          */              
       }
     
    else 
      {
        Log.rawf("\r\n");
        Log.error(">> broker connect fail, \r\n>> terminating LTE connection\r\n");
        Lte.end();
      }
}
