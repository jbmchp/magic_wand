/*
   MPU 5060 gives 16-bit measurements

*/



#include <MPU6050.h> // MPU Calls
#include <Wire.h> // used for I2C
#include <ArduinoJson.h>

DynamicJsonDocument doc(256); // calculate size using https://arduinojson.org/v6/assistant/
//StaticJsonDocument<96> doc;
//JsonObject col = doc.createNestedObject("col");


MPU6050 mpu; // MPU object

volatile bool update = false;

#define Serial Serial3 // redirect serial lines

#define DEBUG
int timestamp = 0;

void setup()
{
  Wire.swapModule(&TWI1);
  Serial.begin(115200);
  while (!Serial)
    delay(10); // pause until serial console opens

  // Init MPU
  while(!mpu.begin(MPU6050_SCALE_500DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  //mpu.setAccelerometerRange(MPU6050_RANGE_4_G); // 2, 4, 8,16G options
  // RAW to G = 4,096 bits
  //mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  //mpu.setFilterBandwidth(MPU6050_BAND_94_HZ); // 94 Hz for filtering a lot of noise w/o being overpowering

  // Header for CSV file
  //Serial.print("sequence,AccelerometerX,AccelerometerY,AccelerometerZ,GyroscopeX,GyroscopeY,GyroscopeZ\r\n");

  // Set static values for JSON object
  doc["version"] = 1;
  doc["sample_rate"] = 2;
  doc["samples_per_packet"] = 6;
  //  delay(5000);
  //      mpu.getEvent(&a, &g, &temp);
  //    doc["column_location"]["AccelerometerX"] = a.acceleration.x;
  //    doc["column_location"]["AccelerometerY"] = a.acceleration.y;
  //    doc["column_location"]["AccelerometerZ"] = a.acceleration.z;
  //    doc["column_location"]["GyroscopeX"] = g.gyro.x;
  //    doc["column_location"]["GyroscopeY"] = g.gyro.y;
  //    doc["column_location"]["GyroscopeZ"] = g.gyro.z;
  //    serializeJson(doc, Serial3);
  //    Serial3.println();
  // Setup 10ms timer
  takeOverTCA0(); // this replaces disabling and resettng the timer, required previously.
  TCA0.SINGLE.CTRLB = 0b00000000;
  TCA0.SINGLE.INTCTRL = 0b00000001; // Interrupt on Overflow
  TCA0.SINGLE.PER = 0x2DB4; // 0x2DB4 = 500ms, 0x75 = 5ms, 0xEA = 10ms
  TCA0.SINGLE.CTRLA = 0b0001111; // Enable the timer with 1024 prescalar

}

// Update row
ISR(TCA0_OVF_vect) {
  update = true;
  TCA0.SINGLE.INTFLAGS  = 0b00000001; // Clear the interrupt flags, otherwise the interrupt will fire continually
}

bool avail = false;

void loop()
{
        static int count = 0;

  if (Serial.available() == 1) {
    String t = Serial.readString();
    if (t == "connect\n" ||  t == "connect" || t == "connect\r\n") {
      avail = true;
    } else if (t == "disconnect\n" ||  t == "disconnect" || t == "disconnect\r\n") {
      avail = false;
    }
  }
  
  if (update){
    if(avail){
      Vector rawAccel = mpu.readRawAccel();
      Vector rawGyro = mpu.readRawGyro();
      doc["column_location"]["AccelerometerX"] = (int16_t)rawAccel.XAxis;
      doc["column_location"]["AccelerometerY"] = (int16_t)rawAccel.YAxis;
      doc["column_location"]["AccelerometerZ"] = (int16_t)rawAccel.ZAxis;
      doc["column_location"]["GyroscopeX"] = (int16_t)rawGyro.XAxis;
      doc["column_location"]["GyroscopeY"] = (int16_t)rawGyro.YAxis;
      doc["column_location"]["GyroscopeZ"] = (int16_t)rawGyro.ZAxis;
              serializeJson(doc, Serial3);
        Serial3.println();
    }
    else {
      count++;
        if(count == 1){
        doc["column_location"]["AccelerometerX"] = 0;
        doc["column_location"]["AccelerometerY"] = 1;
        doc["column_location"]["AccelerometerZ"] = 2;
        doc["column_location"]["GyroscopeX"] = 3;
        doc["column_location"]["GyroscopeY"] = 4;
        doc["column_location"]["GyroscopeZ"] = 5;
        count = 0;
        serializeJson(doc, Serial3);
        Serial3.println();
      }

    }
    update = false;

  }

}
