#include <Wire.h> // used for I2C
#include <MPU6050.h> // MPU calls

MPU6050 mpu;

#define Serial Serial3
#define RAW_TO_G 16384.0
#define G_TO_MS2 9.80665
#define RAW_TO_MS2 0.00059855

static bool writeData = false;
static uint16_t timestamp = 0;
static Vector rawAccel;

//#define DEBUG

void setup() 
{
  delay(5000);
  Serial.begin(115200);
  Wire.swapModule(&TWI1);

  // Init MPU
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_8G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  
  #ifdef DEBUG
  checkSettings();
  #endif

  // Set accelerometer offsets
//   mpu.setAccelOffsetX();
//   mpu.setAccelOffsetY();
//   mpu.setAccelOffsetZ();

  // Header for CSV file
  //Serial.print("timestamp,accX,accY,accZ\r\n");

  // Setup 10ms timer
  takeOverTCA0(); // this replaces disabling and resettng the timer, required previously.
  TCA0.SINGLE.CTRLB = 0b00000000;
  TCA0.SINGLE.INTCTRL = 0b00000001; // Interrupt on Overflow
  TCA0.SINGLE.PER = 0xEA; // 0x2DB4 = 500ms, 0x75 = 5ms, 0xEA = 10ms
  TCA0.SINGLE.CTRLA = 0b0001111; // Enable the timer with 1024 prescalar

}

// Update row
ISR(TCA0_OVF_vect) {
    timestamp++;
    rawAccel = mpu.readRawAccel();
    //Serial.print(timestamp*10);
    //Serial.print(',');
    Serial.print(rawAccel.XAxis*RAW_TO_MS2);
    Serial.print(',');
    Serial.print(rawAccel.YAxis*RAW_TO_MS2);
    Serial.print(',');
    Serial.print(rawAccel.ZAxis*RAW_TO_MS2);
    Serial.println();
    TCA0.SINGLE.INTFLAGS  = 0b00000001; // Always remember to clear the interrupt flags, otherwise the interrupt will fire continually!
}

void loop()
{
}

void checkSettings()
{
  Serial.println();
  
  Serial.print(" * Sleep Mode:            ");
  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");
  
  Serial.print(" * Clock Source:          ");
  switch(mpu.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
  }
  
  Serial.print(" * Accelerometer:         ");
  switch(mpu.getRange())
  {
    case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
    case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
    case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
    case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
  }  

  Serial.print(" * Accelerometer offsets: ");
  Serial.print(mpu.getAccelOffsetX());
  Serial.print(" / ");
  Serial.print(mpu.getAccelOffsetY());
  Serial.print(" / ");
  Serial.println(mpu.getAccelOffsetZ());
  
  Serial.println();
}
