//based off of https://editor.soulmatelights.com/gallery/807-magma
#include <FastLED.h>
#define LED_PIN     14
#define BRIGHTNESS  100
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
const uint8_t kMatrixWidth  = 16;
const uint8_t kMatrixHeight = 48;
#define WIDTH kMatrixWidth
#define HEIGHT kMatrixHeight
#define COLS kMatrixHeight
#define ROWS kMatrixWidth
CRGB leds[kMatrixWidth * kMatrixHeight];
const bool    kMatrixSerpentineLayout = true;
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define enlargedOBJECT_MAX_COUNT WIDTH*2
#define SPEED_ADJ (float)NUM_LEDS/512

int brightness = gbright;

//speed control
byte speed = 127; // 1-255

//scale control
byte xscale = 150; //1-255

//control magma bursts
const byte deltaValue = 6U; 
const byte deltaHue = 8U; 

uint16_t XY(uint8_t x, uint8_t y);

static const TProgmemRGBPalette16 MagmaColor_p FL_PROGMEM = {CRGB::Black, 0x240000, 0x480000, 0x660000, 0x9a1100, 0xc32500, 0xd12a00, 0xe12f17, 0xf0350f, 0xff3c00, 0xff6400, 0xff8300, 0xffa000, 0xffba00, 0xffd400, 0xffffff};                     
extern const TProgmemRGBPalette16 WoodFireColors_p FL_PROGMEM = {CRGB::Black, 0x330e00, 0x661c00, 0x992900, 0xcc3700, CRGB::OrangeRed, 0xff5800, 0xff6b00, 0xff7f00, 0xff9200, CRGB::Orange, 0xffaf00, 0xffb900, 0xffc300, 0xffcd00, CRGB::Gold};    
extern const TProgmemRGBPalette16 NormalFire_p FL_PROGMEM = {CRGB::Black, 0x330000, 0x660000, 0x990000, 0xcc0000, CRGB::Red, 0xff0c00, 0xff1800, 0xff2400, 0xff3000, 0xff3c00, 0xff4800, 0xff5400, 0xff6000, 0xff6c00, 0xff7800};                    
extern const TProgmemRGBPalette16 NormalFire2_p FL_PROGMEM = {CRGB::Black, 0x560000, 0x6b0000, 0x820000, 0x9a0011, CRGB::FireBrick, 0xc22520, 0xd12a1c, 0xe12f17, 0xf0350f, 0xff3c00, 0xff6400, 0xff8300, 0xffa000, 0xffba00, 0xffd400};             
extern const TProgmemRGBPalette16 LithiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x240707, 0x470e0e, 0x6b1414, 0x8e1b1b, CRGB::FireBrick, 0xc14244, 0xd16166, 0xe08187, 0xf0a0a9, CRGB::Pink, 0xff9ec0, 0xff7bb5, 0xff59a9, 0xff369e, CRGB::DeepPink}; 
extern const TProgmemRGBPalette16 SodiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x332100, 0x664200, 0x996300, 0xcc8400, CRGB::Orange, 0xffaf00, 0xffb900, 0xffc300, 0xffcd00, CRGB::Gold, 0xf8cd06, 0xf0c30d, 0xe9b913, 0xe1af1a, CRGB::Goldenrod};    
extern const TProgmemRGBPalette16 CopperFireColors_p FL_PROGMEM = {CRGB::Black, 0x001a00, 0x003300, 0x004d00, 0x006600, CRGB::Green, 0x239909, 0x45b313, 0x68cc1c, 0x8ae626, CRGB::GreenYellow, 0x94f530, 0x7ceb30, 0x63e131, 0x4bd731, CRGB::LimeGreen}; 
extern const TProgmemRGBPalette16 AlcoholFireColors_p FL_PROGMEM = {CRGB::Black, 0x000033, 0x000066, 0x000099, 0x0000cc, CRGB::Blue, 0x0026ff, 0x004cff, 0x0073ff, 0x0099ff, CRGB::DeepSkyBlue, 0x1bc2fe, 0x36c5fd, 0x51c8fc, 0x6ccbfb, CRGB::LightSkyBlue};
extern const TProgmemRGBPalette16 RubidiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x0f001a, 0x1e0034, 0x2d004e, 0x3c0068, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, 0x3c0084, 0x2d0086, 0x1e0087, 0x0f0089, CRGB::DarkBlue};  
extern const TProgmemRGBPalette16 PotassiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x0f001a, 0x1e0034, 0x2d004e, 0x3c0068, CRGB::Indigo, 0x591694, 0x682da6, 0x7643b7, 0x855ac9, CRGB::MediumPurple, 0xa95ecd, 0xbe4bbe, 0xd439b0, 0xe926a1, CRGB::DeepPink}; 

static double fmap(const double x, const double in_min, const double in_max, const double out_min, const double out_max){
    return (out_max - out_min) * (x - in_min) / (in_max - in_min) + out_min;
}

float randomf(float min, float max)
{
  return fmap(random(1024), 0, 1023, min, max);
}

