/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
* 
*        Copyright 2018 Marco De Nicolo
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCBoards.h>
#include <vector>
#include "Greenhouse.hpp"

using namespace std;

#define PORT 7400
WiFiUDP Udp;
const byte MAX_MSG_SIZE PROGMEM=200;
byte packetBuffer[MAX_MSG_SIZE];  //buffer to hold incoming udp packet
IPAddress timeServerIP; // time.nist.gov NTP server address
unsigned long timer;
const String ssid="router_ssid";
const String pass = "router_password";
Greenhouse* GH;

void wifiSetup()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
  
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
      Serial.println();
      Serial.print("Connected, IP address: ");
      Serial.println(WiFi.localIP());
  
      Udp.begin(PORT);
}

//https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/NTPClient/NTPClient.ino
// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, MAX_MSG_SIZE);
  // Initialize values needed to form NTP request
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, 48);
  Udp.endPacket();
}

void setup() 
{
    Serial.begin(115200);
    wifiSetup();
    GH = new Greenhouse();
    digitalWrite(LED_BUILTIN, HIGH); //turn off the led
    
    WiFi.hostByName("europe.pool.ntp.org", timeServerIP);
    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
}

void senderOSC(OSCMessage& msg)
{
    Serial.printf("send to: %s - %d\n",Udp.remoteIP().toString().c_str(),Udp.remotePort());
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
}

void sendOk(OSCMessage& msg, int addrOffset)
{
    Serial.println("sendOK");
    OSCMessage smsg("/ok");
    smsg.add(WiFi.localIP().toString().c_str());
    senderOSC(smsg); 
}

void sendTemperature(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/temperature");
    smsg.add((float)GH->getTemperature());
    senderOSC(smsg); 
}

void sendEnvHumidity(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/envHumidity");
    smsg.add((int)GH->getEnvHumidity());
    senderOSC(smsg);
}

void sendGroundHumidity(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/groundHumidity");
    smsg.add((int)GH->getGroundHumidity());
    senderOSC(smsg);
}

void sendWaterLevel(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/waterLevel");
    smsg.add((int)GH->getWaterLevel());
    senderOSC(smsg);
}

void sendLightSensor(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/lightSensor");
    smsg.add((int)GH->getLightSensor());
    senderOSC(smsg);
}

void sendWeekTimeTable(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/weekTimeTable");
    vector<TimeTable> tt = GH->getWeekTimeTable();
    for(int i=0; i<tt.size(); i++)
    {
        smsg.add(i);
        smsg.add((int)tt[i].day);
        smsg.add((int)tt[i].h);
        smsg.add((int)tt[i].m);
    }
    senderOSC(smsg);
}

void sendLightState(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/lightState");
    smsg.add((int)GH->getLightState());
    senderOSC(smsg);
}

void sendValveState(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/valveState");
    smsg.add((int)GH->getValveState());
    senderOSC(smsg);
}

void sendFanState(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/fanState");
    smsg.add((int)GH->getFanState());
    senderOSC(smsg);
}

void sendIrrigationState(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/irrigationState");
    smsg.add((int)GH->getIrrigationState());
    senderOSC(smsg);
}

void sendAutoLight(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/autoLight");
    smsg.add((int)GH->getAutoLight());
    senderOSC(smsg);
}

