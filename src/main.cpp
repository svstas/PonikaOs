#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <time.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <SPIFFSEditor.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <pins_arduino.h>
#include <dummy.h>
#include <FS.h>
#include <LITTLEFS.h>
#include <DNSServer.h>
#include <stdlib.h>
#include <pgmspace.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <Wire.h>
#include <ESP32SSDP.h>
#include <NetBIOS.h>
#include <esp_now.h>
#include <tinyexpr.h> 




/* GIT*/
#include <HardwareSerial.h>
HardwareSerial XSerial(1);
String sendcmd = "";
TaskHandle_t serialHandler = NULL;
bool serialflag = false;
char serialBuffer[256];
int gbright = 100; // PIXEL
String ptext = "";
uint8_t debug = 0;



#include <PN532_HSU.h>
#include <PN532.h>


PN532_HSU pn532hsu(XSerial);
PN532 nfc(pn532hsu);
bool isnfc = false;



int pw = 0;
int vl = 0;
int pw1 = 0;
int vl1 = 0;

//#include <INA226.h>

//INA226 INA(0x40);

//  XSerial.begin(57600, SERIAL_8N1, 16, 17); // RX TX

extern "C"
{
#include "lwip/err.h"
#include "lwip/dns.h"
}

//#include "tinyexpr.h"

time_t now() { return time(NULL); }
time_t session = 0;
time_t runtime = 0;
uint16_t energy = 0;
time_t sec = 0;
time_t osec = 0;

bool sensloaded = false;
bool isgest = false;


#include <config.h>


struct stepcmd
{
  uint8_t id;
  int pos;
  int speed;
  int accel;
};

std::vector<stepcmd> stepbuf;

#ifdef STEPPER



#include "FastAccelStepper.h"
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper0, *stepper1 = NULL;

int stepflag[] = {0, 0};
bool stepend = 0;
int sstepend = 0;

/*
void ICACHE_RAM_ATTR stepHandler(){
  if (!digitalRead(34)) sstepend++; else sstepend=0;
  if (sstepend>10) {stepend=1; sstepend=0;}
}
*/

#endif


RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int timesleep = 30;

std::vector<String> cmdss;

uint8_t clid = 0;
uint8_t clidx = 0;
File xxxf;

struct conf
{
  const char *hostName = "ponika";
  const char *password = "josperado";
  const char *http_username = "admin";
  const char *http_password = "admin";
  const char *ssid = "ConceptHome";
  const char *passwd = "josperado";
  int wifi = 2; // 3 - STA, 2 - AUTO, 1 - AP
  bool ota = 1;
  bool tcp = 1;
  bool display = 1;
  bool lora = 1;
  uint8_t debug = 0;
  std::vector<uint16_t> i2c;
} conf;

// TwoWire I2Cone = TwoWire(1);

// String udpAddress = "192.168.88.234";
// uint16_t srate = 22050;
bool isadc = false;
bool isserial = false;
// uint16_t sdelay = 285;

// WiFiUDP udp;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

DNSServer dns;

std::vector<String> shells;
std::vector<String> tasks;
std::vector<String> xxtasks;
std::vector<String> csens;

AsyncEventSourceClient *xxclient;
// WiFiClient clientSTA;
#include <ArduinoJson.h>
#include <functions.h>
#include <sensors.h>
#include <commands.h>
#include <i2c.h>
#include <setup.h>
#include <tasker.h>
#include <cron.h>

int ostep = 0;
int sstep = 0;
void loop()
{
  osec = sec;
  sec = now();
  ostep = sstep;



  ArduinoOTA.handle();
  //  dns.processNextRequest();
  if (shells.size())
  {
    String xxt = shells.back();
    shells.pop_back();
    doshell(xxt);
  } // else dotask(false);
  if (tasks.size())
    dotask(true); // else dotask(false);

if (debug>3 && ostep != sstep) {
    sendEvent(String(millis()) + " " + String(sstep) + " - " + String(pw) + " - " + String(pw1) + " - " + String(vl));

//    INA.setMaxCurrentShunt(1, 0.0001);
//    measure(20);

    //  INA.setMaxCurrentShunt(1, 0.002, false);
    //  measure(20);
  
}
  if (osec != sec)
  {
    if (session < 1600000000 || runtime < 0) session = now();
    runtime = now() - session;
  }
//  mysensors->maintance();
 
 if (stepend) {
  tasks.push_back("step 0 0"); 
	 sendEvent("Концевик");
  stepend=0;
 }

  vTaskDelay(10);
}