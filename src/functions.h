#define countof(a) (sizeof(a) / sizeof(a[0]))

const char mjan[] PROGMEM = "Jan";const char mfeb[] PROGMEM = "Feb";const char mmar[] PROGMEM = "Mar";const char mapr[] PROGMEM = "Apr";const char mmay[] PROGMEM = "May";const char mjun[] PROGMEM = "Jun";const char mjul[] PROGMEM = "Jul";const char maug[] PROGMEM = "Aug";const char msep[] PROGMEM = "Sep";const char moct[] PROGMEM = "Oct";const char mnov[] PROGMEM = "Nov";const char mdec[] PROGMEM = "Dec";
const char * const  xmonth[] PROGMEM = {mjan,mfeb,mmar,mapr,mmay,mjun,mjul,maug,msep,moct,mnov,mdec};

String doshell(String msg);

void charEvent(const char* zxmsg) {
    events.send(zxmsg,"m");
}
void sendEvent(String zxmsg);

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    clid = client->id(); 
  //  Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
 //   client->printf("Hello Client %u :)", client->id());
 //   client->printf("ws: %u, ts:%u\n",client->id(),(unsigned int)now());
//    vTaskDelay(500);
//    events.send(&String(now())[0],"t");
      client->ping(); 
  } else if(type == WS_EVT_DISCONNECT){
   // Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
   // Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
   // Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
//      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
     // Serial.printf("%s\n",msg.c_str());
//      int smpls = msg.toInt();
//      int sm = 22050; if (smpls>35000) sm = 44100; 
//      sdelay = (sm - smpls)/2;
//      if (!(sdelay % 2)) sdelay++;
//      if (sdelay>1000) sdelay = 1000;
    
//      Serial.print("DELAY: ");
//      Serial.println(sdelay);
//      Serial.print("msg: ");
//      Serial.println(msg.toInt());

//      if (sdelay<=0)       sdelay=300;                                       // Синхронизация звука

      if(info->opcode == WS_TEXT) {
//        client->text("I got your text message");
      }
      else {}
 //       client->binary("I got your binary message");
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
//        if(info->num == 0)
//          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
//        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

//      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
//      Serial.printf("%s\n",msg.c_str());

      if((info->index + len) == info->len){
//        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
//          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
	if (msg.length()) {
    String cres = doshell(msg);
    if (cres.length()) sendEvent(cres);
    cres = String();
  }

  }
}


void trace(const char* str) {
    if (conf.debug) {
        if (conf.debug>1) sendEvent(String(str));
    //  else  Serial.println(str);  
    }
}
void trace(const __FlashStringHelper *str) {

    if (conf.debug) {
        if (conf.debug>1) sendEvent(String(str));
  //    else  Serial.println(str);  
    }
} 
  

void xtrace(const __FlashStringHelper *str) {
 //   Serial.println(str);  
}

void trace(const char* xfmt, const char* str) {
//  Serial.printf(xfmt,str);
}

void sendEvent(String msg, String to) {
  events.send(msg.c_str(),to.c_str());
}

void sendEvent(String msg) {
  events.send(msg.c_str(),"m");
}
void sendEvent(long msg) {
  events.send(String(msg).c_str(),"m");
}
String secTime(int stime) {
int hours=0, days=0, xminutes=0, sminutes=0, shour=0, xseconds = 0; 
  days = stime/86400;
  if (days) {
  hours = stime-days*86400;
  } else hours = stime;
   shour = hours/3600;
  if (shour) {
   xminutes = hours-shour*3600;
  } else xminutes = stime;
    sminutes = xminutes/60;
  if (sminutes) {
   xseconds = xminutes-sminutes*60;
  } else xseconds = stime;
 
  char buff[30];
      snprintf_P(buff, 
            countof(buff),
            PSTR("%02u:%02u:%02u"),
            shour,
            sminutes,
            xseconds
            );
    if (days) return "%u days "+String(buff); 
    else return String(buff);
}



String printDateTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))return "error getting time!";
  time_t t; time(&t);
//  int ok = sprintf(buff, "%A, %B %d %Y %H:%M:%S", ctime());
  return String(ctime(&t));
}

/*
String printDateTime() {
    char datestring[30];
//  time_t t = now()+3600*conf.timeZone;
  time_t t = now()+3600*3;
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%02u %02u:%02u:%02u"),
            year(t),            
            month(t),
            day(t),
            hour(t),
            minute(t),
            second(t));
    return String(datestring);
}
*/

uint8_t gmonth(char * fmonth) {
  for (uint8_t i=0;i<12;i++) if (strcmp_P(fmonth,xmonth[i])==0) return i+1;
  return 0;
}

boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

String toStringIp(IPAddress ip) {
String res = "";
for (uint8_t i=0;i<3;i++) {
  res += String((ip >> (8*i)) & 0xFF)+".";
}
  res += String(((ip >> 8*3)) & 0xFF);
  return res;
}
void handleRoot(AsyncWebServerRequest *request) {
 // Serial.println("HD_ROOT");
 if (!isIp(request->host())) {
//  Serial.println("HD_ROOT_______");
  
AsyncWebServerResponse *response = request->beginResponse(302,"text/plain","");
response->addHeader("Location",String("http://")+toStringIp(request->client()->localIP()));
request->send(response);
  
 }
}

std::vector<String> ssplit(char * str, const char *sep) {
  std::vector<String>vc;
  char *token = strtok(str, sep);
  while (token != NULL) {
    vc.push_back(token);
    token = strtok(NULL, sep);
  };
  return vc;
  std::vector<String>(vc).swap(vc);
}

bool loadConfig() {

  StaticJsonDocument<2048> doc;

  File f = LittleFS.open("/inits/config.json", "r");
 // DeserializationError error = deserializeJson(doc, f);
  //if (error) Serial.println(F("Failed to read file, using default configuration"));
    conf.hostName = doc["hostName"] | "ponika";
    conf.password = doc["password"] | "josperado";
    conf.ssid = doc["ssid"] | "ponika";
    conf.passwd = doc["passwd"] | "josperado";
    conf.http_username = doc["http_username"] | "admin";
    conf.http_password = doc["http_password"] | "admin";
    conf.wifi = doc["wifi"] | 2;
    conf.ota  = doc["ota"] | 1;
    conf.tcp  = doc["tcp"] | 1;
    conf.display = doc["display"] | 1;
    conf.lora = doc["lora"] | 1;
//    serializeJsonPretty(doc, Serial);
  f.close();
/*    String par;
    String val;

  if (!f) {trace(F("Failed to open config.ini")); return false;}
//  while(f.available()) {
//  }
  f.close();
*/

  f = LittleFS.open("/inits/sensors.ini", "r");
  while(f.available()) {
  String xs = f.readStringUntil('\n');
    if (xs.substring(0,1)=="#") continue;
    if (xs.substring(0,1)=="~") {csens.push_back(xs.substring(1)); continue;}
  }
  f.close();

  return true;
}

