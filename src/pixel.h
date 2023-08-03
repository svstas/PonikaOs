
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
//#include <utf8rus.h>
//#include "FontsRus/TimesNRCyr8.h"
//#include "Fonts/FreeMono12pt7b.h"

static void pixels(void *arg)
{

#define arr_len( x )  ( sizeof( x ) / sizeof( *x ) )

Adafruit_NeoMatrix matrix =  Adafruit_NeoMatrix(16, 16, 3, 1, 14,
  NEO_TILE_LEFT   + NEO_TILE_TOP   + NEO_TILE_COLUMNS   + NEO_TILE_PROGRESSIVE +
  NEO_MATRIX_LEFT + NEO_MATRIX_TOP + NEO_MATRIX_COLUMNS + NEO_MATRIX_SEQUENCE,
  NEO_GRB + NEO_KHZ800);
String ttt = utf8rus("Свобода! РАВЕНСТВО! БРАТСТВО!!!");

char* slogans[]= {"PASTIRALLA WORKSHOP!!!",&ttt[0],"STAND WITH STANDING ROCK","WATER IS LIFE"};
const uint16_t colors[] = { matrix.Color(255, 0, 100), matrix.Color(0, 128, 128), matrix.Color(0, 200, 128),matrix.Color(128, 0, 128) };
int brightness = gbright;
int numMode = arr_len(slogans)-1;
int numColor = arr_len(colors)-1;
int pixelPerChar = 8;
int maxDisplacement;
int mode = 0;
bool pixx = false;



 matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(brightness);
  matrix.setTextColor(colors[0]);
//  matrix.setFont(&TimesNRCyr8pt8b);
//  matrix.setFont(&FreeMono12pt7b);

int y = matrix.height();
int x = matrix.width();
int pass = 0;
int line_pass = 0;
vTaskDelay(10);
for(;;) {

if (gbright!=brightness) {
  brightness=gbright;  matrix.setBrightness(brightness);}

 if (mode > numMode) { mode = 0; }
  
 
  matrix.fillScreen(0);
  matrix.setCursor(x, 0);

  char * message;

  if (ptext.length()) message = &ptext[0]; else  message = slogans[mode]; 

  maxDisplacement = strlen(message) * pixelPerChar + matrix.width();
 
  if(++line_pass > matrix.width()) line_pass = 0;
  //sendEvent(String(message));
  matrix.print(String(message));
  if(--x < -maxDisplacement) {
    x = matrix.width();
    if(++pass >= numColor) { pass = 0;  };
    matrix.setTextColor(colors[pass]);
    mode++;
  }
  vTaskDelay(4);
  matrix.show();

  vTaskDelay(20);
}
vTaskDelete(NULL);
}