void sendSensors(OSCMessage& msg, int addrOffset)
{
    OSCBundle bund;
    bund.add("/temperature").add((float)GH->getTemperature());
    bund.add("/envHumidity").add((int)GH->getEnvHumidity());
    bund.add("/groundHumidity").add((int)GH->getGroundHumidity());
    bund.add("/waterLevel").add((int)GH->getWaterLevel());
    bund.add("/lightSensor").add((int)GH->getLightSensor());
    bund.add("/lightState").add((int)GH->getLightState());
    bund.add("/valveState").add((int)GH->getValveState());
    bund.add("/fanState").add((int)GH->getFanState());
    bund.add("/irrigationState").add((int)GH->getIrrigationState());
    bund.add("/autoLight").add((int)GH->getAutoLight());
    bund.add("/maxTemperature").add((float)GH->getMaxTemperature());
    bund.add("/minTemperature").add((float)GH->getMinTemperature());
    bund.add("/maxEnvHumidity").add((int)GH->getMaxEnvHumidity());
    bund.add("/minEnvHumidity").add((int)GH->getMinEnvHumidity());
    bund.add("/maxGroundHumidity").add((int)GH->getMaxGroundHumidity());
    bund.add("/minGroundHumidity").add((int)GH->getMinGroundHumidity());
    OSCMessage& msgs = bund.add("/weekTimeTable");
    vector<TimeTable> tt = GH->getWeekTimeTable();
    for(int i=0; i<tt.size(); i++)
    {
        msgs.add(i);
        msgs.add((int)tt[i].day);
        msgs.add((int)tt[i].h);
        msgs.add((int)tt[i].m);
    }
    
    
    Serial.printf("send bundle to: %s - %d\n",Udp.remoteIP().toString().c_str(),Udp.remotePort());
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    bund.send(Udp);
    Udp.endPacket();
    bund.empty();
}

void sendMaxTemperature(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/maxTemperature");
    smsg.add((float)GH->getMaxTemperature());
    senderOSC(smsg);
}

void sendMinTemperature(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/minTemperature");
    smsg.add((float)GH->getMinTemperature());
    senderOSC(smsg);
}

void sendMaxEnvHumidity(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/maxEnvHumidity");
    smsg.add((int)GH->getMaxEnvHumidity());
    senderOSC(smsg);
}

void sendMinEnvHumidity(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/minEnvHumidity");
    smsg.add((int)GH->getMinEnvHumidity());
    senderOSC(smsg);
}

void sendMaxGroundHumidity(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/maxGroundHumidity");
    smsg.add((int)GH->getMaxGroundHumidity());
    senderOSC(smsg);
}

void sendMinGroundHumidity(OSCMessage& msg, int addrOffset)
{
    OSCMessage smsg("/minGroundHumidity");
    smsg.add((int)GH->getMinGroundHumidity());
    senderOSC(smsg);
}

void setMaxTemperature(OSCMessage& msg, int addrOffset) 
{
    GH->setMaxTemperature(msg.getFloat(0));
}

void setMinTemperature(OSCMessage& msg, int addrOffset) 
{
    GH->setMinTemperature(msg.getFloat(0));
}

void setMaxEnvHumidity(OSCMessage& msg, int addrOffset) 
{
    GH->setMaxEnvHumidity(msg.getInt(0));
}

void setMinEnvHumidity(OSCMessage& msg, int addrOffset) 
{
    GH->setMinEnvHumidity(msg.getInt(0));
}

void setMaxGroundHumidity(OSCMessage& msg, int addrOffset) 
{
    GH->setMaxGroundHumidity(msg.getInt(0));
}

void setMinGroundHumidity(OSCMessage& msg, int addrOffset) 
{
    GH->setMinGroundHumidity(msg.getInt(0));
}

void setLight(OSCMessage& msg, int addrOffset) 
{
    GH->turnLight(msg.getInt(0));
}

void setAutoLight(OSCMessage& msg, int addrOffset)
{
    GH->setAutoLight(msg.getInt(0));
}

void setIrrigationTime(OSCMessage& msg, int addrOffset)
{
    GH->setIrrigationState(msg.getInt(0));
}

void addIrrigationTime(OSCMessage& msg, int addrOffset) 
{
    TimeTable tt;
    int t[3];
    for(int i = 0; i<3; i++)
        t[i] = msg.getInt(i);
    tt.day = t[0];
    tt.h = t[1];
    tt.m = t[2];
    GH->addIrrigation(tt);
}

void removeIrrigationTime(OSCMessage& msg, int addrOffset) 
{
    GH->removeIrrigation(msg.getInt(0));
}

void startIrrigation(OSCMessage& msg, int addrOffset)
{
    GH->startIrrigation(msg.getInt(0));
}

