#ifdef I2S 
#include <driver/i2s.h>
const i2s_port_t I2S_PORT = I2S_NUM_0;
const int BLOCK_SIZE = 128;
const int headerSize = 44;
bool mstarted = false;

uint16_t xscale(uint8_t* s_buff, uint32_t len, uint16_t sdelay);


#define I2S_WS 12
#define I2S_SD 15
#define I2S_SCK 13
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE   (22050)
//#define I2S_SAMPLE_RATE   (44100)
#define I2S_SAMPLE_BITS   (16)
#define I2S_READ_LEN      (5 * 1024)
#define I2S_CHANNEL_NUM   (1)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

int i2s_read_len = I2S_READ_LEN;

bool i2sinited = false;
 
void i2s_off() {
  if (i2sinited) i2sinited = false; else return;
  i2s_driver_uninstall(I2S_PORT);
}
void i2sInit(){
  if (!i2sinited) i2sinited = true; else return;
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = srate,
    .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BITS),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0,
//    .dma_buf_count = 64,
//    .dma_buf_len = 1024,
    .dma_buf_count = 32,
    .dma_buf_len = 64,
    .use_apll = 1
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
//  trace(F("I2S MIC started"));
}

  char* i2s_read_buff = (char*) calloc(i2s_read_len, sizeof(char));

static void i2s_adc_task(void *arg)
{
//  uint8_t points = 0;
 if (!isadc) isadc = true;
 i2sInit();
 vTaskDelay(40);
 
  size_t bytes_read;
    Serial.println("ADC STARTED");
    while (1) {
      
      if (clid) i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
      else i2s_read(I2S_PORT, (void*) i2s_read_buff, 1458, &bytes_read, portMAX_DELAY);

    if (clid) {ws.binary(clid, (uint8_t*)i2s_read_buff, xscale((uint8_t*)i2s_read_buff, i2s_read_len, sdelay));
    } else {
      udp.beginPacket(udpAddress.c_str(), 9002);
      udp.write((uint8_t*)i2s_read_buff,1458);
      udp.endPacket();
    }
    }
    free(i2s_read_buff);
    i2s_read_buff = NULL;
}

time_t oldxxx = 0;

uint8_t buffer[50] = "hello world";

uint16_t xscale(uint8_t* s_buff, uint32_t len, uint16_t sdelay) {
    if (sdelay<2) return len;
    
    uint16_t dac_value = 0;
    uint8_t xdrop = len/sdelay;
    uint32_t wpoint = 0;
    uint8_t xstep = 0;
    for (int i = 0; i < len; i += 2) {
     xstep++;
      if (xstep == xdrop) {xstep = 0; continue;}
    dac_value = (uint16_t)((s_buff[i]<<8)+s_buff[i+1]);
     s_buff[wpoint+0] =  (dac_value & 0xff00)>>8;
     s_buff[wpoint+1] = (dac_value & 0xff);
     wpoint +=2;
    }
    return wpoint-2;
}
#endif