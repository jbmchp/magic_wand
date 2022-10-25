#include "processing.h"


imu_data_t data_window[(TIME_LENGTH_MS/COLLECTION_FREQ_MS)+1];


bool fill_data_window(imu_data_t data){
    static uint8_t filled = 0;
    if(filled < NUM_COLLECTIONS){
        data_window[filled] = data;
        filled++;
        return false;
    } 

    if(filled >= NUM_COLLECTIONS){
        filled = 0;
        return true;
    }

}