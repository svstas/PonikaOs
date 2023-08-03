int sertx;
int serrx;
int serspeed;
int serialid;
int xleft, xright, xfreq, xchannel;
long mass;
int xmass;

#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "led_strip.h"




 #define COUNT_LOW 1638
 #define COUNT_HIGH 7864

 #define TIMER_WIDTH 16

#include "esp32-hal-ledc.h"

time_t sc,osc;

#define ADC_PIN 34
#define ADC_UNIT ADC_UNIT_1
#define ADC_CH 34
esp_adc_cal_characteristics_t adcChar;


#include <ESP32Servo.h>

ESP32Servo servo1(23);


uint16_t xread(int xpin)
{
  /*
        long sum = 0;
        for (int i=0;i<100;i++) {
        sum +=
        analogRead(xpin);
        }
      return trunc(sum/100);
  */
  return analogRead(xpin);
}

#include <serial.h>
//#include <EBYTE.h>
#include <HX711.h>

HX711 scale;

String doshell(String msg);
void scan_i2c(bool out);
bool find_i2c(uint16_t addr);
bool nope(uint8_t x) { return true; }

/*
#ifdef GEST
TaskHandle_t GestHandler = NULL;
#include "RevEng_PAJ7620.h"
RevEng_PAJ7620 gest = RevEng_PAJ7620();
#include <gesture.h>
#include <VL53L0X.h>
VL53L0X vlx;


#endif
*/

long getmass() { return scale.get_units()  - xmass;}
float xheap() { return String(ESP.getFreeHeap()).toFloat(); }

#ifdef BME
#include <sensors/bme.h>
#endif
#ifdef VLX
#include <sensors/vl53l0x.h>
#endif
#ifdef INA219
#include <sensors/ina219.h>
#endif

const char txt3[] PROGMEM = "Ponika uptime: %12s; freemem: %d";
const char sensHelp[] PROGMEM = "Connected : %2d sensors\n";
const char sensHelp2[] PROGMEM = "Sensor ID |  Type  |                      NAME|TXT_NAME | UPT |             VALUE       |\n";
const char sensHelp1[] PROGMEM = "       %2u |     %3u |          %25s | %3u |    %13s %5s |\n";

const char txt4[] PROGMEM = "|  Task ID  |      PIN  |                 TIME TO DO  |                  REST  |\n";
const char txt5[] PROGMEM = "|       %2u  |       %2u  |        %s  |          %s  |\n";

const char line[] PROGMEM = "-----------------------------------------------------------------------------------------\n";

const char txt2[] PROGMEM = "%2u : %12s; %d\n";

const char sens0[] PROGMEM = "mem|";
const char sens1[] PROGMEM = "htut";
const char sens2[] PROGMEM = "htuh";
const char sens3[] PROGMEM = "bmp1t";
const char sens4[] PROGMEM = "bmp1p";
const char sens5[] PROGMEM = "bme2t";
const char sens6[] PROGMEM = "bme2p";
const char sens7[] PROGMEM = "bme2h";
const char sens8[] PROGMEM = "inabv";
const char sens9[] PROGMEM = "inama";
const char sens10[] PROGMEM = "analog";
const char sens11[] PROGMEM = "shtt";
const char sens12[] PROGMEM = "shth";
const char sens13[] PROGMEM = "tsl2561";
const char sens14[] PROGMEM = "mcp3421";
const char sens15[] PROGMEM = "xms5611";
const char sens16[] PROGMEM = "xts5611";

const char *const psensors[] PROGMEM = {sens0, sens1, sens2, sens3, sens4, sens5, sens6, sens7, sens8, sens9, sens10, sens11, sens12, sens13, sens14, sens15, sens16};

/*
struct DS {
  uint8_t id;
  String sname;
  DeviceAddress addr;
};
std::vector<DS> dlslist;
*/

