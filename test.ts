// Test program for DHT11 sensor
// Connect DHT11 data pin to P2

// Initialize serial
serial.redirectToUSB()

basic.forever(function () {
  serial.writeLine("===== DHT11 Test =====")

  let temp = dht11.readTemperature(DigitalPin.P2)
  let humidity = dht11.readHumidity(DigitalPin.P2)

  serial.writeLine("Temperature: " + temp + " C")
  serial.writeLine("Humidity: " + humidity + " %")

  if (dht11.hasValidData()) {
    serial.writeLine("Status: Valid data")
    basic.showIcon(IconNames.Yes)
  } else {
    serial.writeLine("Status: No valid data")
    basic.showIcon(IconNames.No)
  }

  basic.pause(3000)
})

