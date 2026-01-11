#include "pxt.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

// do not allow polling more often than 2 seconds
#define DHT11_MIN_POLL_INTERVAL 2000
#define POLL_TIMEOUT 500 // 0.5 seconds

namespace dht11 {

  static void poll(int pin_number) {
    last_poll_time_ = uBit.systemTime();

    uint8_t data[5] = {0};

    nrf_gpio_cfg_output(pin_number);

    __disable_irq();

    nrf_gpio_pin_clear(pin_number);
    nrf_delay_ms(18);

    nrf_gpio_pin_set(pin_number);
    nrf_delay_us(40);

    nrf_gpio_cfg_input(pin_number, NRF_GPIO_PIN_PULL_UP);

    if (nrf_gpio_pin_read(pin_number) == 1) {
      // not responding
      goto end;
    }

    int readStartTime = uBit.systemTime();
#define _CHECK_TIMEOUT() do { if (uBit.systemTime() - readStartTime > POLL_TIMEOUT) { goto end; } } while (0)

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

    // 40 bits
    for (int i = 0; i < 40; i++) {
      while(nrf_gpio_pin_read(pin_number) == 0) {
        _CHECK_TIMEOUT();
      }

      int count = 0;
      while (nrf_gpio_pin_read(pin_number) == 1 && count < 100) count++;

      data[i / 8] <<= 1;
      if (count > 40) data[i / 8] |= 1;
    }

    if (((data[0] + data[1] + data[2] + data[3]) & 0xff) != data[4]) {
      // checksum error
      goto end;
    }

    temperature_ = (data[2] << 8 | data[3]) * 0.1f;
    humidity_ = (data[0] << 8 | data[1]) * 0.1f;
    has_valid_data_ = true; 

end:
    __enable_irq();
  }

  static void try_poll(int pin_number) {
    if (has_stale_data()) {
      poll(pin_number);
    }
  }

  //%
  float read_temperature(DigitalPin pin) {
    try_poll(pin);
    return temperature_;
  }

  //%
  float read_humidity(DigitalPin pin) {
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

  static float temperature_;
  static float humidity_;
  static int last_poll_time_;
  static bool has_valid_data_;
} // namespace dht11
