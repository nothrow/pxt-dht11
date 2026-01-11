//% color=#000080 icon="\uf2c9" block="DHT11"
namespace dht11 {
  /**
   * Returns temperature from the DHT11 sensor. Returns 0 if the sensor is not available.
   * Will either return cached value or poll the sensor if the data is stale.
   * Call `dht11.hasStaleData()` to check if the data is stale.
   * This method can be called safely even if the sensor is not available.
   * 
   * Example:
   * ```blocks
   * const temperature = dht11.readTemperature(DigitalPin.P0);
   * ```
   * @param pin The pin to read the temperature from
   * @returns 
   */
  //% block="Read DHT11 temperature"
  //% blockId="read_temperature"
  //% shim=dht11::read_temperature
  export function readTemperature(pin: DigitalPin): number {
    return 25;
  }

  /**
   * Returns humidity from the DHT11 sensor. Returns 0 if the sensor is not available.
   * Will either return cached value or poll the sensor if the data is stale.
   * Call `dht11.hasStaleData()` to check if the data is stale.
   * This method can be called safely even if the sensor is not available.
   * 
   * Example:
   * ```blocks
   * const humidity = dht11.readHumidity(DigitalPin.P0);
   * ```
   * @param pin The pin to read the humidity from
   * @returns The humidity in %
   */
  //% block="Read DHT11 humidity"
  //% blockId="read_humidity"
  //% shim=dht11::read_humidity
  export function readHumidity(pin: DigitalPin): number {
    return 15;
  }

  /**
   * Returns true if the data from the DHT11 sensor is valid -> means that some data has been read from the sensor.
   */
  //% block="Has valid data"
  //% blockId="has_valid_data"
  //% advanced=true
  //% shim=dht11::has_valid_data
  export function hasValidData(): boolean {
    return true;
  }

  /**
   * Returns true if the data from the DHT11 sensor is stale.
   * The data is stale if it has not been polled for at least 2 seconds.
   * @returns True if the data is stale, false otherwise
   */
  //% block="Has stale data"
  //% blockId="has_stale_data"
  //% advanced=true
  //% shim=dht11::has_stale_data
  export function hasStaleData(): boolean {
    return false;
  }
}