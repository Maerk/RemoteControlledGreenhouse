/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
const byte MAX_MSG_SIZE PROGMEM=100;
byte packetBuffer[MAX_MSG_SIZE];  //buffer to hold incoming udp packet
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

void setup() 
{
    Serial.begin(115200);
    wifiSetup();
    GH = new Greenhouse();
    digitalWrite(LED_BUILTIN, HIGH); //turn off the led
}

void senderOSC(OSCMessage& msg)
{
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
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

/*void sendWeekTimeTable(OSCMessage& msg, int addrOffset)
{
    OSCMessage* msg("/weekTimeTable");
    vector<TimeTable> tt = GH->getWeekTimeTable;
    msg.add((int)GH->getLightSensor());
    senderOSC(msg);
}*/

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
    if(t[0]>=0 && t[0]<=6 && t[1]>=0 && t[1]<=23 && t[2]>=0 && t[2]<=59)
    {
        tt.day = t[0];
        tt.h = t[1];
        tt.m = t[2];
        GH->addIrrigation(tt);
    }
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
        messageIN.fill(packetBuffer,size);
        if(!messageIN.hasError()) 
        {
            messageIN.route("/getTemperature", sendTemperature);
            messageIN.route("/getEnvHumidity", sendEnvHumidity);
            messageIN.route("/getGroundHumidity", sendGroundHumidity);
            messageIN.route("/getWaterLevel", sendWaterLevel);
            messageIN.route("/getLightSensor", sendLightSensor); //the sensor return 1 or 0
            //messageIN.route("/getWeekTimeTable", sendWeekTimeTable) //return week_tt
            
            messageIN.route("/setMaxTemperature", setMaxTemperature);  //temperatura massima prima di avviare ventola
            messageIN.route("/setMinTemperature", setMinTemperature); //temperatura minima prima di accendere luce (?)
            messageIN.route("/setMaxEnvHumidity", setMaxEnvHumidity);  //umidità massima prima di accendere ventola
            messageIN.route("/setMinEnvHumidity", setMinEnvHumidity);  //entra in allarme e controlla che la ventola sia spenta
            messageIN.route("/setMaxGroundHumidity", setMaxGroundHumidity); //livello massimo umidità terreno, entra in allarme e controlla che sia chiusa l'acqua
            messageIN.route("/setMinGroundHumidity", setMinGroundHumidity); //livello minimo prima di far partire irrigazione
            //messageIN.route("/setAlarmWaterLevel", setAlarmWaterLevel); //livello allarme acqua (?)

            messageIN.route("/addIrrigationTime", addIrrigationTime);  //ho un orario di irrigazione che passo come parametro [(WeekDay,Hour,Min), ...]
            messageIN.route("/removeIrrigationTime", removeIrrigationTime);  //rimuovo l'innaffiatura all'indice X di week_tt
            messageIN.route("/autoIrrigation", setIrrigationTime);  //setto irrigation_time = true con 1 o false con 0
            
            messageIN.route("/startIrrigation", startIrrigation);  //apre l'acqua per T secondi
            messageIN.route("/startFan", startFan);  //parte ventola per T secondi
            messageIN.route("/light", setLight);  //accende luce con 1 e spegne con 0
        }
        else 
            Serial.println("Error: " + messageIN.getError());
        Udp.flush();
    }
}

void loop() 
{
    GH->updateData();
    receiverOSC();
}
