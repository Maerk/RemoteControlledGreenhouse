/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GREENHOUSE
#define GREENHOUSE

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <vector>

using namespace std;

typedef struct _TimeTable 
{
    uint8_t day; // 0 mon, ...6 sun
    uint8_t h; //hours 0 to 23
      uint8_t m; //min 0 to 59 
}TimeTable; 

class Greenhouse
{
private:
    DHT_Unified* dht;
    
    /*Data*/
    vector<TimeTable> week_tt;
    bool irrigation_time; //true: i use the week time table
    bool light_state; //true: light is on
    bool fan_state; //true: fan is on
    bool valve_state; //true: the valve is open
    bool day_light; //true: there's the sun light
    uint8_t min_env_humidity; // %
    uint8_t max_env_humidity; 
    uint8_t min_ground_humidity; // %
    uint8_t max_ground_humidity; 
    float min_temperature; // C
    float max_temperature;
    //uint8_t water_alarm_level; // %
    uint8_t env_humidity; // %
    uint8_t ground_humidity; // %
    uint8_t water_level; // %
    float temperature; // C

public:
    Greenhouse();
    float getTemperature();
    float getMaxTemperature();
    float getMinTemperature(); 
    uint8_t getEnvHumidity();
    uint8_t getMaxEnvHumidity();
    uint8_t getMinEnvHumidity();  
    uint8_t getGroundHumidity();
    uint8_t getMaxGroundHumidity();
    uint8_t getMinGroundHumidity();   
    uint8_t getWaterLevel();
    //uint8_t getWaterAlarmLevel();
    bool getLightSensor();
    bool getLightState();
    bool getFanState();
    bool getValveState();
    vector<TimeTable> getWeekTimeTable();
    bool getIrrigationState()

    void setMaxTemperature(float temperature);
    void setMinTemperature(float temperature);
    void setMaxEnvHumidity(uint8_t humidity);
    void setMinEnvHumidity(uint8_t humidity);
    void setMaxGroundHumidity(uint8_t humidity);
    void setMinGroundHumidity(uint8_t humidity);
    void turnLight(bool state);
    void startFan(uint32_t seconds);
    void startIrrigation(uint32_t seconds);
    void addIrrigation(TimeTable tt);
    void removeIrrigation(uint32_t index);
    
    void updateData();
};

#endif
