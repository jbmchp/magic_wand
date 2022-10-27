#include <Arduino.h>
#include <MPU6050.h> // MPU Calls
#include <Wire.h> // used for I2C
#include <ArduinoJson.h>

#define MAX_NUMBER_OF_COLUMNS 20
#define MAX_SAMPLES_PER_PACKET 6

const int WRITE_BUFFER_SIZE = 256;
static int8_t ble_output_buffer[WRITE_BUFFER_SIZE];

extern int actual_odr;

static bool          config_received = false;
static unsigned long currentMs, previousMs;
static long          interval = 0;
extern volatile int  samplesRead;

DynamicJsonDocument config_message(256);
#define Serial Serial3 // redirect serial lines
#define SERIAL_BAUD_RATE 115200
MPU6050 mpu; // MPU object
int16_t sensorRawData[MAX_SAMPLES_PER_PACKET*MAX_NUMBER_OF_COLUMNS];

int column_index = 0;

static void sendJsonConfig()
{
    serializeJson(config_message, (void *)ble_output_buffer, WRITE_BUFFER_SIZE);
    Serial.println((char*)ble_output_buffer);
    Serial.flush();
}

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    delay(2000);
    Serial.println("Setting up...");

    column_index += setup_imu(config_message, column_index);
    interval = (1000 / (long) actual_odr);
    config_message["samples_per_packet"] = MAX_SAMPLES_PER_PACKET;

    delay(1000);
    sendJsonConfig();
}


static int packetNum      = 0;
static int sensorRawIndex = 0;
void       loop()
{
    currentMs = millis();
    if (!config_received)
    {
        sendJsonConfig();
        delay(1000);
        if (Serial.available() > 0)
        {
            String rx = Serial.readString();
            Serial.println(rx);
            if (rx.equals("connect") || rx.equals("cnnect"))
            {
                #if USE_SECOND_SERIAL_PORT_FOR_OUTPUT
                Serial.println("Got Connect message");

                #endif
                config_received = true;
            }
        }
    }

    else
    {
        if (Serial.available() > 0)
        {
            String rx = Serial.readString();
            if( rx.equals("disconnect"))
            {
                config_received = false;
            }
        }
        if (currentMs - previousMs >= interval)
        {
            // save the last time you blinked the LED
            previousMs = currentMs;
            sensorRawIndex = update_imu(sensorRawIndex);
            packetNum++;
            int16_t* pData = get_imu_pointer();
            if (packetNum == MAX_SAMPLES_PER_PACKET)
            {
                Serial.write((uint8_t*) pData, sensorRawIndex * sizeof(int16_t));
                Serial.flush();
                sensorRawIndex = 0;
                memset(pData, 0, MAX_NUMBER_OF_COLUMNS * MAX_SAMPLES_PER_PACKET * sizeof(int16_t));
                packetNum = 0;
            }
        }
    }
} //loop()

int16_t* get_imu_pointer()
{
    return &sensorRawData[0];
}


int setup_imu(JsonDocument& config_message, int column_start)
{
    int column_index = column_start;
    if (!mpu.begin(MPU6050_SCALE_500DPS, MPU6050_RANGE_2G))  // Initialize IMU sensor
    {
        Serial.println("Failed to initialize IMU!");
        while (1)
            ;
    }
    config_message["sample_rate"]                       = 2;
    config_message["column_location"]["AccelerometerX"] = column_index++;
    config_message["column_location"]["AccelerometerY"] = column_index++;
    config_message["column_location"]["AccelerometerZ"] = column_index++;
    config_message["column_location"]["GyroscopeX"]     = column_index++;
    config_message["column_location"]["GyroscopeY"]     = column_index++;
    config_message["column_location"]["GyroscopeZ"]     = column_index++;
    return column_index;
}


int update_imu(int startIndex)
{
    int sensorRawIndex = startIndex;
    Vector rawAccel = mpu.readRawAccel();
    Vector rawGyro = mpu.readRawGyro();
    sensorRawData[sensorRawIndex++] = (int16_t)100;
    sensorRawData[sensorRawIndex++] = (int16_t)200;
    sensorRawData[sensorRawIndex++] = (int16_t)300;
    sensorRawData[sensorRawIndex++] = (int16_t)400;
    sensorRawData[sensorRawIndex++] = (int16_t)500;
    sensorRawData[sensorRawIndex++] = (int16_t)600;
    return sensorRawIndex;
}
