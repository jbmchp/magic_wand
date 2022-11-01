/**
 * @brief This example uses username and password authentication to connect to a
 * MQTT server.
 */
#include <Arduino.h>
#include <ecc608.h>
#include <led_ctrl.h>
#include <log.h>
#include <lte.h>
#include <mqtt_client.h>
#include <sequans_controller.h>

#include <Adafruit_MPU6050.h> // MPU Calls
#include <Wire.h> // used for I2C
#include <CircularBuffer.h>

/*
 * Motion detection setup
 */
#define CIRCULAR_BUFFER_INT_SAFE
Adafruit_MPU6050 mpu; // MPU object
#define TIME_LENGTH_MS 1000
#define COLLECTION_FREQ_MS 50

#define NUM_COLLECTIONS TIME_LENGTH_MS/COLLECTION_FREQ_MS
// for 4 G, this value is +/- (9.81 * 4)
#define ACCEL_MAX 39.24
// for 500 DPS
// 16-bit/2 for +/-, then / 65.5 for 500 DPS then * SENSORS_DPS_TO_RADS (0.017453293)
#define GYRO_MAX 8.73
#define GYRO_MIN 4.5

typedef struct{
  float gyroXDiff;
  float gyroYDiff;
  float gyroZDiff;
  float gyroXMax;
  float gyroYMax;
  float gyroZMax;
  float gyroXMin;
  float gyroYMin;
  float gyroZMin;
} stats_t;
static const stats_t EmptyStruct; // for resetting stored values

typedef struct{
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
} imu_data_t;

typedef enum {NONE, FLIP, SHAKE, ROTATE} Motions_t; 

sensors_event_t a, g, temp; // throw away variable
imu_data_t data;
stats_t stats;
CircularBuffer<imu_data_t, NUM_COLLECTIONS> buffer;

uint16_t timestamp = 0;
volatile bool update = false;

#define MQTT_PUB_TOPIC "magicwand"
#define MQTT_THING_NAME    "wand1"
#define MQTT_BROKER        "6de6ef5108aa44d390ca2e675d9d6aad.s1.eu.hivemq.cloud"
#define MQTT_PORT          8883
#define MQTT_USE_TLS       true
#define MQTT_KEEPALIVE     60
#define MQTT_USE_ECC       false
#define HIVEMQ_USERNAME "magicwand_tx"
#define HIVEMQ_PASSWORD "Rossisboss4"

#define LOG Serial3
#define GREEN_LED PIN_PB2 // power LED (pin 13)
#define RED_LED PIN_PB5 // command sent LED (pin 10)

bool TLS_config(void);
bool connect_lte(void);
bool connect_mqtt(void);
void run_action(Motions_t motion);
void reset_from_action(void);
Motions_t find_motion(void);
void update_stats(void);

void setup() {
    //noInterrupts();
    // set LEDs as outputs
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);

    // turn both on signaling power, not connected
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, HIGH);
    
    Log.begin(115200);
    Log.setLogLevelStr("debug");
    LedCtrl.begin();
    LedCtrl.startupCycle();

    Wire.swapModule(&TWI1);

    Log.info("Starting initialization of MQTT with username and password");

    // turn off red, indicated connected
    if(mpu.begin() && TLS_config() && connect_lte() && connect_mqtt()){
      digitalWrite(RED_LED, LOW); // green LED stays on
      Log.info("Red turned off");
    } else {
      digitalWrite(GREEN_LED, LOW); // turn green LED off, keep red ON signaling error (power cycle device)
      Log.info("Green turned off");
    }
    Log.rawf(" OK!\r\n");

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G); // 2, 4, 8,16G options
  // RAW to G = 4,096 bits
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ); // 94 Hz for filtering a lot of noise w/o being overpowering
}

void loop() {

  mpu.getEvent(&a, &g, &temp);
  data.accelX = a.acceleration.x;
  data.accelY = a.acceleration.y;
  data.accelZ = a.acceleration.z;
  data.gyroX = g.gyro.x;
  data.gyroY = g.gyro.y;
  data.gyroZ = g.gyro.z;
  delay(10);
  if(!buffer.push(data)){ // wait for buffer to fill
    update_stats();
    Motions_t motion = find_motion();
    if(motion != NONE){
      digitalWrite(RED_LED, HIGH); // red on while animation block
      run_action(motion);
      Log.infof("Ran action");
      reset_from_action();
      Log.infof("Reset from action");
      motion = NONE;
      Log.infof("Cleared motion");
      buffer.clear();
      Log.infof("Cleared buffer");
      Log.infof("Delaying 6s");
      delay(6000);
      digitalWrite(RED_LED, LOW);
      Log.infof("Light off");
    }
  }
}

/*
 * ############# Motion detection functions
 */

