#include "Arduino.h"
#include <utf8rus.h>


bool startst0 = false;
bool startst1 = false;
bool online = false;
long oldmass;
String card, ocard;

String getNFC() {
          boolean success;
          uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
          uint8_t uidLength;
//          sendEvent("NFC Request");                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
          success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
            String out="";
        if (success) {
            String out="";
         
//         sendEvent(String(uid));

        ocard = card;
         for (uint8_t i=0; i < uidLength; i++) 
           {
//              out += " 0x" + String(uid[i], HEX); 
              out += String(uid[i], HEX); 
           }
//           if (!out.length()) out="---";
          card = out;
        } else card = "0";

//        if (ocard!=card) out="1"; else out="0";
//        ocard = card;
        return card;
    
}

struct AMessage
{
    char ucMessageID;
    char ucData[256];
} xMessage;

QueueHandle_t xQueue;

uint8_t PWM_valueMax[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int8_t PWM_PinChannel[16] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

void PWM_initialize(int pin, int channel = 0, uint32_t valueMax = 255, int basefreq = 10000, int timer = 13)
{
    PWM_PinChannel[pin] = channel;
    PWM_valueMax[pin] = valueMax;
    ledcSetup(channel, basefreq, timer);
    ledcAttachPin(pin, channel);
};

void analogWrite(uint8_t pin, uint32_t value)
{
    // pulse width, 8191 from 2 ^ 13 - 1

    if (value)
    {
        PWM_initialize(pin);
        uint32_t width = (8191 / PWM_valueMax[pin]) * std::min<int>(value, PWM_valueMax[pin]);

        // write PWM width

        ledcWrite(PWM_PinChannel[pin], width);
    }
    else
        ledcDetachPin(pin);
};

bool i2ctouch = false;

String listDir(fs::FS &fs, String dirname, uint8_t levels)
{
    String out = "";
    String xdir = "";

    if (dirname.substring(0, 1) != "/")
        xdir = "/" + dirname;
    else
        xdir = dirname;
    sendEvent("Listing directory: " + xdir);

    File root = fs.open(&xdir[0]);
    if (!root)
    {
        out += "- failed to open directory";
        return out;
    }
    if (!root.isDirectory())
    {
        out += " - not a directory";
        return out;
    }

    File file = root.openNextFile();
    while (file)
    {
        out = "";
        if (file.isDirectory())
        {
            //            out+="  DIR : ";
            out += String(file.name()); //+"\n";
            out += "\t\t\tDIR\n";
            if (levels)
            {
                listDir(fs, String(file.name()), levels - 1);
            }
        }
        else
        {
            //          out+="  FILE: ";
            String fname = String(file.name());
            out += fname; //+"\n";
            if (fname.length() < 8)
                out += "\t";
            if (fname.length() < 16)
                out += "\t";
            out += "\t";
            out += "SIZE: ";
            out += String(file.size()) + "\n";
        }
        sendEvent(out);
        file = root.openNextFile();
    }
    return "";
}

int vvv = 0;
int mistake = 300;
void tasker(void *arg)
{
    struct AMessage *pxRxedMessage;
    std::vector<String> cmds;
    uint8_t boxid = 0;
    time_t xflag = 0;
    while (xQueue == 0)
        ;
    if (xQueueReceive(xQueue, &(pxRxedMessage), (TickType_t)10))
    {
        cmds = ssplit(&String(pxRxedMessage->ucData)[0], "|");

        for (uint8_t i = 0; i < cmds.size(); i++)
        {
                char cmd = cmds[i][0]; 

            std::vector<String> param = ssplit(&cmds[i].substring(1)[0], ":");

            switch (cmd)
            {
            case 'o':  // params = pin,delay
                doshell("~gpio write "+param[0]+" 1");
                vTaskDelay(param[1].toInt());
                doshell("~gpio write "+param[0]+" 0");
                
            break;
            case 'y':               // Взвешивать постоянно
                if (param[0].toInt()) {
                    online = true;
                    long shownmass = 0;
                    sendEvent("Start scaler...");
//                        mass = scale.get_units() - xmass;
//                     sendEvent(String(ceil(mass/100)));
                    while(online) {
                        oldmass=mass;
                        mass = abs(scale.get_units() - xmass); 
                        if (mass>1500000) mass=oldmass;
                        vTaskDelay(100);
//                        sendEvent("---"+String(mass));
                    if (mass<mistake) mass = 0;        
                   if ((abs(mass-oldmass)<mistake || oldmass == mass) && shownmass>0) continue;
                        shownmass = ceil(mass/100);
                        sendEvent("y:"+String(shownmass));
                        shownmass=1;
//                       sendEvent(String(ceil(mass/100)));
                    vTaskDelay(param[1].toInt());
                    }
                } else {
                  online = false;  
                    sendEvent("Stop scaler...");
                }
            break;
            case 'm':                                       // Спираль скидывания мешков
                    if (param[0]=="n") {
                    card = getNFC();
    		     while (card.length()<4) {		
                    	tasks.push_back("step 1 1612 300 4600");
                    	vTaskDelay(600);
	                card = getNFC();
                 }
                    sendEvent(card);
                    tasks.push_back("servo 165");
                    vTaskDelay(50);
                    tasks.push_back("step 1 1612 300 4600");
                    	vTaskDelay(1500);
    	                card = getNFC();
                        sendEvent(card+" "+ocard);
                    while (ocard == card && card!="0") {
                    tasks.push_back("step 1 1612 300 4600");
                   	vTaskDelay(1500);
                    card = getNFC();
                   	vTaskDelay(50);
                       sendEvent(card+" "+ocard);
                    }
                    tasks.push_back("servo 185");
                    
//			sendEvent(card);			

		    } else for (int j=0;j<param[0].toInt();j++) {
                    tasks.push_back("step 1 1600 300 4600");
                    vTaskDelay(1500);
                    }
            break;
            case 't':
                    if (param[1].toInt()) { 
                        tasks.push_back("~step 0 90000 20000 40000");
			                vTaskDelay(param[1].toInt());
                        tasks.push_back("~step 0 -90000 20000 40000");
                                                     
                                  // Поднять/Опусить стол
                    } else if (param[0].toInt()) 
                        tasks.push_back("~step 0 90000 20000 40000");
                    else 
                        tasks.push_back("~step 0 -90000 20000 40000");
            break;
            case 'q':                                       // Серво шторки Откр:162 Закр:182

                    tasks.push_back("servo "+param[0]);

            break;
            case 's':                                       // Большая дверь 1 - открыть 0 - закрыть
                if (param[0].toInt())
                    tasks.push_back("step 0 9820 250 5000");
                else
                    tasks.push_back("step 0 -9820 250 5000");
                break;
            case 'n':
                    sendEvent(getNFC());
                break;            
            case 'l':            				            // Свет в камере приема
            {
                int lpin = param[0].toInt();
                int lup = param[1].toInt();
                int ldel = param[2].toInt();

                if (lup)
                {
                    for (int i = 50; i < 255; i++)
                    {
                        analogWrite(lpin, i);
                        vTaskDelay(ldel);
                    }
                }
                else
                {
                    for (int i = 255; i > -1; i--)
                    {
                        analogWrite(lpin, i);
                        vTaskDelay(ldel);
                    }
                }
            }

            break;
            case 'a':
                mysensors->openBox(param[0].toInt());
                break;
            case 'z':
                mysensors->closeBox(param[0].toInt());
                break;
            case 'b':
                boxid = param[0].toInt();
                break;
            case 'g':
                analogWrite((uint8_t)param[0].toInt(), (uint8_t)param[1].toInt());
                if (param[0].toInt() == mysensors->Boxes[boxid].pin_open)
                    mysensors->Boxes[boxid].move = 1;
                else
                    mysensors->Boxes[boxid].move = 2;
                break;
            case 'd':                                               // Пауза
                vTaskDelay(cmds[i].substring(1).toInt());
                break;
            case 'r':                                               // Ждем HIGH на GPIO ноге
                while (digitalRead(cmds[i].substring(1).toInt()) == 1)
                {
                    vTaskDelay(10);
                    xflag = 0;
                }
                if (!xflag)
                    xflag = millis();
                else
                {
                    if (millis() - xflag > 9)
                    {

                        mysensors->Boxes[boxid].move = 0;
                        if (param[0].toInt() == mysensors->Boxes[boxid].end_open)
                            mysensors->Boxes[boxid].state = 1;
                        else
                            mysensors->Boxes[boxid].state = 2;
                        xflag = 0;
                    }
                    vTaskDelay(2);
                }

                break;
            case 'c':
                shells.push_back(xxtasks[cmds[i].substring(1).toInt()]);
                break;
            }
            (param).swap(param);
        }
    }

    (cmds).swap(cmds);
    vTaskDelete(NULL);
}

String doshell(String msg)
{
    msg.toLowerCase();
    msg.trim();
    
   if (msg.substring(0, 1) == "|")
    {
        msg = xxtasks[msg.substring(1).toInt()];
    }
   
    if (msg.substring(0, 1) == "!")
    {
        tasks.push_back(msg.substring(1));
        return "";
    }

    else if (msg.substring(0, 1) == "~")
    {
        Serial.println(msg.substring(1));
        return "";
    }

    else if (msg.substring(0,2)=="||") {

        uint8_t scmd = msg.substring(0,1).toInt();
        sendEvent("SyncTime");
                    Serial.println(String("||"+String(now())));
       return ""; 
    }

    
    //   else if (msg.substring(0,3)=="udp") {
    //        udpAddress = msg.substring(4); udpAddress.trim();
    //        return "SET UDP address: "+udpAddress;
    //   }

    else if (msg=="help") {tasks.push_back("cat help.txt");}
    else if (msg.substring(0,4)=="echo") {sendEvent(msg.substring(5)); return "";}

    else if (msg == "boxes")
    {

        mysensors->showBoxes();
        return "";
    }

    else if (msg == "reset")
    {
        trace("Restarting...");
        sendEvent("Restarting...");
        delay(1000);
        ESP.restart();
    }

    else if (msg.lastIndexOf("|") > 0)
    {
        TaskHandle_t hnd;
        xQueue = xQueueCreate(10, sizeof(struct AMessage *));
        xTaskCreatePinnedToCore(tasker, "tasker", 8192, NULL, 2, &hnd, 0);
        struct AMessage *pxMessage;
        strcpy(xMessage.ucData, msg.c_str());
        pxMessage = &xMessage;
        xQueueSend(xQueue, (void *)&pxMessage, 0);
        vTaskDelay(10);
        bool flag = false;
        for (int i = 0; i < xxtasks.size(); i++) if (xxtasks[i] == msg) {flag = true; break;}
        if (!flag && msg.substring(0,1) != "c") {
            xxtasks.push_back(msg);
            return "C" + String(xxtasks.size());
        }   
        return "";
    }

    else if (msg=="mass") {
        tasks.push_back("mass");
        return "";
    }
    else if (msg.substring(0,5)=="xmass") {
        tasks.push_back("xmass");
        int mm = msg.substring(6).toInt();
        if (mm) mistake = mm;
        return "";
    }
    else if (msg == "nfc") {
                    sendEvent(getNFC());
    }

    else if (msg == "i2c")
    {
        tasks.push_back("i2c");
        return "";
    }

    else if (msg.substring(0, 2) == "ls")
    {
        String path = msg.substring(3);
        path.trim();
        if (!path.length())
            path = "/";
        if (path.substring(0, 1) != "/")
            path = "/" + path;
        if (path.endsWith("/"))
            path = path.substring(0, path.length() - 1);
        if (path == "/i2c")
        {
            tasks.push_back("i2c");
            return "";
        }
        else if (path == "/dev")
        {
            tasks.push_back("dev");
            return "";
        }
        else
            return listDir(LittleFS, path, 0);
    }

    else if (msg == "cmds")
    {
        String out = "";
        for (int i = 0; i < xxtasks.size(); i++)
            out += "C" + String(i) + ": " + xxtasks[i] + "\n";
        return out;
    }
    else if (msg == "free")
    {
        //    unsigned int totalBytes = LittleFS.totalBytes();
        //    unsigned int usedBytes = LittleFS.usedBytes();

        return "Flash size" + String(LittleFS.totalBytes()) + "\n" + "Free heap memory : " + String(ESP.getFreeHeap()) + " of " + String(ESP.getHeapSize());
    }

    else if (msg == "psram")
        return "Free PSRAM memory : " + String(ESP.getFreePsram()); //+" of "+String(ESP.getPsramSize());

#ifdef batPin
    if (msg == "bat")
    {
        return String((analogRead(batPin)) * 3600 / 4095 * 2 + 100);
    }
#endif

    if (msg.substring(0, 4) == "pxbr")
    {

        String param = msg.substring(5);
        param.trim();
        gbright = param.toInt();
        return "led brightness : " + param;
    }
    if (msg.substring(0, 3) == "txt")
    {
        String param = msg.substring(4);
        param.toUpperCase();
        if (param.substring(0, 1) == "~")
        {
            param = utf8rus(param.substring(1));
        }
        param.trim();
        ptext = param;
        return "";
    }

    if (msg.substring(0,5) == "servo")  {
        tasks.push_back(msg);
        return "";
    }

    if (msg.substring(0, 4) == "step")
    {
#ifdef STEPPER
        String param = msg.substring(5);
        param.trim();
        std::vector<String> args = ssplit(&param[0], " ");
        uint8_t mot = args[0].toInt();
        stepflag[0] = 0; stepflag[1] = 0;
	    if (args[1].toInt()) stepflag[mot] = true; else stepflag[mot] = false; 

       	switch(mot) {

                case 0:
                            if (args[1].toInt() == 0) stepper0->forceStopAndNewPosition(0); else {
                            stepper0->setSpeedInUs(args[2].toInt());  // the parameter is us/step !!!
                            stepper0->setAcceleration(args[3].toInt());
                            stepper0->move(args[1].toInt());
                            }
                break;
                case 1:
                            if (args[1].toInt() == 0) stepper1->forceStopAndNewPosition(0); else {
                            stepper1->setSpeedInUs(args[2].toInt());  // the parameter is us/step !!!
                            stepper1->setAcceleration(args[3].toInt());
                            stepper1->move(args[1].toInt());
                            }
                break;
        }   


        return "";
#endif

 

}

    if (msg.substring(0, 4) == "gpio")
    {
        String param = msg.substring(5);
        param.trim();
        std::vector<String> args = ssplit(&param[0], " ");
        uint8_t pin = args[1].toInt();
        if (args[0] == "mode")
        {
            if (args[2] == "input_pulldown")
                pinMode(pin, INPUT_PULLDOWN);
            else if (args[2] == "input_pullup")
                pinMode(pin, INPUT_PULLUP);
            else if (args[2] == "input")
                pinMode(pin, INPUT);
            else if (args[2] == "output")
                pinMode(pin, OUTPUT);
            else if (args[2] == "analog")
                ledcAttachPin(pin, 0);
            args[2].toUpperCase();
            return "GPIO " + String(pin) + " MODE : " + args[2];
        }
        if (args[0] == "read")
        {
            return "GPIO " + String(pin) + ": " + String(digitalRead(pin));
        }
        if (args[0] == "aread")
        {
            return "GPIO " + String(pin) + ": " + String(analogRead(pin));
        }

        if (args[0] == "write")
        {
            digitalWrite(pin, args[2].toInt());
            args[2].toUpperCase();
            return "GPIO " + String(pin) + ": " + args[2];
        }

        if (args[0] == "analog")
        {
            analogWrite((uint8_t)args[1].toInt(), (uint8_t)args[2].toInt());
            args[2].toUpperCase();
            return "GPIO " + String(pin) + ": " + args[2];
        }

        return "";
        //          return String((analogRead(batPin)) * 3600 / 4095 * 2 + 100);
    }

    if (msg == "time")
    {
        //            if (xdisplay) disp(String(printDateTime()));
        return String(printDateTime()).c_str();
    }

if (msg == "xtime" || msg=="now")
    {
        //            if (xdisplay) disp(String(printDateTime()));
        return String(now()).c_str();
    }

    if (msg.substring(0, 6) == "uptime")
    {
        if (msg.substring(7) == "sec")
            return "System running (seconds): " + String(runtime);
        else
            return "System running: " + secTime(runtime);
    }
    if (msg.substring(0, 3) == "dls")
    {
        //            sdelay = msg.substring(4).toInt();
        //            return "delay set: "+String(sdelay);
    }

    if (msg.substring(0, 5) == "sleep")
    {
        timesleep = msg.substring(6).toInt(); // param.trim();
        sendEvent("Sleeping for every " + String(timesleep) + " seconds");
        vTaskDelay(500);
        esp_sleep_enable_timer_wakeup(timesleep * 1000000);
        esp_deep_sleep_start();
    }

    if (msg.substring(0, 3) == "cat")
    {
        String path = msg.substring(4);
        path.trim();
        if (path.substring(0, 1) != "/")
            path = "/" + path;
        if (!LittleFS.exists(path))
            return "File not found :" + path;
        File f = LittleFS.open(path, "r");
        while (f.available())
            sendEvent(f.readStringUntil('\n'));
        f.close();
        return "";
    }

    if (msg == "pnp")
    {
        tasks.push_back("pnp");
        return "";
    }
    if (msg == "dev")
    {
        mysensors->getList();
        return "";
    }

    if (msg == "bme")
    {
#ifdef BME
        BMESensor.refresh();
        if (i2ctouch)
        {
            i2ctouch = false;
            BMESensor.refresh();
        }
        if (BMESensor.temperature > 90 || BMESensor.temperature < -30 || BMESensor.humidity > 110 || BMESensor.humidity < 5 || BMESensor.pressure > 1100 || BMESensor.pressure < 900)
            BMESensor.refresh();

        sendEvent("Temperature: " + String(BMESensor.temperature) + "C"); // display temperature in Celsius
        sendEvent("Humidity:    " + String(BMESensor.humidity) + "%");
        sendEvent("Pressure:    " + String(BMESensor.pressure / 100.0F) + "hPa"); // display pressure in hPa
        return "";
#endif
        return "Sendor not inited";
    }
    if (msg == "ws")
    {
        clid = clidx;
        return "Starting... ";
    }
    if (msg == "clid")
        return String(clid);
    //         if (msg == "mic")  micstart = !micstart;

    if (msg.substring(0, 5) == "debug")
    {
        debug = msg.substring(6).toInt();
        return "DEBUG SET : " + String(debug);
    }

    return "command not found.{"+msg+"}";
}
