
#ifdef I2S
TaskHandle_t adcHandler = NULL;
#include <i2s.h>
#endif
bool donow = false;
TaskHandle_t pixHandler = NULL;

#include "pixel.h"
#include "balls.h"
//#include "plasma.h"
#include "fire.h"
#include "udpmatrix.h"



void dotask(bool flag)
{
    if (donow)
        return;
    else
        donow = true;
    String xt;
    if (flag)
    {
        xt = tasks.back();
        tasks.pop_back();
    }
    xt.trim();
    if (xt.substring(0,1)=="~") {
        Serial.println(xt.substring(1));
    }

    else if (xt == "nfc") {
                    sendEvent(getNFC());
    }

    else if (xt == "xmass") {
            xmass = scale.get_units();
    }
    else if (xt == "mass") {
//    sendEvent("Massa!!!");
//    scale.power_up();
    mass = getmass();
    vTaskDelay(200);

    sendEvent(String(mass));
    }

    else if (xt.substring(0,5) == "servo")  {
        String param = xt.substring(6);
        param.trim();
/*        
        std::vector<String> args = ssplit(&param[0], " ");
        int vect = args[0].toInt();   
        int steps = args[1].toInt();
        int dl = args[2].toInt();
*/
        sendEvent("Servo send : " + param);
        vTaskDelay(100);

        servo1.write(param.toInt());
    }


    else if (xt.substring(0, 4) == "step")
    {
#ifdef STEPPER
        String param = xt.substring(5); param.trim();
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


#endif
    }




    else if (xt == "ptext")
    {
        if (pixHandler == NULL)
        {
            sendEvent("PIX started.");
            vTaskDelay(200);
            xTaskCreatePinnedToCore(pixels, "pix", 32192, NULL, 10, &pixHandler, 1);
        }
        else
        {
            sendEvent("PIX OFF");
            vTaskDelete(pixHandler);
            vTaskDelay(100);
            pixHandler = NULL;
        }
    }

    else if (xt == "pudp")
    {
        if (pixHandler == NULL)
        {
            sendEvent("UDP PIX started.");
            vTaskDelay(200);
            xTaskCreatePinnedToCore(pudp, "udp", 32192, NULL, 6, &pixHandler, 1);
        }
        else
        {
            sendEvent("PIX OFF");
            vTaskDelete(pixHandler);
            vTaskDelay(100);
            pixHandler = NULL;
        }
    }

    else if (xt == "pballs")
    {
        if (pixHandler == NULL)
        {
            sendEvent("PIX started.");
            vTaskDelay(200);
            xTaskCreatePinnedToCore(plasma, "plasma", 32192, NULL, 12, &pixHandler, 1);
        }
        else
        {
            sendEvent("PIX OFF");
            vTaskDelete(pixHandler);
            vTaskDelay(100);
            pixHandler = NULL;
        }
    }

    else if (xt == "pfire")
    {
        if (pixHandler == NULL)
        {
            sendEvent("PIX started.");
            vTaskDelay(200);
            xTaskCreatePinnedToCore(fire, "fire", 32192, NULL, 12, &pixHandler, 1);
        }
        else
        {
            sendEvent("PIX OFF");
            vTaskDelete(pixHandler);
            vTaskDelay(100);
            pixHandler = NULL;
        }
    }

#ifdef I2S
    else if (xt == "adc")
    {
        if (adcHandler == NULL)
        {
            xTaskCreatePinnedToCore(i2s_adc_task, "adc", 8192, NULL, 1, &adcHandler, 1);
            sendEvent("Mic started.");
        }
        else
        {
            sendEvent("adcoff", "c");
            vTaskDelete(adcHandler);
            isadc = false;
            vTaskDelay(100);
            adcHandler = NULL;
            i2s_off();
            sendEvent("Mic stopped.");
        }
    }
#endif
    // if (xt=="goff") if (GestHandler != NULL) {vTaskDelete(GestHandler);isgest=false;sendEvent("GESTURE: OFF");}

#ifdef GEST
    else if (xt == "gest")
    {
        if (!isgest)
        {
            sendEvent("GESTURE: ON");
            xTaskCreatePinnedToCore(gesture, "gesture", 2048, NULL, 5, &GestHandler, 1);
            isgest = true;
        }
        else
        {
            sendEvent("GESTURE: OFF");
            vTaskDelete(GestHandler);
            isgest = false;
        }
    }
#endif

    else if (xt.substring(0, 4) == "kill")
    {
        uint8_t ttid = xt.substring(6).toInt();
        sendEvent("kill " + String(ttid));
        // if (!pool.size()) return;
    }

    else if (xt == "pnp")
    {
        sensloaded = false;
        //   vTaskDelay(10);
        mysensors->pnp();
        //     vTaskDelay(10);
        sensloaded = true;
    }
    //#ifdef I2S
    else if (xt == "i2c")
    {
        if (!sensloaded)
        {
            donow = false;
            return;
        }
        sensloaded = false;
        //    vTaskDelay(10);
        scan_i2c();
        //   vTaskDelay(10);
        sensloaded = true;
    }
    //#endif

    else if (xt == "dev")
    {
        if (!sensloaded)
        {
            donow = false;
            return;
        }
        sensloaded = false;
        //    vTaskDelay(10);
        mysensors->List();
        //    vTaskDelay(10);
        sensloaded = true;
    }
    else if (xt == "init")
    {
        if (!sensloaded)
        {
            donow = false;
            return;
        }
        sensloaded = false;
        //    vTaskDelay(10);
        mysensors->xinit();
        //    vTaskDelay(10);
        sensloaded = true;
    }

    // if (xt=="i2s") i2sInit();
    // if (xt=="lora") LoraInit();
    donow = false;
}