class Sensors
{
  typedef std::function<float()> result;

private:
  void xpush(uint8_t sid, float sres)
  {
    LSensors[sid].xsum = LSensors[sid].xsum + LSensors[sid].value;
    if (LSensors[sid].xshift > 4)
    {
      LSensors[sid].xshift = 0;
      for (uint8_t i = 11; i > 0; i--)
        LSensors[sid].svalue[i] = LSensors[sid].svalue[i - 1];
      LSensors[sid].svalue[0] = (float)(LSensors[sid].xsum / 5);
      LSensors[sid].xsum = 0;
    }
    else
      LSensors[sid].xshift = LSensors[sid].xshift + 1;
    LSensors[sid].svalue[0] = sres;
    return;
  }

  bool canUpdate()
  {
    //        if (millis()-mupdate<900) return true; else return false;
    return true;
  }

  // long mupdate = millis();

public:
  int uptime()
  {
    return runtime;
  }

  struct Sensor
  {

    uint8_t id;
    char name[20] = "";
    float value = 0;
    float oldvalue = 0;
    bool isint = false;
    float xsum = 0;
    long last_update;
    uint8_t spointer = 0;
    uint8_t xshift = 0;
    uint8_t uptime;
    uint8_t sdelay = 1;
    bool kalman = false;
    std::vector<uint8_t> relays;
    bool changed = false;
    result fres;
    float svalue[12] = {};
    //    char xname[15] = "";
    //    char Sname[1] = "";
    char formula[24] = "X";
    char ext[6] = "     ";
  };
  typedef struct
  {
    String name;
    uint8_t pin_open;
    uint8_t pin_close;
    uint8_t end_open;
    uint8_t end_close;
    uint8_t pin_lock;
    char state = 0; // 0 - undefined 1 - open 2 - close 3 - move
    char move = 0;  // 0 - STOP 1 - OPENING 2 - CLOSING
  } box;

  std::vector<Sensor> LSensors;
  std::vector<box> Boxes;
  //  Sensor LSensors[12] = {};

  // char names[20][17];
  uint8_t pt = 0;
  uint8_t ptr = 0;
  uint8_t snow = 0;

  int checkid(uint8_t id)
  {
    for (uint8_t i = 0; i < pt; i++)
    {
      if (LSensors[i].id == id)
        return i;
    }
    return -1;
  }

  int8_t getSensId(char *sname)
  {

    //     if (strncmp(sname, "time", 4) == 0) return 1;

    for (uint8_t i = 0; i < pt; i++)
    {
      if (strcmp_P(sname, LSensors[i].name) == 0)
        return i;
    }
    return 0;
  }

  bool block = false;

  uint8_t cnt()
  {
    return pt;
  }
  void xclear()
  {
    pt = 0;
  }

  String loadNames(bool toscreen)
  {
    //  loadConfig(true);
    String out; //= "mem@100||100|Память|X|1|int|10000|32000|||||";
    for (uint8_t i = 0; i < csens.size(); i++)
    {
      String sss = csens[i];
      int sep = sss.indexOf("=");
      String ssname = sss.substring(0, sep);
      ssname.trim();
      sss = sss.substring(sep + 1);
      sss.trim();
      out += ssname + "@";
      uint8_t sid = getSensId(&ssname[0]);

      String rels = "";

      if (sid > -1 && sid < 200)
      {

        out += String(LSensors[sid].id);

        uint8_t rsize = LSensors[sid].relays.size();
        for (uint8_t i = 0; i < rsize; i++)
        {
          rels += LSensors[sid].relays[i] + 1;
          if (i < rsize - 1)
            rels += ",";
        };
      }
      else
        out += "-1";

      out += "|" + rels + "|" + sss;
      out += "|";
      if (i < csens.size() - 1)
        out += "#";
    }

    if (toscreen)
    {
      sendEvent(out, "x");
      out = String();
      //        evtSync(true);
    }
    else
      return out;
    out = String();
  }
  void List()
  {
    for (uint8_t i = 0; i < pt - 1; i++)
    {
      sendEvent(LSensors[i].name);
    }
  }

