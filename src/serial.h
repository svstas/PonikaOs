/*
#include <TinyGPS++.h> 
//#include <gps.h>

//TinyGPSPlus gps;

void displayInfo()
 {
   Serial.print(F("Location: ")); 
   if (gps.location.isValid())
   {
     Serial.print(gps.location.lat(), 6);
     Serial.print(F(","));
     Serial.print(gps.location.lng(), 6);
   }
   else
   {
     Serial.print(F("INVALID"));
   }
 
   Serial.print(F("  Date/Time: "));
   if (gps.date.isValid())
   {
     Serial.print(gps.date.month());
     Serial.print(F("/"));
     Serial.print(gps.date.day());
     Serial.print(F("/"));
     Serial.print(gps.date.year());
   }
   else
   {
     Serial.print(F("INVALID"));
   }
 
   Serial.print(F(" "));
   if (gps.time.isValid())
   {
     if (gps.time.hour() < 10) Serial.print(F("0"));
     Serial.print(gps.time.hour());
     Serial.print(F(":"));
     if (gps.time.minute() < 10) Serial.print(F("0"));
     Serial.print(gps.time.minute());
     Serial.print(F(":"));
     if (gps.time.second() < 10) Serial.print(F("0"));
     Serial.print(gps.time.second());
     Serial.print(F("."));
     if (gps.time.centisecond() < 10) Serial.print(F("0"));
     Serial.print(gps.time.centisecond());
   }
   else
   {
     Serial.print(F("INVALID"));
   }
 
   Serial.println();
 }


*/


static void serial_in(void *arg) {
 if (!isserial) isserial = true;
 
// XSerial.begin(serspeed, SERIAL_8N1, serrx,sertx);

    while (1) {
//          while(XSerial.available()>0) Serial.write(XSerial.read());
//          while(Serial.available()>0) XSerial.write(Serial.read());
    }
}


#include "esp_task_wdt.h"
static void serial_task(void *arg)
{
time_t omm,mm;
bool isinited = false;
 if (!isserial) isserial = true;
    String inp = "";

time_t msec, omsec;
bool flag = false;
digitalWrite(13,LOW);
//vTaskDelay(50);
//digitalWrite(13,HIGH);
//sendEvent("init");
//vTaskDelay(2400);
//isinited = true;
int tms;
    while (1) {
      if (isinited) {
        msec = millis();
        tms = msec-omsec;
        if(tms>1200 && !flag) {flag = true; Serial.println("++"+String(now()));
        if (debug==5) sendEvent("sending sync master");
          continue;
        }
        if(tms>7000) {
          Serial.end();vTaskDelay(200);Serial.begin(10000000);
        if (debug==5) sendEvent("reboot slave");
         digitalWrite(13, LOW);
          vTaskDelay(200);
         digitalWrite(13, HIGH);
          isinited = false;
          vTaskDelay(2400);
          Serial.println("||"+String(now()));
          vTaskDelay(400);
          isinited = true;
          flag = false;
          omsec = msec;
        }
      } else {
        if (!flag) {flag=true;digitalWrite(13,HIGH);}
      }

      if (Serial.available()>0) {
      inp = Serial.readStringUntil('\n');
      inp.trim();
      isinited = true;
      omsec = msec;
      flag = false;

      if (inp=="||") {
                  Serial.println(String("||"+String(now())));
                  if (debug==5) sendEvent("Sending time sync");
                    vTaskDelay(20);
                continue;
      } 

      if (inp=="++") {
          if (debug == 5) sendEvent("sync_slave");

          continue;
      }
//      if (debug == 3) {
        omm = mm;
        mm = micros();
        if (omm!=mm) sendEvent(String(mm)+"> "+inp);
//      }
      }
      vTaskDelay(10);
    }
}

  