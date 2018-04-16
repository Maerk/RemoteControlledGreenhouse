/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCBoards.h>
#include "Greenhouse.h"

WiFiUDP Udp;
const byte MAX_MSG_SIZE PROGMEM=100;
byte packetBuffer[MAX_MSG_SIZE];  //buffer to hold incoming udp packet
#define PORT 7400
const String ssid="Vodafone2.4GHz-Deny";
const String pass = "Semola98";
const IPAddress outIp(192,168,1,7);
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
}

void senderOSC(String text)
{
    OSCMessage msg(text.c_str());
    //msg.add("\n");
    Udp.beginPacket(outIp, 7400);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
}

void sendTemperature(OSCMessage& msg)
{
    senderOSC(String("/temperature/" + String(GH->getTemperature()))); 
}

void sendEnvHumidity(OSCMessage& msg, int addrOffset)
{
    senderOSC(String("/humidity/" + String(GH->getEnvHumidity())));
}

void sendGroundHumidity(OSCMessage& msg, int addrOffset)
{
    senderOSC(String("/humidity/" + String(GH->getGroundHumidity())));
}

void sendWaterLevel(OSCMessage& msg, int addrOffset)
{
    senderOSC(String("/waterLevel/" + String(GH->getWaterLevel())));
}

void sendLightSensor(OSCMessage& msg, int addrOffset)
{
    senderOSC(String("/lightSensor/" + String(GH->getLightSensor())));
}
/*
void sendWeekTimeTable(OSCMessage& msg, int addrOffset)
{
    senderOSC(String("/weekTimeTable/" + String(GH->getWeekTimeTable())));
}
*/
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

void setLight(OSCMessage& msg) 
{
    GH->turnLight(msg.getInt(0));
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
            messageIN.dispatch("/getTemperature", sendTemperature);
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

            //messageIN.route("/addIrrigationTime", __);  //ho un orario di irrigazione che passo come parametro [(WeekDay,Hour,Min), ...]
            //messageIN.route("/removeIrrigationTime", __);  //rimuovo l'innaffiatura all'indice X di week_tt
            //messageIN.dispatch("/autoIrrigation/1", __);  //setto irrigation_time = true 
            //messageIN.dispatch("/autoIrrigation/0", __); //irrigation_time = false
            
            //messageIN.route("/startIrrigation", __);  //apre l'acqua per T secondi
            //messageIN.route("/startFan", __);  //parte ventola per T secondi
            messageIN.dispatch("/light", setLight);  //accende luce 
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
    delay(100);
}