  void getList()
  {
    char buff[300];
    sprintf_P(buff, sensHelp, pt);
    charEvent(buff);
    snprintf_P(buff, countof(buff), line);
    charEvent(buff);
    snprintf_P(buff, countof(buff), sensHelp2);
    charEvent(buff);
    snprintf_P(buff, countof(buff), line);
    charEvent(buff);

    for (uint8_t i = 0; i < pt - 1; i++)
    {
      //            sendEvent(String(i));
      //        if (LSensors[i].id==13) continue;
      //       buff[0]=0;
      //        char se[30];
      char res[10];
      strcpy(res, cget(i));
      //      charEvent(res);
      //        if (LSensors[i].id < 50) strcpy_P(se, psensors[LSensors[i].id]); else strcpy(se, dlslist[LSensors[i].id - 50].sname.c_str());
      //        strcpy_P(se, psensors[LSensors[i].id]);
      //        strcpy(se, LSensors[i].name);
      //  const char sensHelp1[] PROGMEM = "Sensor ID: %02u; %20s; %3u; %2u; %10u; %6s; %20s; %20s;\n";
      //  const char sensHelp[] PROGMEM  = "Connected : %02d sensors\nSensor ID: id;nick;uptime;refresh;last;formula;value;Name\n

      snprintf_P(buff, countof(buff), sensHelp1, i, LSensors[i].id, LSensors[i].name, LSensors[i].uptime, res, LSensors[i].ext);
      charEvent(buff);
    }
    snprintf_P(buff, countof(buff), line);
    charEvent(buff);
    return;
  };

  int16_t getVal(char *sname)
  {
    return LSensors[getSensId(sname)].value; // else return "0";
  }

  bool setVal(char *sname, long value)
  {
    uint8_t id = getSensId(sname);
    if (id)
      LSensors[getSensId(sname)].value = value;
    else
      return false;
    return true;
  }

  float getValId(uint8_t id)
  {
    if (id > -1 && id < pt)
    {
      return LSensors[id].value;
    }
    else
      return -111;
  }

  char *cget(uint8_t id)
  {
    char buf[10] = "-";
    char *rbuf = buf;
    if (id > -1 && id < 200)
    {
      //      sendEvent(">>"+String(id)+":"+String(LSensors[id].value),"sms");

      if (LSensors[id].isint)
        itoa(LSensors[id].value, buf, 10);
      else
        dtostrf(LSensors[id].value, 8, 2, buf);

      //        charEvent(buf);
      return rbuf;
    }
    else
      return rbuf;
  }

  /*
    bool updateSensor(char* sname) {
          updateSensorId(getSensId(sname));
        return true;
    }
  */

  //    int p = 0;

  bool updateSensorId(uint8_t xid)
  {
    LSensors[xid].last_update = now();
    float res = 0;
    if (LSensors[xid].id == 9)
    {
      String out = "";
      bool arg = false;
      uint8_t xxid = 0;
      String xxname;
      for (char c : LSensors[xid].formula)
      {
        if (c == '%')
        {
          if (!arg)
          {
            arg = true;
            continue;
          }
          else
          {
            arg = false;
            continue;
          }
        }
        if (arg)
        {
          xxname += String(c);
          continue;
        }
        else
        {
          if (!xxid && xxname.length())
          {
            xxid = getSensId(&xxname[0]);
            out += String(LSensors[xxid].value);
            xxname = String();
            xxid = 0;
            continue;
          }
        }
        out += String(c);
      }
      int error;
      res = te_interp(&out[0], &error);
      LSensors[xid].value = res;
    }
    else if (LSensors[xid].id == 3)
    {

      // Serial
    }
    else
    {
      res = (float)LSensors[xid].fres(); //+p; p++;
                                         //        if (LSensors[xid].isint) res = LSensors[xid].fres(); else res = (float)LSensors[xid].fres();
      if (xid == 0)
      {

        //            sendEvent(String(xid)+" - "+String(res));
      }
      if (LSensors[xid].formula[0] != 'X')
      {
        String fff = String(LSensors[xid].formula);
        fff.replace("X", String(res));
        int error;
        res = te_interp(&fff[0], &error);
      }
      LSensors[xid].value = res;
    }

    if (LSensors[xid].value == LSensors[xid].svalue[0])
      LSensors[xid].changed = false;
    else
      LSensors[xid].changed = true;

    xpush(xid, res);
    return true;
  }

