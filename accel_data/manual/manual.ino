/*
 * MPU 5060 gives 16-bit measurements
 * 
 */



#include <Adafruit_MPU6050.h> // MPU Calls
#include <Wire.h> // used for I2C
#include <CircularBuffer.h>

Adafruit_MPU6050 mpu; // MPU object
#define TIME_LENGTH_MS 1500
#define COLLECTION_FREQ_MS 50

#define NUM_COLLECTIONS TIME_LENGTH_MS/COLLECTION_FREQ_MS

// for 4 G, this value is +/- (9.81 * 4)
#define ACCEL_MAX 39.24

// for 500 DPS
// 16-bit/2 for +/-, then / 65.5 for 500 DPS then * SENSORS_DPS_TO_RADS (0.017453293)
#define GYRO_MAX 8.73


typedef struct{
  float gyroXDiff;
  float gyroYDiff;
  float gyroZDiff;
  float accelXDiff;
  float accelYDiff;
  float accelZDiff;
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

typedef enum {NONE, PLUS, CIRCLE} Motions_t;

sensors_event_t a, g, temp; // throw away variable
imu_data_t data;
stats_t stats;
CircularBuffer<imu_data_t, NUM_COLLECTIONS> buffer;

#define Serial Serial3 // redirect serial lines

//#define DEBUG
uint16_t timestamp = 0;
volatile bool update = false;

void setup() 
{
  Wire.swapModule(&TWI1);
  Serial.begin(115200);
  while (!Serial)
    delay(10); // pause until serial console opens

  // Init MPU
  if (!mpu.begin()) {
    while (1) {
      Serial.println("Failed to find MPU6050 chip");
      delay(1000);
    }
  }


  mpu.setAccelerometerRange(MPU6050_RANGE_4_G); // 2, 4, 8,16G options
  // RAW to G = 4,096 bits
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ); // 94 Hz for filtering a lot of noise w/o being overpowering
  delay(1000);
  // Header for CSV file
  Serial.print("Timestamp,AccelerometerX,AccelerometerY,AccelerometerZ,Timestamp,GyroscopeX,GyroscopeY,GyroscopeZ\r\n");


  // Setup 10ms timer
  takeOverTCA0(); // this replaces disabling and resettng the timer, required previously.
  TCA0.SINGLE.CTRLB = 0b00000000;
  TCA0.SINGLE.INTCTRL = 0b00000001; // Interrupt on Overflow
  TCA0.SINGLE.PER = 0xEA; // 0x2DB4 = 500ms, 0x75 = 5ms, 0xEA = 10ms, 0x492 = 50ms
  TCA0.SINGLE.CTRLA = 0b0001111; // Enable the timer with 1024 prescalar

}

// Update row
ISR(TCA0_OVF_vect) {
    mpu.getEvent(&a, &g, &temp);
    data.accelX = a.acceleration.x;
    data.accelY = a.acceleration.y;
    data.accelZ = a.acceleration.z;
    data.gyroX = g.gyro.x;
    data.gyroY = g.gyro.y;
    data.gyroZ = g.gyro.z;
    update = true;


    
//    // Collect data
//    Serial.print(timestamp*10);
//    Serial.print(',');
//    Serial.print(data.a.acceleration.x);
//    Serial.print(',');
//    Serial.print(data.a.acceleration.y);
//    Serial.print(',');
//    Serial.print(data.a.acceleration.z);
//    Serial.print(',');
//    Serial.print(timestamp*10);
//    Serial.print(',');
//    Serial.print(data.g.gyro.x);
//    Serial.print(',');
//    Serial.print(data.g.gyro.y);
//    Serial.print(',');
//    Serial.print(data.g.gyro.z);
//    Serial.println();
//    timestamp++;
    
    TCA0.SINGLE.INTFLAGS  = 0b00000001; // Clear the interrupt flags, otherwise the interrupt will fire continually
}

