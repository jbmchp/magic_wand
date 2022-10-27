//#include <c70636-project-1_inferencing.h>
#include <Adafruit_MPU6050.h> // MPU Calls
#include <Wire.h> // used for I2C

Adafruit_MPU6050 mpu;

#define Serial Serial3

//#define DEBUG
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal

void setup() 
{
  Wire.swapModule(&TWI1);
  Serial.begin(115200);
  while (!Serial)
    delay(10); // pause until serial console opens
    
  // Init MPU
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  } else {
    Serial.println("IMU initialized\r\n");
  }
//
//  if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 6) {
//    ei_printf("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME should be equal to 6 (the 6 sensor axes)\n");
//    return;
//  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G); // 2, 4, 8,16G options
  // RAW to G = 4,096 bits
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ); // 94 Hz for filtering a lot of noise w/o being overpowering

  // Header for CSV file
  //Serial.print("timestamp,accX,accY,accZ\r\n");
  delay(5000);
  // Setup 10ms timer
  takeOverTCA0(); // this replaces disabling and resettng the timer, required previously.
  TCA0.SINGLE.CTRLB = 0b00000000;
  TCA0.SINGLE.INTCTRL = 0b00000001; // Interrupt on Overflow
  TCA0.SINGLE.PER = 0xEA; // 0x2DB4 = 500ms, 0x75 = 5ms, 0xEA = 10ms
  TCA0.SINGLE.CTRLA = 0b0001111; // Enable the timer with 1024 prescalar

}

sensors_event_t a, g, temp;

// Update row
ISR(TCA0_OVF_vect) {
    //timestamp++;
    mpu.getEvent(&a, &g, &temp);

    //Serial.print(timestamp*10);
    //Serial.print(',');
    Serial.print(a.acceleration.x);
    Serial.print(',');
    Serial.print(a.acceleration.y);
    Serial.print(',');
    Serial.print(a.acceleration.z);
    Serial.print(',');
    Serial.print(g.gyro.x);
    Serial.print(',');
    Serial.print(g.gyro.y);
    Serial.print(',');
    Serial.print(g.gyro.z);
    Serial.println();
    
    TCA0.SINGLE.INTFLAGS  = 0b00000001; // Clear the interrupt flags, otherwise the interrupt will fire continually
}

void loop()
{
}