  void getSensP(char *sname)
  {
    for (auto psens : psensors)
    {

      charEvent(psens);
    }
  }

  void addSensor(const char *sname, uint8_t xdelay, std::function<float()> callback, bool isint, uint8_t id, String xfrm, String ext)
  {
    Sensor one;
    one.id = id;
    one.isint = false;
    //      one.name = sname.c_str();
    strcat(one.name, sname);
    ext += "    ";
    //      for (uint8_t i=0;i<(7-ext.length());i++) ext+=" ";
    ext.toCharArray(one.ext, 6);

    //      snprintf(one.name, "%c", sname);
    //      one.name = sname;
    //      if (id==10) one.kalman=true;
    one.sdelay = xdelay;
    one.fres = callback;
    time_t time;
    time = millis();
    one.value = callback();
    time = millis() - time;
    one.uptime = time;
    one.last_update = now();
    xfrm.toCharArray(one.formula, 24);
    LSensors.push_back(one);
    /*
          if (id > 49 && id<100) {
            DS n;
            n.sname = String(sname);
           dlslist.push_back(n);
           one.id=50+dlslist.size();
          }
    */
    pt++;
  }

  //  uint8_t count=0;

  void maintance()
  {

    if (!sensloaded) return;

//sendEvent(String(snow)+" "+String(LSensors[snow].sdelay)+" "+String(LSensors[snow].last_update));

    if (LSensors[snow].sdelay && !((now() - LSensors[snow].last_update) < LSensors[snow].sdelay))
    {
//sendEvent(String(snow));


      /*
              if (LSensors[snow].last_update * 1000 - xmillis < LSensors[snow].sdelay * 1000) {
                snow++;
                if (snow > (pt - 2)) snow = 0;
                return;
              }
      */

      time_t xmillis = millis();
      updateSensorId(snow);
      LSensors[snow].uptime = millis() - xmillis;
      vTaskDelay(20);
//      sendEvent("OK "+String(snow));
    }

    snow++;
    if (snow > (pt - 2))
      snow = 0;
  }

  void openBox(uint8_t id)
  {
    String bcmd = "b" + String(id) + "|g" + String(Boxes[id].pin_open) + ":240|r" + String(Boxes[id].end_open) + "|g" + String(Boxes[id].pin_open) + ":0|";
    //        sendEvent(bcmd);
    shells.push_back(bcmd);
  }
  void closeBox(uint8_t id)
  {
    String bcmd = "b" + String(id) + "|g" + String(Boxes[id].pin_close) + ":240|d1500|g" + String(Boxes[id].pin_close) + ":0|";
    //        sendEvent(bcmd);
    shells.push_back(bcmd);
  }

