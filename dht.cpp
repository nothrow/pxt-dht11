#include "pxt.h"
#include "nrf_gpio.h"
#include "us_ticker_api.h"

// do not allow polling more often than 2 seconds
#define DHT11_MIN_POLL_INTERVAL 2000
#define POLL_TIMEOUT 500 // 0.5 seconds

namespace dht11 {

  static float temperature_;
  static float humidity_;
  static uint32_t last_poll_time_;
  static bool has_valid_data_;

  static void busy_loop_us(uint32_t us);
  static void poll(int pin_number);
  static void try_poll(int pin_number);

  //%
  float read_temperature(int pin) {
    try_poll(pin);
    return temperature_;
  }

  //%
  float read_humidity(int pin) {
    try_poll(pin);
    return humidity_;
  }


  //%
  bool has_valid_data() {
    return has_valid_data_;
  }

  //%
  bool has_stale_data() {
    return last_poll_time_ + DHT11_MIN_POLL_INTERVAL < uBit.systemTime();
  }

  // Simple busy-loop delay functions using us_ticker
  static void busy_loop_us(uint32_t us) {
    uint32_t start = us_ticker_read();
    while ((us_ticker_read() - start) < us);
  }

  static void poll(int pin_number) {
    last_poll_time_ = uBit.systemTime();

    uint8_t data[5] = {0};

    nrf_gpio_cfg_output(pin_number);

    nrf_gpio_pin_clear(pin_number);
    busy_loop_us(18000);  // 18ms LOW pulse

    nrf_gpio_pin_set(pin_number);

    nrf_gpio_cfg_input(pin_number, NRF_GPIO_PIN_PULLUP);
    busy_loop_us(40);  // 40µs HIGH pulse

    if (nrf_gpio_pin_read(pin_number) == 1) {
      // not responding
      return;
    }

    uint32_t readStartTime = us_ticker_read();
#define _CHECK_TIMEOUT() do { if (us_ticker_read() - readStartTime > POLL_TIMEOUT * 1000) { return; } } while (0)
#define _CHECK_TIMEOUT_IRQ() do { if (us_ticker_read() - readStartTime > POLL_TIMEOUT * 1000) { __enable_irq(); return; } } while (0)

    // wait for sensor to pull the pin down (response start)
    while (nrf_gpio_pin_read(pin_number) == 1) {
      _CHECK_TIMEOUT();
    }

    // wait for sensor to pull the pin up (response end)
    while (nrf_gpio_pin_read(pin_number) == 0) {
      _CHECK_TIMEOUT();
    }

    while (nrf_gpio_pin_read(pin_number) == 1) {
      _CHECK_TIMEOUT();
    }

    // Disable interrupts for time-critical bit reading
    __disable_irq();

    // 40 bits
    for (int i = 0; i < 40; i++) {
      // Wait for low pulse to end
      while(nrf_gpio_pin_read(pin_number) == 0) {
        _CHECK_TIMEOUT_IRQ();
      }

      // Measure high pulse duration using microsecond timing
      uint32_t start = 0;
      while (nrf_gpio_pin_read(pin_number) == 1) {
        _CHECK_TIMEOUT_IRQ();
        start++;
        busy_loop_us(1);
        if (start > 100) break;
      }

      data[i / 8] <<= 1;
      // Threshold: ~30us for '0', ~70us for '1', so use 50us as threshold
      if (start > 50) data[i / 8] |= 1;
    }

    __enable_irq();

    if (((data[0] + data[1] + data[2] + data[3]) & 0xff) != data[4]) {
      // checksum error
      return;
    }

    temperature_ = data[2] + data[3] * 0.1f;
    humidity_ = data[0] + data[1] * 0.1f;
    has_valid_data_ = true; 
  }

  static void try_poll(int pin_number) {
    if (has_stale_data()) {
      poll(pin_number);
    }
  }
} // namespace dht11
