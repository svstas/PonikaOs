#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

//#define PIN 14

void prain(void *arg) {

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(48, 16, 14,
  NEO_MATRIX_LEFT    + NEO_MATRIX_TOP +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);

  matrix.begin();
  matrix.setBrightness(30);
  matrix.clear();
  uint16_t i;

for(;;) {
  for(i = 0; i < 256; i++) {
    for(int x = 0; x < matrix.width(); x++) {
      for(int y = 0; y < matrix.height(); y++) {
          uint32_t whl = 0;
          uint32_t wp = ((x+y) * 256 / (8*32) + i) & 255;
  if(wp < 85) {
   whl = matrix.Color(wp * 3, 255 - wp * 3, 0);
  } else if(wp < 170) {
   wp -= 85;
   whl = matrix.Color(255 - wp * 3, 0, wp * 3);
  } else {
   wp -= 170;
   whl = matrix.Color(0, wp * 3, 255 - wp * 3);
  }
          

        matrix.drawPixel(x, y, whl);
      }
    }
    matrix.show();
    vTaskDelay(3);
  }
}
}



