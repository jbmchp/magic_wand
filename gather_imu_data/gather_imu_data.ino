/*
 * Used to gather IMU data to export to a CSV file via putty
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

sensors_event_t a, g, temp; // throw away variable

#define Serial Serial3 // redirect serial lines

//#define DEBUG
uint16_t timestamp = 0;

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

// Every X ms
ISR(TCA0_OVF_vect) {
    // Collect data
    mpu.getEvent(&a, &g, &temp);
    // Print collected data
    Serial.print(timestamp*10);
    Serial.print(',');
    Serial.print(a.acceleration.x);
    Serial.print(',');
    Serial.print(a.acceleration.y);
    Serial.print(',');
    Serial.print(a.acceleration.z);
    Serial.print(',');
    Serial.print(timestamp*10);
    Serial.print(',');
    Serial.print(g.gyro.x);
    Serial.print(',');
    Serial.print(g.gyro.y);
    Serial.print(',');
    Serial.print(g.gyro.z);
    Serial.println();
    timestamp++;
    TCA0.SINGLE.INTFLAGS  = 0b00000001; // Clear the interrupt flags, otherwise the interrupt will fire continually
}

void loop()
{
}