void startFan(OSCMessage& msg, int addrOffset)
{
    GH->startFan(msg.getInt(0));
}

void receiverOSC() 
{
    //OSCBundle bundleIN; // NO
    OSCMessage messageIN;
    int size;
    if( (size = Udp.parsePacket())>0) 
    {
        Serial.println("received");
        Serial.printf("Received %d bytes from %s, port %d\n", size, Udp.remoteIP().toString().c_str(), Udp.remotePort());
        Udp.read(packetBuffer,size);
        Serial.printf("%s\n",packetBuffer);
        
        if(timeServerIP == Udp.remoteIP())
        {
            unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
            unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
            timer = (highWord << 16 | lowWord) - 2208988800UL;
            Serial.printf("epoch: %lu\n",timer);
            GH->setTime(timer);
        }
        else
        {
            messageIN.fill(packetBuffer,size);
            if(!messageIN.hasError()) 
            {
                messageIN.route("/oscPing", sendOk);
                messageIN.route("/getTemperature", sendTemperature);
                messageIN.route("/getEnvHumidity", sendEnvHumidity);
                messageIN.route("/getGroundHumidity", sendGroundHumidity);
                messageIN.route("/getWaterLevel", sendWaterLevel);
                messageIN.route("/getLightSensor", sendLightSensor); //sensor return 1(day) or 0(night)
                messageIN.route("/getWeekTimeTable", sendWeekTimeTable); //return week time table with indexes
                messageIN.route("/getLightState", sendLightState);
                messageIN.route("/getValveState", sendValveState);
                messageIN.route("/getFanState", sendFanState);
                messageIN.route("/getIrrigationState", sendIrrigationState);
                messageIN.route("/getAutoLight", sendAutoLight);
                messageIN.route("/getSensors", sendSensors);
    
                messageIN.route("/getMaxTemperature", sendMaxTemperature);
                messageIN.route("/getMinTemperature", sendMinTemperature);
                messageIN.route("/getMaxEnvHumidity", sendMaxEnvHumidity);
                messageIN.route("/getMinEnvHumidity", sendMinEnvHumidity);
                messageIN.route("/getMaxGroundHumidity", sendMaxGroundHumidity);
                messageIN.route("/getMinGroundHumidity", sendMinGroundHumidity);
                
                messageIN.route("/setMaxTemperature", setMaxTemperature);  //maximum temperature before starting fan
                messageIN.route("/setMinTemperature", setMinTemperature); //minimum temperature before stopping fan (if is running) and turn on light (?)
                messageIN.route("/setMaxEnvHumidity", setMaxEnvHumidity);  //maximum humidity before starting fan
                messageIN.route("/setMinEnvHumidity", setMinEnvHumidity);  //minimum temperature before stopping fan (if is running)
                messageIN.route("/setMaxGroundHumidity", setMaxGroundHumidity); //maximum ground humidity, check if the valve is open and close it
                messageIN.route("/setMinGroundHumidity", setMinGroundHumidity); //minimum ground humidity before starting irrigation
                //messageIN.route("/setAlarmWaterLevel", setAlarmWaterLevel); //alarm water level (?)
    
                messageIN.route("/addIrrigationTime", addIrrigationTime);  //it's the time of the waterings, parameters:[(WeekDay,Hour,Min), ...]
                messageIN.route("/removeIrrigationTime", removeIrrigationTime);  //remove a time of the waterings, parameter: index of the week time table
                messageIN.route("/autoIrrigation", setIrrigationTime);  //if 1 set irrigation_time = true else false
                messageIN.route("/autoLight", setAutoLight); //if 1 set auto_light true else false
                
                messageIN.route("/startIrrigation", startIrrigation);  //open the valve for T seconds
                messageIN.route("/startFan", startFan);  //start fan for T seconds
                messageIN.route("/light", setLight);  //if 1 turn on  the light else turn off
            }
            else 
                Serial.println("Error: " + messageIN.getError());
        }
        Udp.flush();
    }
}

void loop() 
{
    GH->updateData();
    receiverOSC();
}
