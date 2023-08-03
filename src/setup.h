#include <WiFiEvents.h>
//#include <stepper.h>

uint8_t oprogr = 0;

#ifdef BUTTON
#include <button.h>
#endif

#ifdef INA219
void measure(uint8_t count)
{
  pw = INA.getPower_mW();
  vl = INA.getBusVoltage();
  pw1 = INA.getCurrent_mA();

  // delay(3000);
  Serial.println("\nBUS\tSHUNT\tCURRENT\tPOWER");
  Serial.println(" V\t mV\t mA\t mW");
  for (int i = 0; i < count; i++)
  {
    Serial.print(INA.getBusVoltage(), 3);
    Serial.print("\t");
    Serial.print(INA.getShuntVoltage_mV(), 3);
    Serial.print("\t");
    Serial.print(INA.getCurrent_mA(), 3);
    Serial.print("\t");
    Serial.print(INA.getPower_mW(), 3);
    Serial.println();
    //    delay(500);
  }
}

void printConfig()
{
  Serial.print("LSB:\t");
  Serial.println(INA.getCurrentLSB(), 10);
  Serial.print("LSB_uA:\t");
  Serial.println(INA.getCurrentLSB_uA(), 3);
  Serial.print("shunt:\t");
  Serial.println(INA.getShunt(), 3);
  Serial.print("maxCur:\t");
  Serial.println(INA.getMaxCurrent(), 3);
  Serial.println();
}
#endif


void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{
  // Copies the sender mac address to a string
  char macStr[18];
  trace("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  trace(macStr);
  //  charEvent(macStr);
}

void setup()
{

  ++bootCount;

//  Serial.begin(115200);
  pinMode(13, OUTPUT);
  //  Serial.setDebugOutput(true);

    Serial.begin(10000000);


  if (bootCount > 1)
  {
    if (bootCount > 10)
      ESP.restart();
#ifdef BME
    BMESensor.refresh();
    delay(500);
//  Serial.println(bootCount);
//  Serial.println(BMESensor.temperature);
//  Serial.flush();
#endif
    esp_sleep_enable_timer_wakeup(timesleep * 1000000);
    esp_deep_sleep_start();
    return;
  }

#ifdef BUTTON
  button_init();
#endif

  //  Wire.begin(21, 19, 400000);
  //  Wire.begin(21, 19);
  Wire.begin(16, 17);


#ifdef BME
  BMESensor.begin();
  delay(100);
  BMESensor.refresh();
#endif

  // pinMode(LED, OUTPUT);
  // pinMode(4, OUTPUT);
  // digitalWrite(4,LOW);

  xtrace(F("System starting..."));
  LittleFS.begin();
  xtrace(F("FS init"));
  if (loadConfig())
    trace(F("Config loaded"));

  if (conf.wifi)
    WiFi.onEvent(WiFiEvent);

  if (conf.wifi == 1)
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(conf.hostName, conf.password);
    delay(2000);
    WiFi.softAPConfig(ip, ip, netmask);
    xtrace(F("WiFi Started"));
  }
  else if (conf.wifi == 2)
  {
    AsyncWiFiManager wifiManager(&server, &dns);
    wifiManager.setConfigPortalTimeout(240);
    wifiManager.setBreakAfterConfig(true);
    //  wifiManager.resetSettings();

    if (!wifiManager.autoConnect(conf.hostName, conf.password))
    {
      xtrace(F("failed to connect, we should reset as see if it connects"));
      delay(3000);
      ESP.restart();
      ESP.restart();
      delay(5000);
    }
  }
  else if (conf.wifi == 3)
  {
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(conf.ssid, conf.passwd);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      xtrace(F("Setting as a Wi-Fi Station.."));
    }

    if (esp_now_init() != ESP_OK)
    {
      xtrace(F("Error initializing ESP-NOW"));
      return;
    }
    esp_now_register_recv_cb(OnDataRecv);
  }

  if (conf.ota)
  {
    xtrace(F("OTA Started"));
    ArduinoOTA.onStart([]()
                       { events.send("Update Start", "ota"); });
    ArduinoOTA.onEnd([]()
                     { events.send("Update End", "ota"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          {
    char p[32];
   
   uint8_t progr = (progress)/(total/100);
    if (oprogr != progr) {
      oprogr = progr;

    sprintf(p, "Progress: %d\n", progr);
    events.send(p,"m");
    } });

    ArduinoOTA.onError([](ota_error_t error)
                       {
    if(error == OTA_AUTH_ERROR) events.send("Auth Failed", "ota");
    else if(error == OTA_BEGIN_ERROR) events.send("Begin Failed", "ota");
    else if(error == OTA_CONNECT_ERROR) events.send("Connect Failed", "ota");
    else if(error == OTA_RECEIVE_ERROR) events.send("Recieve Failed", "ota");
    else if(error == OTA_END_ERROR) events.send("End Failed", "ota"); });
    ArduinoOTA.setHostname(conf.hostName);
    ArduinoOTA.begin();
  }

  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.setName(conf.hostName);
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.begin();

  MDNS.addService("http", "tcp", 80);
  MDNS.begin(conf.hostName);
  NBNS.begin(conf.hostName);

  // MODE_AP!!! NEEDED!!!
  //  dns.setErrorReplyCode(DNSReplyCode::NoError);
  //  dns.start(53, "*", IPAddress(192,168,4,1));
  xtrace(F("DNS Started"));
  xtrace(F(conf.hostName));
  xtrace(F("WiFi OK"));

  if (conf.tcp)
  {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    events.onConnect([](AsyncEventSourceClient *client)
                     {
      //    client->send("hello!",NULL,millis(),1000);
      xxclient = client; });
    server.addHandler(&events);
    server.addHandler(new SPIFFSEditor(LittleFS, conf.http_username, conf.http_password));
    server.on("/generate_204", handleRoot); // Android captive portal. Maybe not needed. Might be handled by notFound handler.
    server.on("/fwlink", handleRoot);       // Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", String(now())); });
    server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", String(ESP.getFreeHeap())); });
    server.on("/realip", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->redirect("http://" + toStringIp(WiFi.localIP()) + "/"); });
    server.on("/_cmd", HTTP_GET, [](AsyncWebServerRequest *request)
              {if(request->method() == HTTP_GET && request->hasParam("cmd")){
    String cres = doshell(request->getParam("cmd")->value());
    if (!cres.length()) request->send(304); else request->send(200,"text/html",cres);
    cres = String();} });

    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.onNotFound([](AsyncWebServerRequest *request)
                      {
                        handleRoot(request);
                        /*    request->send(404); */ });
    server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                        {
      /* //    if(!index)
      //      trace("UploadStart: %s\n", filename.c_str());
      //      trace("%s", (const char*)data);
      //    if(final)  trace("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
      */ });
    server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                         {
    if(!index)
      trace("BodyStart: %u\n", &String(total)[0]);
      trace("%s", (const char*)data);
    if(index + len == total)
      trace("BodyEnd: %u\n", &String(total)[0]); });
    server.begin();
    //  vTaskDelay(500);
    session = now();
    mysensors->pnp();
  }

  // int pos = 0;
  //	ESP32PWM::allocateTimer(0);/
  //	servo1.setPeriodHertz(50);

  // servo1.attach(servo1Pin, minUs, maxUs);
  /*
    for (pos = 0; pos <= 1800; pos += 1) { // sweep from 0 degrees to 180 degrees
      // in steps of 1 degree
      servo1.write(pos);
      delay(1);             // waits 20ms for the servo to reach the position
    }
  */