void loop()
{
  bool in_motion = false;
  if(update){ // on update
    if(!buffer.push(data)){ // wait for buffer to fill
       update_stats();
    }
    Motions_t motion = find_motion();
//    if(motion != NONE && in_motion == false){
//      in_motion = true;
    if(motion != NONE){
      run_action(motion);
      reset_from_action();
      motion == NONE;
      buffer.clear();
    }

//    } else if(in_motion == true && motion == NONE){ // wait until motion is finished before running another action
//      in_motion == false;
//    }
    update = false;
  }

}

void run_action(Motions_t motion){
  switch(motion){
    case PLUS:
      Serial.println("Plus detected");
    break;  
    case NONE:
    break;
  }
}

void reset_from_action(void){
  stats = EmptyStruct;
}

Motions_t find_motion(){
  Serial.print("GyroX: ");
  Serial.print(stats.gyroXDiff);
  Serial.print(", GyroY: ");
  Serial.print(stats.gyroYDiff);
  Serial.print(", GyroZ: ");
  Serial.print(stats.gyroZDiff);
  Serial.print(", AccelX: ");
  Serial.print(stats.accelXDiff);
  Serial.print(", AccelY: ");
  Serial.print(stats.accelYDiff);
  Serial.print(", AccelZ: ");
  Serial.print(stats.accelZDiff);
  Serial.println();
//  if(stats.gyroZDiff > (GYRO_MAX*2)*0.85 && stats.gyroXDiff > (GYRO_MAX*2)*0.85){
//    return PLUS;
//  }
  return NONE;
}

// returns stats for a 1.5 second window
void update_stats(){
  float gyroXMax = buffer[0].gyroX;
  float gyroXMin = buffer[0].gyroX;
  float gyroYMax = buffer[0].gyroY;
  float gyroYMin = buffer[0].gyroY;
  float gyroZMax = buffer[0].gyroZ;
  float gyroZMin = buffer[0].gyroZ;
  float accelXMax = buffer[0].accelX;
  float accelXMin = buffer[0].accelX;
  float accelYMax = buffer[0].accelY;
  float accelYMin = buffer[0].accelY;
  float accelZMax = buffer[0].accelZ;
  float accelZMin = buffer[0].accelZ;
  
  // gyro = (16-bit value / 65.5 * SENSORS_DPS_TO_RADS(0.017453293F)) / 2 (bc it is signed)
  for(int i = 1; i < NUM_COLLECTIONS; i++){
    gyroXMax = max(buffer[i].gyroX, gyroXMax);
    gyroXMin = min(buffer[i].gyroX, gyroXMin);
    gyroYMax = max(buffer[i].gyroY, gyroYMax);
    gyroYMin = min(buffer[i].gyroY, gyroYMin);
    gyroZMax = max(buffer[i].gyroZ, gyroZMax);
    gyroZMin = min(buffer[i].gyroZ, gyroZMin);
    accelXMax = max(buffer[i].accelX, accelXMax);
    accelXMin = min(buffer[i].accelX, accelXMin);
    accelYMax = max(buffer[i].accelY, accelYMax);
    accelYMin = min(buffer[i].accelY, accelYMin);
    accelZMax = max(buffer[i].accelZ, accelZMax);
    accelZMin = min(buffer[i].accelZ, accelZMin);
  }
  Serial.print("GyroXMax: ");
  Serial.print(gyroXMax);
  Serial.print(", GyroXMin: ");
  Serial.print(gyroXMin);
  Serial.print(", GyroYMax: ");
  Serial.print(gyroYMax);
  Serial.print(", GyroYMin: ");
  Serial.print(gyroYMin);
  Serial.print(", GyroZMax: ");
  Serial.print(gyroZMax);
  Serial.print(", GyroZMin: ");
  Serial.print(gyroZMin);
  Serial.println();
  
  stats.gyroXDiff = gyroXMax-gyroXMin;
  stats.gyroYDiff = gyroYMax-gyroYMin;
  stats.gyroZDiff = gyroZMax-gyroZMin;
  stats.accelXDiff = accelXMax-accelXMin;
  stats.accelYDiff = accelYMax-accelYMin;
  stats.accelZDiff = accelZMax-accelZMin;

}
