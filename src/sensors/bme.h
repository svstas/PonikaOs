#include <BME280_t.h>                                                   // import BME280 template library
#define ASCII_ESC 27
#define MYALTITUDE  150.50
char bufout[10];
BME280<> BMESensor; 
bool bmeInited = false;
time_t lastcheck = now();

float getBME(uint8_t tp) {
  if (lastcheck!=now()) {
      BMESensor.refresh();
      lastcheck = now();
      if (BMESensor.temperature>90 || BMESensor.temperature<-30 || BMESensor.humidity>110 || BMESensor.humidity<5 || BMESensor.pressure>1100 || BMESensor.pressure<900) BMESensor.refresh(); 
  }
        switch(tp) {
          case 1: return BMESensor.temperature;
          case 2: return BMESensor.humidity;
          case 3: return BMESensor.pressure / 100.0F;
     }
        return 0;
}
