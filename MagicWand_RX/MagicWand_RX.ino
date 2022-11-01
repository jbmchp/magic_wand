/* 
 *  RX side of the Magic Wand. It expects one of the following to be published to magicwand:
 *  magicwand/shake
 *  magicwand/flip
 *  magicwand/rotate
 *  
 *  TODO:
 *  adjust animation lengths so it goes for ~10 seconds before returning to standby
 *  
 *  On receipt, the animation will commence for 10 seconds, then return to the microchip logo
 */



#include <Arduino.h>

// neopixel array libraries
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "mchp.h"
#include "mchp_upside_down.h"
#include "mchp_rotations.h"

bool TLS_config(void);
bool connect_lte(void);
bool connect_mqtt(void);
void logo_shake(void);
void logo_rotate(void);
void logo_flip(void);
void logo_standby(void);

#define PIN PIN_PB2 // pin D13, PIN_PE1 = D6

#define WHITE_BACKDROP // if a white background should fill the rest of the pixel rectangle
#define BRIGHTNESS 32 // turn up if in bright room


#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(32, 8, 1, 3, PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
    NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG +
// progressive vs zigzag makes no difference for a 4 arrays next to one another
    NEO_TILE_COLUMNS+ NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE,
  NEO_GRB + NEO_KHZ800 );

// COlors used
#define LED_BLACK    0
#define LED_GREEN_LOW     (15 << 5)  
#define LED_RED_HIGH    (31 << 11)
#define LED_GREEN_HIGH    (63 << 5)  
#define LED_BLUE_HIGH     31
#define LED_WHITE_HIGH    (LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH)


#include <ecc608.h>
#include <led_ctrl.h>
#include <log.h>
#include <lte.h>
#include <mqtt_client.h>
#include <sequans_controller.h>


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
//#define Serial Serial3

// functions to react to motion
void state_off();
void state_standby();
void state_flip();
void state_shake();
void state_rotate();
void call_animation(int repetitions, void (*animation_function)());

// states of FSM
typedef enum  {
  OFF, STANDBY, SHAKE, FLIP, ROTATE
}APP_STATES;

typedef struct  {
  APP_STATES state;
}APP_DATA_t;

APP_DATA_t state_mach;  // create instance of FSM


void setup() {
  //Serial.begin(115200);
  // Setup MQTT
  Log.begin(115200);
  Log.setLogLevelStr("info");
  LedCtrl.begin();
  LedCtrl.startupCycle();

  // Setup LED matrix
  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(BRIGHTNESS);
  matrix->clear();
  matrix->show();

  // Setup cell connection, matrix turns green then black when ready
  Log.info("Starting initialization of MQTT with username and password");
  matrix->clear();
  matrix->fillScreen(LED_GREEN_LOW);
  matrix->show();
  if(TLS_config() && connect_lte() && connect_mqtt()){
    matrix->clear();
    matrix->fillScreen(LED_GREEN_HIGH);
    matrix->show();
    delay(2000);
    matrix->clear();
    matrix->fillScreen(LED_BLACK);
    matrix->show();
  } else {
    matrix->clear();
    matrix->fillScreen(LED_RED_HIGH);
    matrix->show();
    while(1);
  }
  
  state_mach.state = STANDBY; // start in STANDBY state
}

void loop() {
    // Recieve message
    String message = MqttClient.readMessage(MQTT_SUB_TOPIC);
    // Update state
    if (message != "") {
        Log.infof("Got new message: %s\r\n", message.c_str());
        if(message == "Shake"){
          state_mach.state = SHAKE;
          Log.infof("Moving to state SHAKE");
        } else if(message == "Flip"){
          state_mach.state = FLIP;
          Log.infof("Moving to state FLIP");
        } else if(message == "Rotate"){
          state_mach.state = ROTATE;
          Log.infof("Moving to state ROTATE");
        }
        message = "";
    }
  // Act on message (hold for ~10s
  switch(state_mach.state)  {
    case OFF: // off functions
    case STANDBY:
      state_standby();
      break;
    case SHAKE:
      call_animation(5, state_shake);
      state_mach.state = STANDBY;
      break;
    case FLIP:
      call_animation(1, state_flip);
      state_mach.state = STANDBY;
      break;
    case ROTATE:
      call_animation(2, state_rotate);
      state_mach.state = STANDBY;

      break;
    default:
      state_off();
      break;
  }
}

