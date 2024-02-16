# Temperature and Humidity monitoring system

The repository contains the scripts that are used to program an ATOM-Lite from M5Stack.

## Highlights
- Uses MODBUS RTU over RS485 to read data from the sensors attached.
- Microcontroller used is a [Atom-Lite](https://shop.m5stack.com/products/atom-lite-esp32-development-kit) from M5Stack.
- The sensors used were [TSH300v2](https://www.paralanstore.net/teracom-systems/sensor-modbus-rtu/teracom-digital-humidity-and-temperature-sensor-tsh300v2) and [TST300v2](https://www.paralanstore.net/teracom-systems/sensor-modbus-rtu/teracom-digital-temperature-sensor-tst300v2).
- The data were collected sequentially from the sensors over a predetermined time interval.
- Once data were collected it was sent to AWS servers over MQTT.
- Number of sensors in the setup is 6
    - More sensors can be added by updating the variable ```const uint8_t num_sensors = ?```.
    - For the above update, sensors should be of similar type.
- Fail-safe has been implemented for the cases of WiFi disconnect or AWS disconnect.

## MQTT Topic Details
- Publishing topic `b1164/01/temperature_humidity`
    - where `01` is the Device ID. The IoT devices sending data to the AWS servers were categorized by the Device ID
- Subscribing topic `b1164/01/parameters`
    - Topic to control the IoT devices from the cloud

## Running the Code
### Steps
- Use [vscode](https://code.visualstudio.com/)  with [PlatformIO](https://platformio.org/)
- Plug in an Atom-Lite through USB
- Upload the program using the vscode's platformIO interface
- Open Serial Monitor for verification 

### Consideration
- Change the topics and AWS secret keys appropriately for your new IoT device.
    - DO NOT use the one in repo as it is being used by other IoT device. 
- Change the device ID appropriately, if you are uploading to multiple devices.
- Change the WiFi credential appropriately.
    - If using in UWNet, then make sure that the device's Mac Address is added to the UWNet. Otherwise you won't be able to connect to internet.
- Verify the sensor's Slave address.
- Ensure that the ```num_sensors``` variable is set to the right number of sensors.
- Different sensors will have different registers to read. This code is only for the sensor [TSH300v2](https://www.paralanstore.net/teracom-systems/sensor-modbus-rtu/teracom-digital-humidity-and-temperature-sensor-tsh300v2). If you are using other sensors, make sure to do the following
    - Update register addresses
    - Update number of registers to read
    - Update the data conversions requirements

## Contact
For questions reach out to vselvaraj@wisc.edu



