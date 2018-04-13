/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCBundle.h>
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

void sendTemperature(OSCMessage& msg, int addrOffset)
{
    float temp = GH->getTemperature();
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" *C");
    String temps = String("/temperature/" + String(temp));
    senderOSC(temps); 
}

void sendEnvHumidity(OSCMessage& msg, int addrOffset)
{
    uint8_t humi = GH->getEnvHumidity();
    //con 3.3v ha il 10% in meno rispetto 5v
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.println("%");
    String humis = String("/humidity/" + String(humi));
    senderOSC(humis);
}

void sendGroundHumidity(OSCMessage& msg, int addrOffset)
{
    uint8_t humi = GH->getGroundHumidity();
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.println("%");
    String humis = String("/humidity/" + String(humi));
    senderOSC(humis);
}

void setMaxTemperature(OSCMessage& msg, int addrOffset) 
{
    //GH->setMaxTemperature()
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
            //messageIN.route("/getWaterLevel", __);
            //messageIN.route("/getLightSensor", __); //the sensor return 1 or 0
            //messageIN.route("/getWeekTimeTable", __) //return week_tt
            
            messageIN.route("/setMaxTemperature", setMaxTemperature);  //temperatura massima prima di avviare ventola
            //messageIN.route("/setMinTemperature", setMinTemperature); //temperatura minima prima di accendere luce (?)
            //messageIN.route("/setMaxEnvHumidity", __);  //umidità massima prima di accendere ventola
            //messageIN.route("/setMinEnvHumidity", __);  //entra in allarme e controlla che la ventola sia spenta
            //messageIN.route("/setMaxGroundHumidity", __); //livello massimo umidità terreno, entra in allarme e controlla che sia chiusa l'acqua
            //messageIN.route("/setMinGroundHumidity", __); //livello minimo prima di far partire irrigazione
            //messageIN.route("/setAlarmWaterLevel", __); //livello allarme acqua (?)

            //messageIN.route("/addIrrigationTime", __);  //ho un orario di irrigazione che passo come parametro [(WeekDay,Hour,Min), ...]
            //messageIN.route("/removeIrrigationTime", __);  //rimuovo l'innaffiatura all'indice X di week_tt
            //messageIN.dispatch("/autoIrrigation/1", __);  //setto irrigation_time = true 
            //messageIN.dispatch("/autoIrrigation/0", __); //irrigation_time = false
            
            //messageIN.route("/startIrrigation", __);  //apre l'acqua per T secondi
            //messageIN.route("/startFan", __);  //parte ventola per T secondi
            //messageIN.dispatch("/light/1", __);  //accende luce dispatch non fa partial match come route
            //messageIN.dispatch("/light/0", __);  //spegne luce
        }
        Udp.flush();
    }
}

void loop() 
{
    //GH->updateData();
    receiverOSC();
    delay(2000);
}
