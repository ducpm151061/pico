#include "ccs811.h"

int main() {
  stdio_init_all();
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  CCS811 ccs811(CCS811_ADDR);
  printf("begin: %d", ccs811.begin());

  while (true) {
    gpio_put(LED_PIN, 1);
    sleep_ms(250);
    if (ccs811.dataAvailable()) {
      ccs811.readAlgorithmResults();
      printf("CO2[");
      printf("%d", ccs811.getCO2());
      printf("] tVOC[");
      printf("%d", ccs811.getTVOC());
      printf("]");
      printf("\n");
    }
    gpio_put(LED_PIN, 0);
    sleep_ms(250);
  }
}