  void xinit()
  {
    for (uint8_t i = 0; i < csens.size(); i++)
    {
      uint8_t sep = csens[i].indexOf("=");
      if (!sep)
        continue;
      String par = csens[i].substring(0, sep);
      String val = csens[i].substring(sep + 1);
      par.trim();
      val.trim();
      std::vector<String> vc = ssplit(&val[0], "|");
      uint8_t xtype = vc[0].toInt();
      switch (xtype)
      {
      case 11:
//        trace(F("Start."));
        scale.begin(vc[1].toInt(), vc[2].toInt());
//        sendEvent("Scale start "+vc[1]+" "+vc[2]);
        vTaskDelay(100);
//        trace(F("Start.."));
         xmass = scale.get_units();
      
//      sendEvent(String(mass));
        //    void addSensor(const char * sname, uint8_t xdelay, std::function<float()> callback, bool isint, uint8_t id,String xfrm,String ext) {
//      sendEvent(par+","+vc[1]+","+vc[2]+","+vc[4]+","+vc[5]+","+String(xtype)+","+","+","+","+",");
        addSensor(&par[0], vc[4].toInt(), std::bind(getmass), false, xtype, vc[5], " ");
 //       trace(F("Start..."));

        break;

      case 200:
      {
        //~box1 = 200|14|12|27|28|29|Бокс1
        box box1;
        box1.pin_open = vc[1].toInt();
        box1.pin_close = vc[2].toInt();
        box1.end_open = vc[3].toInt();
        box1.end_close = vc[4].toInt();
        box1.pin_lock = vc[5].toInt();
        box1.name = vc[6];

        pinMode(box1.end_open, INPUT_PULLUP);  // OPEN_END
        pinMode(box1.end_close, INPUT_PULLUP); // CLOSE_END
        pinMode(box1.pin_open, OUTPUT);        // OPEN
        pinMode(box1.pin_close, OUTPUT);       // CLOSE
        pinMode(box1.pin_lock, OUTPUT);        // LOCK
        Boxes.push_back(box1);
        sendEvent("added box " + vc[6]);
        break;
      }
      case 201:
      {

#ifdef STEPPER

  addSensor(&par[0], vc[1].toInt(), std::bind(nope, 0), 0, xtype, vc[3], vc[5]);

/*
if (vc[4].toInt()==1) {
  
stepper0 = engine.stepperConnectToPin(vc[1].toInt());
  if (stepper0) {
    stepper0->setDirectionPin(vc[2].toInt());
    stepper0->setEnablePin(vc[3].toInt());
    stepper0->setAutoEnable(true);
    stepper0->setDelayToEnable(50);
    stepper0->setDelayToDisable(1000);
  addSensor(&par[0], vc[1].toInt(), std::bind(nope, 0), 0, xtype, vc[3], vc[5]);
sendEvent("stepper added "+vc[4]+":"+vc[1]);
  }
} else {
stepper1 = engine.stepperConnectToPin(vc[1].toInt());
  if (stepper1) {
    stepper1->setDirectionPin(vc[2].toInt());
    stepper1->setEnablePin(vc[3].toInt());
    stepper1->setAutoEnable(true);
    stepper1->setDelayToEnable(50);
    stepper1->setDelayToDisable(1000);
  addSensor(&par[0], vc[1].toInt(), std::bind(nope, 0), 0, xtype, vc[3], vc[5]);
sendEvent("stepper added "+vc[4]+":"+vc[1]);
  }
}
*/
#endif
        break;
      }
      case 202:
      {

        int pin = vc[1].toInt();
        pinMode(pin, OUTPUT);

        break;
      }
      case 9:
      {
        addSensor(&par[0], vc[1].toInt(), std::bind(nope, 0), (vc[4] == "int"), xtype, vc[3], vc[5]);
        break;
      }
      case 3:
      {
/*                                                                                                                        //
        sertx = vc[2].toInt();
        serrx = vc[3].toInt();
        serspeed = vc[4].toInt();
        serialid = pt;
*/
        //                 xTaskCreatePinnedToCore(serial_in, "xxserial", 4096, NULL, 4, &serialHandler,vc[1].toInt());
          sendEvent("Looking for NFC");

          if (!isnfc) {
//              pinMode(21,INPUT_PULLUP);
             nfc.begin(); isnfc=true;} 
          vTaskDelay(50);
          nfc.wakeup();
          uint32_t versiondata = nfc.getFirmwareVersion();
        if (!versiondata) {sendEvent("Not found"); break;}
        addSensor(&par[0], 0, std::bind(nope, 1), true, xtype, "", "");
        String out = "Found chip PN5"+String((versiondata>>24) & 0xFF, HEX)+"Firmware ver. ";
           out+=String((versiondata>>16) & 0xFF, DEC)+"."+String((versiondata>>8) & 0xFF, DEC);
           sendEvent("NFC "+out);
          nfc.setPassiveActivationRetries(0xFF);
          nfc.SAMConfig();
          
        // configASSERT(serialHandler);
        break;
      }
      case 4:
      {

        xleft = vc[1].toInt();
        xright = vc[2].toInt();
        xfreq = vc[3].toInt();
        xchannel = vc[4].toInt();
        pinMode(xleft, OUTPUT);
        ledcAttachPin(xright, xchannel);
        ledcSetup(xchannel, xfreq, 8);

        addSensor(&par[0], 0, std::bind(nope, 1), true, xtype, "", "");
        break;
      }



                case 5: { // SERVO
                  
                  uint8_t xpin    = vc[1].toInt();
//                    ledcSetup(1, 50, TIMER_WIDTH); // channel 1, 50 Hz, 16-bit width
//                    ledcAttachPin(xpin, 1);   // GPIO 22 assigned to channel 1
//                  servo1.attach(xpin);
                  if (debug==3) sendEvent("Servo: "+String(xpin));                  
//                  addSensor(&par[0],0,std::bind(nope,1),true,xtype,"","");
                  break;
                }



      case 1:
      {
        uint8_t pin = vc[1].toInt();
        pinMode(pin, ANALOG);
/*
        adc_gpio_init(ADC_UNIT, (adc_channel_t)ADC_CH);
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(ADC_CH, ADC_ATTEN_DB_11);
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adcChar);
*/
        //                  analogSetClockDiv(128);
        //                  addSensor(&par[0],vc[2].toInt(),std::bind(analogRead,pin),(vc[5]=="int"),xtype,vc[4],vc[6]);
        addSensor(&par[0], vc[2].toInt(), std::bind(xread, pin), (vc[5] == "int"), xtype, vc[4], vc[6]);
        //                 vTaskDelay(100);
        //                 if (vc[5]=="int") LSensors[pt-1].isint = true;
        //                 vc[4].toCharArray(LSensors[pt-1].formula, 24);
        break;
      }
      case 2:
      {
        uint8_t addr = vc[1].toInt();

        switch (addr)
        {

          case 0x24:

          if (find_i2c(0x24)) {

//            addSensor(&par[c.b0], 1, std::bind(nope), 1, 2, vc[4], vc[6]);

          }

          break;

#ifdef BME          
        case 0x76:
        {
          uint8_t xf = 0;
          if (find_i2c(0x76))
          {
            if (par == "bme2t")
              xf = 1;
            else if (par == "bme2h")
              xf = 2;
            else if (par == "bme2p")
              xf = 3;
            addSensor(&par[0], 1, std::bind(getBME, xf), (vc[5] == "int"), 2, vc[4], vc[6]);
          }
          break;

        }

  #endif

        case 0x29:
        {
          #ifdef VLX
          uint8_t xf = 0;
          if (vlx.init())
//            Serial.println("VLX ok");
          vlx.startContinuous();
          if (find_i2c(0x29))
          {
            addSensor(&par[0], 1, std::bind(getVLX), 1, 2, vc[4], vc[6]);
          }
          #endif
          break;
        }
        }
        break;
      }
      }
    }
  }

