#ifndef __SENS_DB_H__
#define __SENS_DB_H__

#define NBR_COLLECTED_SENSORS 3


typedef struct{
    String name;
    String zone;
    String sensor;
    float value;  
    String unit;
    uint8_t nbr_decimals;
} sensor_entry_st;

#endif