void call_animation(int repetitions, void (*animation_function)()){
  for(int i = 0; i < repetitions; i++){
    animation_function();
  }
}

void state_off(void){
  matrix->clear();
  matrix->fillScreen(LED_BLACK);
  matrix->show();
}

void state_standby(void){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp, 24, 24);
    matrix->show();
}

void state_shake(void){
  logo_shake();
}

void state_flip(void){
  logo_flip();
}

void state_rotate(void){
  logo_rotate();
}

/*
 * ############## ANIMATION FUNCTIONS
 */
void logo_shake(void){
  for(int i = 4; i >= 0; i--){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(i, 0, (const uint16_t *) mchp, 24, 24);
    matrix->show();
    delay(25);
  }
  for(int i = 1; i < 8; i++){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(i, 0, (const uint16_t *) mchp, 24, 24);
    matrix->show();
    delay(25);
  }
  for(int i = 8; i > 4; i--){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(i, 0, (const uint16_t *) mchp, 24, 24);
    matrix->show();
    delay(25);
  }
}
void logo_flip(void){
  // flip
  for(int i = 0; i < 12; i++){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp, 24, 24);
    for(int j = -1; j < i; j++){
      matrix->drawLine(0, 0+j, 32, 0+j, LED_BLACK);
      matrix->drawLine(0, 24-j, 32, 24-j, LED_BLACK);
    }
    matrix->show();
  }

  for(int i = 12; i >= 0; i--){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp_upside_down, 24, 24);
    for(int j = 0; j < i; j++){
      matrix->drawLine(0, 0+j, 32, 0+j, LED_BLACK);
      matrix->drawLine(0, 24-j, 32, 24-j, LED_BLACK);
    }
    matrix->show();
  }

  delay(250);

  // unfip
  for(int i = 0; i < 12; i++){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);    
    #endif
    matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp_upside_down, 24, 24);
    for(int j = -1; j < i; j++){
      matrix->drawLine(0, 0+j, 32, 0+j, LED_BLACK);
      matrix->drawLine(0, 24-j, 32, 24-j, LED_BLACK);
    }
    matrix->show();
  }

  for(int i = 12; i >= 0; i--){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp, 24, 24);
    for(int j = 0; j < i; j++){
      matrix->drawLine(0, 0+j, 32, 0+j, LED_BLACK);
      matrix->drawLine(0, 24-j, 32, 24-j, LED_BLACK);
    }
    matrix->show();
  }
  delay(250);
}

void logo_rotate(void){
  #define ROTATION_DEGREES 15
  for(int i = 0; i < (int)360/ROTATION_DEGREES; i++){
   matrix->clear();
   #ifdef WHITE_BACKDROP
   matrix->fillScreen(LED_WHITE_HIGH);    
   #endif
   matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp_rotate[i], 24, 24);
   matrix->show();
   delay(50);
  }
   matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp, 24, 24);
}

/*
 * ############## END OF ANIMATION FUNCTIONS
 */

 /*
  * ############ IoT FUNCTIONS
  */
bool TLS_config(void)
{
  
    SequansController.begin();
    Log.info(">> Setting up security profile for MQTT TLS without ECC");

    const char* command = "AT+SQNSPCFG=2,2,\"\",0,1";
    
    SequansController.writeBytes((uint8_t*)command, strlen(command), true);

    if (!SequansController.waitForURC("SQNSPCFG", NULL, 0, 4000)) {
        Log.infof(">> Error: Failed to set security profile\r\n");
        return false;
    }
    Log.info(">> TLS config complete");
    SequansController.end();
    return true;
}

bool connect_lte(void)
{
    
    Log.info(">> LTE connect");

    // Establish LTE connection
    if (!Lte.begin()) 
    {
        Log.error(">> LTE connect fail\r\n");
        return false;
        // Halt here
    }
  return true;
    
} // end connect lte

bool connect_mqtt(void)
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
          return true;
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
        return false;
      }
      return false;
}