  void pnp()
  {
    sensloaded = false;
    sendEvent("Scaning devices...");
    vTaskDelay(20);
    std::vector<Sensor>(LSensors).swap(LSensors);
    LSensors.clear();
    Boxes.clear();
    pt = 1;
    scan_i2c(false);
    //     addSensor("mem",1,std::bind(xheap),true,0,"X","bytes");
    addSensor("mem", 1, std::bind(xheap), true, 0, "X", "bytes");
    xinit();
    vTaskDelay(50);
    sensloaded = true;
    sendEvent("added " + String(pt - 1) + " sensors");
  }

  void showBoxes()
  {
    String out = "";
    for (uint8_t i = 0; i < Boxes.size(); i++)
    {
      out += "Box[" + String(i) + "] => " + Boxes[i].name + "," + String(Boxes[i].end_open) + ";\n";
    }
    sendEvent(out);
    out = String();
  }

  String onLine()
  {
    unsigned long sstime = now() - 65;
    String mem = String(ESP.getFreeHeap());
    String out;
    for (uint8_t j = 12; j > 0; j--)
    {
      sstime = sstime + 5;
      out += String(sstime) + "|" + mem;

      for (uint8_t i = 0; i < pt; i++)
      {
        if (LSensors[i].isint)
          out += "|" + String(LSensors[i].id) + ":" + String((int)LSensors[i].svalue[j - 1]);
        else
          out += "|" + String(LSensors[i].id) + ":" + String(LSensors[i].svalue[j - 1]);
      }
      out += "\n";
    }
    return out;
    out = String();
  }
};

Sensors *mysensors = new Sensors();
