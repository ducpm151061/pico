#include "ds1307.h"

static void printTime(DS1307 *ds1307) {
  ds1307->getTime();
  printf("%d", ds1307->hour);
  printf(":");
  printf("%d", ds1307->minute);
  printf(":");
  printf("%d", ds1307->second);
  printf("	");
  printf("%d", ds1307->month);
  printf("/");
  printf("%d", ds1307->dayOfMonth);
  printf("/");
  printf("%d", ds1307->year + 2000);
  printf(" ");
  printf("%d", ds1307->dayOfMonth);
  printf("*");
  printf("%d", ds1307->dayOfWeek);
  switch (ds1307->dayOfWeek) {
  case MON:
    printf("MON");
    break;
  case TUE:
    printf("TUE");
    break;
  case WED:
    printf("WED");
    break;
  case THU:
    printf("THU");
    break;
  case FRI:
    printf("FRI");
    break;
  case SAT:
    printf("SAT");
    break;
  case SUN:
    printf("SUN");
    break;
  default:
    break;
  }
  printf("\n");
}

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
  stdio_init_all();
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  DS1307 ds1307;
  ds1307.begin();
  ds1307.fillByYMD(2013, 1, 19); // Jan 19,2013
  ds1307.fillByHMS(15, 28, 30);  // 15:28 30"
  ds1307.fillDayOfWeek(SAT);     // Saturday
  ds1307.setTime();
  while (true) {
    printf("LED ON\n");
    gpio_put(LED_PIN, 1);
    sleep_ms(500);
    printTime(&ds1307);
    printf("LED OFF\n");
    gpio_put(LED_PIN, 0);
    sleep_ms(500);
  }
#endif
}