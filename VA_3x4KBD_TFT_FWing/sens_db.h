#ifndef __SENS_DB_H__
#define __SENS_DB_H__

#define NBR_COLLECTED_SENSORS 3


typedef struct{
    String name;
    String zone;
    String sensor;
    float value;  
} sensor_entry_st;

#endif
