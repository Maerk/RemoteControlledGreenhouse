/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Greenhouse.h"

Greenhouse::Greenhouse()
{
    dht = new DHT_Unified(D2, DHT11); //pin,type
    irrigation_time = false;
    light_state = false;
    fan_state = false;
    water_state = false;
    day_light = false;
    min_env_humidity = 20; // %
    max_env_humidity = 85; 
    min_ground_humidity = 20; // %
    max_ground_humidity = 70; 
    min_temperature = 10.0; // C
    max_temperature = 47.0;
    water_alarm_level = 10; // %
    
    env_humidity = 50; // %
    ground_humidity = 50; // %
    water_level = 50; // %
    temperature = 23.0; // C
    
    pinMode(A0,INPUT);
    dht->begin();
}
float Greenhouse::getTemperature()
{
    sensors_event_t event;  
    dht->temperature().getEvent(&event);
    if (isnan(event.temperature)) 
    {
       Serial.println("Error reading temperature!");
       return -999.0;
    }
    return event.temperature;
}
float Greenhouse::getMaxTemperature()
{
    return max_temperature;
}
float Greenhouse::getMinTemperature()
{
    return min_temperature;
}
uint8_t Greenhouse::getEnvHumidity()
{
    sensors_event_t event;
    dht->humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) 
    {
        Serial.println("Error reading humidity!");
        return 120;
    }
    return event.relative_humidity;
}
uint8_t Greenhouse::getMaxEnvHumidity()
{
    return max_env_humidity;
}
uint8_t Greenhouse::getMinEnvHumidity()
{
    return min_env_humidity;
}
uint8_t Greenhouse::getGroundHumidity()
{
  return 42;
    //return analogRead(A0);
}
uint8_t Greenhouse::getMaxGroundHumidity()
{
    return max_ground_humidity;
}
uint8_t Greenhouse::getMinGroundHumidity()
{
    return max_ground_humidity;
}
uint8_t getWaterLevel()
{
    return water_level;
}
//uint8_t getWaterAlarmLevel();
bool getLightSensor()
{
    return day_light;
}
bool getLightState()
{
    return light_state;
}
bool getFanState()
{
    return fan_state;
}
bool getValveState()
{
    return valve_state;
}
vector<TimeTable> getWeekTimeTable()
{
    return week_tt;
}
bool getIrrigationState()
{
    return irrigation_time;
}
void setMaxTemperature(float temperature)
{
    max_temperature = temperature;
}
void setMinTemperature(float temperature)
{
    min_temperature = temperature;
}
void setMaxEnvHumidity(uint8_t humidity)
{
    max_env_humidity = humidity;
}
void setMinEnvHumidity(uint8_t humidity)
{
    min_env_humidity = humidity;
}
void setMaxGroundHumidity(uint8_t humidity)
{
    max_ground_humidity = humidity;
}
void setMinGroundHumidity(uint8_t humidity)
{
    min_ground_humidity = humidity;
}
void turnLight(bool state)
{
    /*
    * turn light .........................
    */
    light_state = state;
}
void startFan(uint32_t seconds)
{
    fan_state = true;
    /*
     * run fan ...........................
     */
    fan_state = false;
}
void startIrrigation(uint32_t seconds)
{
    valve_state = true;
    /*
     * open valve ........................
     */
    valve_state = false;
}
void addIrrigation(TimeTable tt)
{
    week_tt.push_back(tt);
}
void removeIrrigation(uint32_t index)
{
    if(index >= 0 && index < week_tt.size())
        week_tt.erase(week_tt.begin()+index);
}
void updateData()
{
    /*
     * read sensors data ..................
     */
}

