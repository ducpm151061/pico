
#define LED_PIN PICO_DEFAULT_LED_PIN
#define DHT_PIN 15
#define MAX_TIMINGS 85

typedef struct {
    float humidity;
    float temp_celsius;
} dht_reading;

void read_from_dht(dht_reading *result);

