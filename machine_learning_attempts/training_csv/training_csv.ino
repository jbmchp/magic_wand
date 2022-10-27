/*
 * MPU 5060 gives 16-bit measurements
 * 
 */



#include <Adafruit_MPU6050.h> // MPU Calls
#include <Wire.h> // used for I2C


Adafruit_MPU6050 mpu; // MPU object

sensors_event_t a, g, temp;

#define Serial Serial3 // redirect serial lines

//#define DEBUG
int timestamp = 0;

void setup() 
{
  Wire.swap(&TWI1);
  Serial.begin(115200);
  while (!Serial)
    delay(10); // pause until serial console opens

  // Init MPU
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(1000);
      Serial.println("Failed to find MPU6050 chip");
    }
  } else {
    Serial.println("Initialized MPU6050");
  }
}

//  mpu.setAccelerometerRange(MPU6050_RANGE_4_G); // 2, 4, 8,16G options
//  // RAW to G = 4,096 bits
//  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
//  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ); // 94 Hz for filtering a lot of noise w/o being overpowering
//  delay(3000);
//  // Header for CSV file
//  Serial.print("AccelerometerX,AccelerometerY,AccelerometerZ,GyroscopeX,GyroscopeY,GyroscopeZ\r\n");


//  // Setup 10ms timer
//  takeOverTCA0(); // this replaces disabling and resettng the timer, required previously.
//  TCA0.SINGLE.CTRLB = 0b00000000;
//  TCA0.SINGLE.INTCTRL = 0b00000001; // Interrupt on Overflow
//  TCA0.SINGLE.PER = 0x2DB4; // 0x2DB4 = 500ms, 0x75 = 5ms, 0xEA = 10ms
//  TCA0.SINGLE.CTRLA = 0b0001111; // Enable the timer with 1024 prescalar
//
//}
//
//// Update row
//ISR(TCA0_OVF_vect) {
//  //update = true;
//
//    mpu.getEvent(&a, &g, &temp);
//
////    Serial.print(timestamp);
////    Serial.print(',');
//    Serial.print(a.acceleration.x);
//    Serial.print(',');
//    Serial.print(a.acceleration.y);
//    Serial.print(',');
//    Serial.print(a.acceleration.z);
//    Serial.print(',');
//    Serial.print(g.gyro.x);
//    Serial.print(',');
//    Serial.print(g.gyro.y);
//    Serial.print(',');
//    Serial.print(g.gyro.z);
//    Serial.println();
////    timestamp++;
//    
//    TCA0.SINGLE.INTFLAGS  = 0b00000001; // Clear the interrupt flags, otherwise the interrupt will fire continually
//}
//
//bool avail = false;

void loop()
{
  Serial.println("test");
}
//  if(Serial.available() == 1){
//    String t = Serial.readString();
//    if(t == "connect\n" ||  t == "connect" || t == "connect\r\n"){
//      avail = true;
//    }
//  }
//  if(update){
//    mpu.getEvent(&a, &g, &temp);
//    doc["column_location"]["AccelerometerX"] = a.acceleration.x;
//    doc["column_location"]["AccelerometerY"] = a.acceleration.y;
//    doc["column_location"]["AccelerometerZ"] = a.acceleration.z;
//    doc["column_location"]["GyroscopeX"] = g.gyro.x;
//    doc["column_location"]["GyroscopeY"] = g.gyro.y;
//    doc["column_location"]["GyroscopeZ"] = g.gyro.z;
//    serializeJson(doc, Serial3);  
//    Serial3.println();
//    update = false;
//  }
//
//}
