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
#define MAX_SAMPLES_PER_PACKET 6
#define MAX_NUMBER_OF_COLUMNS 20
#define DEBUG
int timestamp = 0;

static int16_t sensorRawData[MAX_SAMPLES_PER_PACKET*MAX_NUMBER_OF_COLUMNS];
static int16_t* pData = &sensorRawData[0];
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
static int packetNum = 0;
static int sensorRawIndex = 0;

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
      // get new values

        // store in array
        
      sensorRawIndex = update_imu(sensorRawIndex);
      packetNum++;
      if (packetNum == MAX_SAMPLES_PER_PACKET) // after 6 samples, write the data
      {
          Serial.write((uint8_t*) pData, sensorRawIndex * sizeof(int16_t));
          Serial.flush();
          sensorRawIndex = 0;
          memset(pData, 0, MAX_NUMBER_OF_COLUMNS * MAX_SAMPLES_PER_PACKET * sizeof(int16_t));
          packetNum = 0;
      }
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
        serializeJson(doc, Serial);
        Serial.println();
        Serial.flush();
      }

    }
    update = false;

  }

}


int update_imu(int startIndex)
{
    int sensorRawIndex = startIndex;
        Vector rawAccel = mpu.readRawAccel();
        Vector rawGyro = mpu.readRawGyro();
        sensorRawData[sensorRawIndex++] = rawAccel.XAxis;
        sensorRawData[sensorRawIndex++] = rawAccel.YAxis;
        sensorRawData[sensorRawIndex++] = rawAccel.ZAxis;
        sensorRawData[sensorRawIndex++] = rawGyro.XAxis;
        sensorRawData[sensorRawIndex++] = rawGyro.YAxis;
        sensorRawData[sensorRawIndex++] = rawGyro.ZAxis;
    return sensorRawIndex;
}