void drawPixelXYF(float x, float y, CRGB color)
{
  if (x < 0 || y < 0 || x > ((float)WIDTH - 1) || y > ((float)HEIGHT - 1)) return;
  uint8_t xx = (x - (int)x) * 255, yy = (y - (int)y) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  #define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    int16_t xn = x + (i & 1), yn = y + ((i >> 1) & 1);
    CRGB clr = leds[XY(xn, yn)];
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
    leds[XY(xn, yn)] = clr;
  }
}


// (c) Сотнег (SottNick) 2021
class EffectMagma {
private:    
    float ff_y, ff_z;        
    uint8_t step, ObjectNUM = WIDTH; 
    uint8_t shiftHue[HEIGHT];
    float trackingObjectPosX[enlargedOBJECT_MAX_COUNT];
    float trackingObjectPosY[enlargedOBJECT_MAX_COUNT];
    uint8_t trackingObjectHue[enlargedOBJECT_MAX_COUNT];
    float trackingObjectSpeedX[enlargedOBJECT_MAX_COUNT];
    float trackingObjectShift[enlargedOBJECT_MAX_COUNT];
    float speedfactor;

    void regen();
    void LeapersMove_leaper(uint8_t l);
    void LeapersRestart_leaper(uint8_t l);

public:
    void load();
    bool run();
};


void EffectMagma::load() {
  speedfactor = fmap(speed, 1, 255, SPEED_ADJ*0.1, SPEED_ADJ);
  ObjectNUM = map(xscale, 1, 255, WIDTH, enlargedOBJECT_MAX_COUNT);
  regen();
}


void EffectMagma::regen() {
  for (uint8_t j = 0; j < HEIGHT; j++) {
    shiftHue[j] = map(j, 0, HEIGHT+HEIGHT/4, 255, 0); // init colorfade table
  }


  for (uint8_t i = 0 ; i < enlargedOBJECT_MAX_COUNT ; i++) {
    LeapersRestart_leaper(i);  
    trackingObjectHue[i] = 50U;
  }
}
bool EffectMagma::run() {
  fadeToBlackBy(leds, NUM_LEDS, 25);

  for (uint8_t i = 0; i < ObjectNUM; i++) {
    LeapersMove_leaper(i);
    drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], ColorFromPalette(CopperFireColors_p, trackingObjectHue[i], 255));
  }

  for (uint8_t i = 0; i < WIDTH; i++) {
    for (uint8_t j = 0; j < HEIGHT; j++) {
      leds[XY(i, HEIGHT-1 - j)] += ColorFromPalette(RubidiumFireColors_p, qsub8(inoise8(i * deltaValue, (j + ff_y + random8(2)) * deltaHue, ff_z), shiftHue[j]), 255U);
    }
  }

  ff_y += speedfactor * 2;
  ff_z += speedfactor;
  blur2d(leds, WIDTH, HEIGHT,4 );
  return true;
}

void EffectMagma::LeapersMove_leaper(uint8_t l) {
#define GRAVITY            0.1
  trackingObjectShift[l] -= GRAVITY * speedfactor;

  trackingObjectPosX[l] += trackingObjectSpeedX[l] * speedfactor;
  trackingObjectPosY[l] += trackingObjectShift[l] * speedfactor;

  // bounce off the ceiling?
  if (trackingObjectPosY[l] > HEIGHT + HEIGHT/4) {
    trackingObjectShift[l] = -trackingObjectShift[l];
  }
  
  // settled on the floor?
  if (trackingObjectPosY[l] <= (HEIGHT/8-1)) {
    LeapersRestart_leaper(l);
  }

  // bounce off the sides of the screen?
  if (trackingObjectPosX[l] < 0 || trackingObjectPosX[l] > WIDTH-1) {
    LeapersRestart_leaper(l);
  }

}

void EffectMagma::LeapersRestart_leaper(uint8_t l) {
  randomSeed(millis());
  // leap up and to the side with some random component
  trackingObjectSpeedX[l] = randomf(-0.75, 0.75);
  trackingObjectShift[l] = randomf(0.50, 0.85);
  trackingObjectPosX[l] = randomf(0, WIDTH);
  trackingObjectPosY[l] = randomf(0, (float)HEIGHT/4-1);

  // for variety, sometimes go 100% faster
  if (random8() < 12) {
    trackingObjectShift[l] += trackingObjectShift[l] * 2;
  }

}

bool load = true;
EffectMagma eff;


void draw() {
  if (load) {
    eff.load();
    load = false;
  }
  eff.run();
  //FastLED.delay(16);
}

uint16_t XY(uint8_t x, uint8_t y)
{
  uint16_t i;
  if (kMatrixSerpentineLayout == false)
  {
    i = (y * kMatrixWidth) + x;
  }
  if (kMatrixSerpentineLayout == true)
  {
    if (y & 0x01)
    {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    }
    else
    {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  return i;
}



void plasma(void *arg){

  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);  


for(;;) {

if (gbright!=brightness) {
  brightness=gbright;    LEDS.setBrightness(brightness);  
}

draw();
LEDS.show();
vTaskDelay(10);
}
}


