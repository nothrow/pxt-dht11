#include "pxt.h"

// do not allow polling more often than 2 seconds
#define DHT11_MIN_POLL_INTERVAL 2000
#define POLL_TIMEOUT_MS 500 // 0.5 seconds
#define POLL_TIMEOUT_PULSE_US 200 // 200 microseconds per bit (50us low + up to 70us high + margin)

#ifndef NO_LOGGING
#define LOG(...) uBit.serial.printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

namespace dht11 {

  static float temperature_;
  static float humidity_;
  static uint32_t last_poll_time_;
  static bool has_valid_data_;

  static void busy_loop_us(uint32_t us);
  static void poll(MicroBitPin* pin);
  static void try_poll(int pin_number);

  static void write_pin(MicroBitPin* name, int value);
  static void set_pullup(MicroBitPin* pin);
  static int read_pin(MicroBitPin* pin);
  // return the duration of the pulse in microseconds
  static int pulse_in(MicroBitPin* pin);
  
  //%
  float read_temperature(int digitalPin) {
    try_poll(digitalPin);
    return temperature_;
  }

  //%
  float read_humidity(int digitalPin) {
    try_poll(digitalPin);
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

  // Simple busy-loop delay function using system timer
  static void busy_loop_us(uint32_t us) {
    uint64_t start = system_timer_current_time_us();
    while ((system_timer_current_time_us() - start) < us);
  }

  static void poll(MicroBitPin* pin) {
    last_poll_time_ = uBit.systemTime();

    uint8_t data[5] = {0};

    write_pin(pin, 0);
    busy_loop_us(18000);  // 18ms LOW pulse

    set_pullup(pin);
    read_pin(pin);
    
    busy_loop_us(40);  // 40µs HIGH pulse

    if (read_pin(pin) == 1) {
      // not responding
      return;
    }
    
    uint64_t timeoutAt = system_timer_current_time_us() + POLL_TIMEOUT_MS * 1000;
#define _CHECK_TIMEOUT() do { if (system_timer_current_time_us() > timeoutAt) { return; } } while (0)

    // wait for sensor to pull the pin up (response end)
    while (read_pin(pin) == 0) {
      _CHECK_TIMEOUT();
    }

    while (read_pin(pin) == 1) {
      _CHECK_TIMEOUT();
    }

    int measurements[40] = {0};
    int pin_states[40] = {0};
    bool success = true;

    // 40 bits
    for (int i = 0; i < 40; i++) {
      // Check pin state BEFORE calling pulse_in
      pin_states[i] = read_pin(pin);
      
      uint32_t duration = pulse_in(pin);

      measurements[i] = duration;
      
      // If timeout, abort reading
      if (duration == 0) {
        uBit.serial.printf("DHT11: Timeout on bit %d (pin was %d before pulse_in)\r\n", i, pin_states[i]);
        success = false;
        break;
      }

      data[i >> 3] <<= 1;
      // Threshold: 0=~28us, 1=~70us, use 40us as threshold
      if (duration > 40) data[i >> 3] |= 1;
    }

    for(int i = 0; i < 40; i++) {
      if (measurements[i] > 0 || i == 0) {
        uBit.serial.printf("Bit %d: pin_before=%d, duration=%d us\r\n", i, pin_states[i], measurements[i]);
      }
    }
    
    if (!success) {
      uBit.serial.printf("DHT11: Failed to read all 40 bits\r\n");
      return;
    }

    uBit.serial.printf("DHT11: Raw data: %d %d %d %d %d\r\n", 
                       data[0], data[1], data[2], data[3], data[4]);

    if (((data[0] + data[1] + data[2] + data[3]) & 0xff) != data[4]) {
      // checksum error
      uint8_t expected = (data[0] + data[1] + data[2] + data[3]) & 0xff;
      uBit.serial.printf("DHT11: Checksum error! Expected: %d, Got: %d\r\n", 
                         expected, data[4]);
      return;
    }

    temperature_ = data[2] + data[3] * 0.1f;
    humidity_ = data[0] + data[1] * 0.1f;
    has_valid_data_ = true;
    
    uBit.serial.printf("DHT11: Success! Temp=%d.%d C, Humidity=%d.%d %%\r\n", 
                       data[2], data[3], data[0], data[1]); 
  }

  static void try_poll(int digitalPin) {
    if (has_stale_data()) {
      poll(getPin(digitalPin));
    }
  }

  static void write_pin(MicroBitPin* pin, int value) {
    pin->setDigitalValue(value);
  }

  static void set_pullup(MicroBitPin* pin) {
#if MICROBIT_CODAL
    pin->setPull(codal::PullMode::Up);
#else
    pin->setPull(PinMode::PullUp);
#endif
  }

  static int read_pin(MicroBitPin* pin) {
    return pin->getDigitalValue();
  }


  static int pulse_in(MicroBitPin* pin) {
#if MICROBIT_CODAL
    // set polarity
    pin->setPolarity(1);
    // record pulse
    int period = pin->getPulseUs(POLL_TIMEOUT_PULSE_US);
    // timeout
    if (DEVICE_CANCELLED == period)
        return 0;
    // success!
    return period;
#else
    // DHT11 protocol: LOW (50us) then HIGH (28us or 70us)
    // Ensure we start from LOW state
    uint64_t sync_start = system_timer_current_time_us();
    
    // If pin is HIGH, wait for it to go LOW first
    while(pin->getDigitalValue() == 1) {
        if(system_timer_current_time_us() - sync_start > POLL_TIMEOUT_PULSE_US)
            return 0;
    }
    
    // Now wait for LOW pulse to end (pin goes HIGH)
    uint64_t wait_start = system_timer_current_time_us();
    while(pin->getDigitalValue() == 0) {
        if(system_timer_current_time_us() - wait_start > POLL_TIMEOUT_PULSE_US)
            return 0;
    }

    // Measure HIGH pulse duration
    uint64_t pulse_start = system_timer_current_time_us();
    while(pin->getDigitalValue() == 1) {
        if(system_timer_current_time_us() - pulse_start > POLL_TIMEOUT_PULSE_US)
            return 0;
    }
    uint64_t pulse_end = system_timer_current_time_us();
    return pulse_end - pulse_start;
#endif
  }

} // namespace dht11
