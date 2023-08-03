#ifdef BUTTON
#define ESP_INTR_FLAG_DEFAULT 0
#define BUTTON_PIN 0
SemaphoreHandle_t semaphore = nullptr;
bool pressed = false;
bool xdisplay = false;
void IRAM_ATTR handler(void* arg) {
  xSemaphoreGiveFromISR(semaphore, NULL);
};
void button_task(void* arg) {
  for(;;) {
    if(xSemaphoreTake(semaphore,portMAX_DELAY) == pdTRUE) {
      xdisplay = !xdisplay;
      sendEvent(String(xdisplay));
    } 
  }
}
void button_init(){
  semaphore = xSemaphoreCreateBinary();
  gpio_pad_select_gpio((gpio_num_t)0);
  gpio_set_direction((gpio_num_t)0, GPIO_MODE_INPUT);
  gpio_set_intr_type((gpio_num_t)0, GPIO_INTR_NEGEDGE);
  xTaskCreate(button_task, "button_task", 4096, NULL, 10, NULL);
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add((gpio_num_t)BUTTON_PIN, handler, NULL);
}
#endif