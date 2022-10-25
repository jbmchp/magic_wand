#include <Adafruit_MPU6050.h>

#define TIME_LENGTH_MS 1500
#define COLLECTION_FREQ_MS 10

#define NUM_COLLECTIONS TIME_LENGTH_MS/COLLECTION_FREQ_MS


typedef struct{
    sensors_event_t a;
    sensors_event_t g;
    sensors_event_t temp;
} imu_data_t;