void run_action(Motions_t motion){
  String message_to_publish = "";
  switch(motion){
    case SHAKE:
      message_to_publish = "Shake";
      //Serial.println("Shake detected");
      break;
    case ROTATE:
      message_to_publish = "Rotate";
      //Serial.println("Rotate detected");
      break;
    case FLIP:
      message_to_publish = "Flip";
      //Serial.println("Flip wand detected");
      break;
    case NONE:
    //Serial.println("No motion detected");
    break;
  }

  // Send message
  if(message_to_publish != ""){
      Log.infof("Publishing message: %s\r\n", message_to_publish.c_str());
    bool publishedSuccessfully = MqttClient.publish(MQTT_PUB_TOPIC,
                                                    message_to_publish.c_str());
    if (publishedSuccessfully) {
      Log.infof("Published message: %s\r\n", message_to_publish.c_str());
    } else {
      Log.error("Failed to publish");
    }
  }

}

void reset_from_action(void){
  stats.gyroXDiff = 0;
  stats.gyroYDiff = 0;
  stats.gyroZDiff = 0;
  stats.gyroXMax = 0;
  stats.gyroYMax = 0;
  stats.gyroZMax = 0;
  stats.gyroXMin = 0;
  stats.gyroYMin = 0;
  stats.gyroZMin = 0;
}

Motions_t find_motion(void){
// circle has roughly equal gyro values for X,Y, Z
  if ( (stats.gyroXMax > GYRO_MAX*0.8) && (stats.gyroXDiff > (GYRO_MAX*2)*0.8)
      && (stats.gyroYMax < (GYRO_MAX*0.4)) && (stats.gyroYDiff < (GYRO_MAX*2)*0.4) && 
          (stats.gyroZMax < (GYRO_MAX*0.4)) && (stats.gyroZDiff < (GYRO_MAX*2)*0.4) ) {
    return ROTATE;
}
// shake has strong Y and Z, but small X
  else if ( (stats.gyroXMax < GYRO_MAX*0.5) && (stats.gyroXDiff < GYRO_MAX*0.5)
      && (stats.gyroYMax > (GYRO_MAX*0.3)) && (stats.gyroYDiff > (GYRO_MAX*2)*0.3) && 
          (stats.gyroZMax > (GYRO_MAX*0.5)) && (stats.gyroZDiff > (GYRO_MAX*2)*0.5))  {
    return SHAKE;
          }
  // flip has strong Y but small X & Z
  else if((stats.gyroYMax > (GYRO_MAX*0.8)) && (stats.gyroYDiff > (GYRO_MAX*2*0.8))) { 
    return FLIP;
  }
 return NONE;
}

// returns stats for a 1.5 second window
void update_stats(void){
  stats.gyroXMax = buffer[0].gyroX;
  stats.gyroXMin = buffer[0].gyroX;
  stats.gyroYMax = buffer[0].gyroY;
  stats.gyroYMin = buffer[0].gyroY;
  stats.gyroZMax = buffer[0].gyroZ;
  stats.gyroZMin = buffer[0].gyroZ;
  
  // gyro = (16-bit value / 65.5 * SENSORS_DPS_TO_RADS(0.017453293F)) / 2 (bc it is signed)
  for(int i = 1; i < NUM_COLLECTIONS; i++){
    stats.gyroXMax = max(buffer[i].gyroX, stats.gyroXMax);
    stats.gyroXMin = min(buffer[i].gyroX, stats.gyroXMin);
    stats.gyroYMax = max(buffer[i].gyroY, stats.gyroYMax);
    stats.gyroYMin = min(buffer[i].gyroY, stats.gyroYMin);
    stats.gyroZMax = max(buffer[i].gyroZ, stats.gyroZMax);
    stats.gyroZMin = min(buffer[i].gyroZ, stats.gyroZMin);
  }
  stats.gyroXDiff = stats.gyroXMax-stats.gyroXMin;
  stats.gyroYDiff = stats.gyroYMax-stats.gyroYMin;
  stats.gyroZDiff = stats.gyroZMax-stats.gyroZMin;
}
/*
 * ############## End motion detection functions
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
    if (!Lte.begin()) {
        Log.error("Failed to connect to operator");

        // Halt here
        return false;
    }
  return true;
    
} // end connect lte

bool connect_mqtt(void)
{
    int connect_attempt = 0;
  // Connect to the MGtt broker
    if(MqttClient.begin(MQTT_THING_NAME,MQTT_BROKER,MQTT_PORT,MQTT_USE_TLS,MQTT_KEEPALIVE,MQTT_USE_ECC,HIVEMQ_USERNAME,
                         HIVEMQ_PASSWORD)){
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
          //MqttClient.subscribe(MQTT_SUB_TOPIC);
          return true;           
       }
     
    else 
      {
        Log.rawf("\r\n");
        Log.error(">> broker connect fail, \r\n>> terminating LTE connection\r\n");
        Lte.end();
        return false;
      }                     
    }
      return false;
}