#ifdef INA219
  if (!INA.begin())
  {
//    Serial.println("could not connect. Fix and Reboot");
  }
#endif
/*
  Serial.println();
  Serial.print("MAN:\t");
  Serial.println(INA.getManufacturerID(), HEX);
  Serial.print("DIE:\t");
  Serial.println(INA.getDieID(), HEX);
  Serial.println();
  delay(100);

  INA.setMaxCurrentShunt(1, 0.002);
  Serial.println("normalized = true (default)");
  printConfig();

  INA.setMaxCurrentShunt(1, 0.002, false);
  Serial.println("normalized = false");
  printConfig();

  INA.setMaxCurrentShunt(15, 0.002);
  Serial.println("normalized = true (default)");
  printConfig();

  INA.setMaxCurrentShunt(15, 0.002, false);
  Serial.println("normalized = false");
  printConfig();
*/
  configTime(3600 * 2, 3600, "pool.ntp.org");

#ifdef STEPPER

  engine.init();


  stepper0 = engine.stepperConnectToPin(26);
  
  if (stepper0) {
    stepper0->setDirectionPin(12);
    stepper0->setEnablePin(27);
    stepper0->setAutoEnable(true);
    stepper0->setDelayToEnable(50);
    stepper0->setDelayToDisable(1000);
  }
  
  stepper1 = engine.stepperConnectToPin(32);
  
  if (stepper1) {
    stepper1->setDirectionPin(33);
    stepper1->setEnablePin(25);
    stepper1->setAutoEnable(true);
    stepper1->setDelayToEnable(50);
    stepper1->setDelayToDisable(1000);
  }

//pinMode(34, INPUT_PULLUP);
//attachInterrupt(digitalPinToInterrupt(34), stepHandler, FALLING);


#endif




  debug = 3;
  vTaskDelay(200);
  xTaskCreatePinnedToCore(serial_task, "xserial", 8192, NULL, 3, NULL, 0);
//  vTaskDelay(500);
//  digitalWrite(13,LOW);

WiFiClient client;
client.connect("192.168.1.1",8080);

String readRequest = "GET /reset/192.168.1.28 HTTP/1.1\r\nHost: 192.168.1.1\r\nConnection: close\r\n\r\n";
client.print(readRequest);
vTaskDelay(2500);
client.stop();

    mysensors->pnp();


}
