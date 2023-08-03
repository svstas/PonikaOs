#include <AsyncUdp.h>
#include <Adafruit_NeoPixel.h>
#define PIN            14  //pin de salida D1
#define NUMPIXELS      768  //numero de pixeles a controlar

Adafruit_NeoPixel xpixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned int localPort = 65506;      // local port to listen for UDP packets
const int PACKET_SIZE = 1357;
byte packetBuffer[PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
AsyncUDP udp;

typedef struct
{
      byte r;
      byte g;
      byte b;
} colorpin;

colorpin led;
int led_index = 0;

void pudp(void *arg)
{

    /*
      udp.begin(localPort);
      int cb = udp.parsePacket();
      if (!cb)
      {
            //    Serial.setDebugOutput(true);
      }
      else
      {
            // We've received a packet, read the data from it
            udp.read(packetBuffer, PACKET_SIZE); // read the packet into the buffer
            if (cb >= 6 && packetBuffer[0] == 0x9C)
            {
                  // header identifier (packet start)
                  byte blocktype = packetBuffer[1]; // block type (0xDA)
                  unsigned int framelength = ((unsigned int)packetBuffer[2] << 8) | (unsigned int)packetBuffer[3]; // frame length (0x0069) = 105 leds
                  //    Serial.print("Frame.");
                  //    Serial.println(framelength); // chan/block
                  byte packagenum = packetBuffer[4];   // packet number 0-255 0x00 = no frame split (0x01)
                  byte numpackages = packetBuffer[5];   // total packets 1-255 (0x01)

                  if (blocktype == 0xDA)
                  {
                        // data frame ...
                        //        Serial.println("command");

                        int packetindex;

                        if (cb >= framelength + 7 && packetBuffer[6 + framelength] == 0x36)
                        {
                              // header end (packet stop)
                              //Serial.println("s:");
                              int i = 0;
                              packetindex = 6;
                              if (packagenum == 1)
                              {
                                    led_index = 0;
                              }
                              while (packetindex < (framelength + 6))
                              {
                                    led.r = ((int)packetBuffer[packetindex]);
                                    led.g = ((int)packetBuffer[packetindex + 1]);
                                    led.b = ((int)packetBuffer[packetindex + 2]);
                                    xpixels.setPixelColor(led_index, led.r, led.g, led.b);
                                    led_index++;
                                    Serial.println(led_index);

                                    packetindex += 3;
                              }
                        }
                        //  Serial.print(packagenum);
                        //  Serial.print("/");
                        //  Serial.println(numpackages);

                  }

                  if ((packagenum == numpackages) && (led_index == NUMPIXELS))
                  {
                        pixels.show();
                        led_index == 0;
                  }

            }

      }
*/
}
