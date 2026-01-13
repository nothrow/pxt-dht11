# DHT11 for micro:bit

A reliable, easy-to-use DHT11 temperature and humidity sensor library for BBC micro:bit.

## Why This Library?

Existing DHT11 libraries for micro:bit are poorly documented and prone to crashing (hanging the entire micro:bit). This library was built from the ground up to be:

- **Crash-resistant**: Built-in timeout mechanisms prevent your micro:bit from hanging
- **Simple**: Just call `readTemperature()` or `readHumidity()` - the library handles all the low-level sensor communication
- **Safe**: Can be called even if the sensor is disconnected or malfunctioning

## Usage

Add this extension to your micro:bit project and start reading sensor data immediately:

### Basic Example

```typescript
basic.forever(function () {
    let temperature = dht11.readTemperature(DigitalPin.P2)
    let humidity = dht11.readHumidity(DigitalPin.P2)
  
    basic.showString("T:" + temperature + "C")
    basic.showString("H:" + humidity + "%")
    basic.pause(2000)
})
```

### That's It

The library handles all the hard work for you:

- ✅ Low-level sensor communication protocol
- ✅ Bit timing and signal processing
- ✅ Checksum validation
- ✅ Automatic caching (2-second minimum poll interval)
- ✅ Timeout handling to prevent crashes

### Advanced Usage

For more control, you can check if data is stale or valid:

```typescript
if (dht11.hasStaleData()) {
    basic.showString("Data will refresh on next read* call!")
}

if (dht11.hasValidData()) {
    basic.showString("Sensor is working!")
}
```

## Important Note on Timing

Due to the automatic caching system, calls to `readTemperature()` and `readHumidity()` are **not deterministic in execution time**:

- If data is fresh (< 2 seconds old), the cached value is returned immediately ⚡
- If data is stale (≥ 2 seconds old), the sensor is polled which takes ~18-20ms 🕐

This design choice makes the library safer and easier to use, but means you cannot rely on consistent execution timing for these calls.

## Hardware Setup

1. Connect DHT11 VCC to 3.3V
2. Connect DHT11 GND to GND
3. Connect DHT11 DATA to any digital pin (e.g., P0)

## API Reference

### `dht11.readTemperature(pin: DigitalPin): number`

Returns temperature in Celsius. Returns 0 if sensor is unavailable.

### `dht11.readHumidity(pin: DigitalPin): number`

Returns relative humidity in %. Returns 0 if sensor is unavailable.

### `dht11.hasValidData(): boolean`

Returns true if at least one successful sensor reading has been obtained.

### `dht11.hasStaleData(): boolean`

Returns true if cached data is older than 2 seconds.

## License

MIT

## Supported Targets

- micro:bit (v1 and v2)
