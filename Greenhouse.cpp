/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
* 
*        Copyright 2018 Marco De Nicolo
*/

#include "Greenhouse.hpp"

Greenhouse::Greenhouse(int32_t dht_pin, int32_t waterlevel_pin, int32_t moisture_pin, int32_t photoresistor_pin, int32_t fan_pin, int32_t valve_pin, int32_t light_pin)
{
    this->dht_pin = dht_pin;
    this->waterlevel_pin = waterlevel_pin;
    this->moisture_pin = moisture_pin;
    this->photoresistor_pin = photoresistor_pin;
    this->fan_pin = fan_pin;
    this->valve_pin = valve_pin;
    this->light_pin = light_pin;
    dht = new DHT_Unified(dht_pin, DHT11); //pin,type
    pinMode(waterlevel_pin, OUTPUT);
    pinMode(moisture_pin, OUTPUT);
    pinMode(photoresistor_pin, INPUT);
    pinMode(fan_pin, OUTPUT);
    digitalWrite(fan_pin, HIGH);
    pinMode(valve_pin, OUTPUT);
    digitalWrite(valve_pin, HIGH);
    pinMode(light_pin, OUTPUT);
    pinMode(A0,INPUT);
    dht->begin();
    
    irrigation_time = false;
    light_state = false;
    fan_state = false;
    valve_state = false;
    day_light = false;
    min_env_humidity = 20; // %
    max_env_humidity = 85; 
    min_ground_humidity = 1; // [0-4]
    max_ground_humidity = 3; 
    min_temperature = 10.0; // C
    max_temperature = 47.0;
    //water_alarm_level = 1; // [0-4]
    
    env_humidity = 50; // %
    ground_humidity = 2; // [0-4]
    water_level = 2; // [0-4]
    temperature = 23.0; // C
}
float Greenhouse::getTemperature()
{
    return temperature;
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
    return env_humidity;
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
  return ground_humidity;
}
uint8_t Greenhouse::getMaxGroundHumidity()
{
    return max_ground_humidity;
}
uint8_t Greenhouse::getMinGroundHumidity()
{
    return min_ground_humidity;
}
uint8_t Greenhouse::getWaterLevel()
{
    return water_level;
}
//uint8_t Greenhouse::getWaterAlarmLevel();
bool Greenhouse::getLightSensor()
{
    return day_light;
}
bool Greenhouse::getLightState()
{
    return light_state;
}
bool Greenhouse::getFanState()
{
    return fan_state;
}
bool Greenhouse::getValveState()
{
    return valve_state;
}
vector<TimeTable> Greenhouse::getWeekTimeTable()
{
    return week_tt;
}
bool Greenhouse::getIrrigationState()
{
    return irrigation_time;
}
void Greenhouse::setMaxTemperature(float temperature)
{
    max_temperature = temperature;
}
void Greenhouse::setMinTemperature(float temperature)
{
    min_temperature = temperature;
}
void Greenhouse::setMaxEnvHumidity(uint8_t humidity)
{
    max_env_humidity = humidity;
}
void Greenhouse::setMinEnvHumidity(uint8_t humidity)
{
    min_env_humidity = humidity;
}
void Greenhouse::setMaxGroundHumidity(uint8_t humidity)
{
    max_ground_humidity = humidity;
}
void Greenhouse::setMinGroundHumidity(uint8_t humidity)
{
    min_ground_humidity = humidity;
}
void Greenhouse::setIrrigationState(bool state)
{
    irrigation_time = state;
}
void Greenhouse::turnLight(bool state)
{
    /*
    * turn light .........................
    */
    if(state)
        digitalWrite(light_pin, HIGH);
    else
        digitalWrite(light_pin, LOW);
    light_state = state;
}
void Greenhouse::startFan(uint64_t seconds)
{
    fan_t = millis();
    fan_sec = seconds*1000;
    fan_state = true;
    digitalWrite(fan_pin,LOW); //lowtrigger
}
void Greenhouse::startIrrigation(uint64_t seconds)
{
    valve_t = millis();
    valve_sec = seconds*1000;
    valve_state = true;
    digitalWrite(valve_pin,LOW);
}
void Greenhouse::addIrrigation(TimeTable tt)
{
    if(week_tt.size() < MAX_TT_SIZE && tt.day>=0 && tt.day<=6 && tt.h>=0 && tt.h<=23 && tt.m>=0 && tt.m<=59)
        week_tt.push_back(tt);
}
void Greenhouse::removeIrrigation(uint32_t index)
{
    if(index >= 0 && index < week_tt.size())
        week_tt.erase(week_tt.begin()+index);
}
void Greenhouse::updateData()
{
    /*
     * read sensors data
     */
     
    day_light = !digitalRead(photoresistor_pin);
    
    sensors_event_t event;  
    dht->temperature().getEvent(&event);
    if (isnan(event.temperature)) 
       Serial.println("Error reading temperature!");
    else
        temperature = event.temperature;
    dht->humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) 
        Serial.println("Error reading humidity!");
    else
        env_humidity = event.relative_humidity;
        
    digitalWrite(moisture_pin, HIGH);
    delay(1500);
    int reada = analogRead(A0);
    if(reada<12)
        ground_humidity = 0; //very low
    else if(reada<46)
        ground_humidity = 1; //low
    else if(reada<90)
        ground_humidity = 2; //medium
    else
        ground_humidity = 3; //high
    digitalWrite(moisture_pin, LOW);
    
    delay(200);
    
    digitalWrite(waterlevel_pin, HIGH);
    delay(1500);
    reada = analogRead(A0);
    if(reada<12)
        water_level = 0; //empty
    else if(reada<40)
        water_level = 1; //low
    else if(reada<50)
        water_level = 2; //medium
    else
        water_level = 3; //high
    digitalWrite(waterlevel_pin, LOW);

    //Fan timer control and set fan_state=false
    if(fan_state)
    {
        uint64_t now = millis();
        if((now-fan_t)>=fan_sec)
        {
            fan_state = false;
            digitalWrite(fan_pin,HIGH);
        }
    }

    //Valve timer control and set valve_state=false
    if(valve_state)
    {
        uint64_t now = millis();
        if((now-valve_t)>=valve_sec)
        {
            valve_state = false;
            digitalWrite(valve_pin,HIGH);
        }
    }

    /*Serial.println("Moisture Sensor: " + String(ground_humidity));
    Serial.println("Water Level Sensor: " + String(water_level));
    Serial.println("Photoresistor Sensor: " + String(day_light));
    Serial.println("Temperature Sensor: " + String(temperature));
    Serial.println("Humidity Sensor: " + String(env_humidity));*/
}

