#include <Arduino.h>
#include <ModbusRTU.h>
#include <secrets.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Status Checks
bool WIFI_CONNECTED, MQTT_CONNECTED;

// Time variables to sendg data
unsigned long previous_record_time = millis();
const unsigned long interval = 6000UL;

// Define the AWS Topics
#define AWS_IOT_PUBLISH_TOPIC "b1164/01/temperature_humidity"
#define AWS_IOT_SUBSCRIBE_TOPIC "b1164/01/parameters"

// WiFi and MQTT
WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

// Writing an address
// CAUTION: If you turn this to be TRUE, it will overwrite your current address
bool to_write = false;
uint16_t address = 1;
uint8_t device_id = device_id;

// Initialize the number of sensors
const uint8_t num_sensors = 6;
// Forward declaration
union cum_data
{
  uint32_t i;
  float f;
};

// Handle incoming messages - MQTT
void message_handler(String &topic, String &payload) {
  Serial.println("Incoming: " + topic + " -- " + payload);
}

// Connect to AWS
void connect_AWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("+");
  }

  
  // Set the AWS Connectiong parameters
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT Broker on the endpoint
  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  // Set the keep alive timer
  client.setKeepAlive(60);
  // Message handler
  client.onMessage(message_handler);
  // Connect
  Serial.print("Connecting to AWS IOT");
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
  // Check for connection
  if (!client.connected()) {
    Serial.println("AWS Connection Timeout");
    return;
  }
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");

}


void publish_message(union cum_data* t, union cum_data* h) {

  // A JSON to hold the data
  // const int capacity = JSON_OBJECT_SIZE(num_sensors * 2);
  StaticJsonDocument<300> doc;

  // Load the data into JSON file
  // The device mac address
  doc["addr"] = WiFi.macAddress();
  doc["device_id"] = 1;
  for (int i = 0; i < num_sensors; i++) {
    
    if (t[i].f > 1) {
      doc["t" + String(i)] = (float) round((t[i].f) * 100) / 100.0;
    } else {
      doc["t" + String(i)] = "na";
    }

    if (h[i].f > 1) {
      doc["h" + String(i)] = (float) round((h[i].f) * 100) / 100.0;
    } else {
      doc["h" + String(i)] = "na";
    }

  }

  // Process the JSON file
  char JSON_buffer[2048];
  serializeJson(doc, JSON_buffer);

  // Publish the data
  bool publish_status = client.publish(AWS_IOT_PUBLISH_TOPIC, JSON_buffer);
  // Print what is to be published
  Serial.print("The temperature values are being published with a status of (");
  Serial.print(publish_status);
  Serial.println(")");
}


// Get the MODBUS Handler
ModbusRTU mb;

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Callback to monitor errors
  if (event != Modbus::EX_SUCCESS) {
    Serial.print("Request result: 0x");
    Serial.print(event, HEX);
  }
  return true;
}


void read_parse_sensor_data(uint32_t* data, uint8_t address, uint16_t offset, uint16_t num_regs, cbTransaction cb=nullptr) {

  // Placeholders for temperature and humidity
  uint16_t res[4];

  // If the slave is not busy
  if (!mb.slave()) {
    mb.readHreg(address, 100, res, num_regs, cb);

    while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
    }
  }

  // Get the values
  data[0] = (((unsigned long) res[0] << 16) | res[1]);
  data[1] = (((unsigned long) res[2] << 16) | res[3]);

}



void setup() {
  // put your setup code here, to run once:
  // Initialize the serial
  Serial.begin(9600);

  // Connect to AWS
  connect_AWS();

  // Initialize the Serial and MODBUS
  Serial2.begin(19200, SERIAL_8E1, 22, 19);
  mb.begin(&Serial2);
  // Initialize as master
  mb.master();

  // Print some information
  Serial.print("WiFi MAC Address: " );
  Serial.println(WiFi.macAddress());

  // Write the address if required
  if (to_write) {
    // Write the required address
    mb.writeHreg(1, 10, &address, 1, cb);

    // Delay and print
    while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(10);
    }
    delay(5000);
    Serial.print("Address of ");
    Serial.print(address);
    Serial.println(" has been written successfully");
  }
  digitalWrite(27, HIGH);

}

void loop() {

  /*
  Data Collection and transfer
  */
  // Storage and conversion
  union cum_data temperature_data[num_sensors], humidity_data[num_sensors];
  if (millis() - previous_record_time > interval) {

    // Data collection
    uint8_t address = 1;
    for (int i = 0; i < num_sensors; i++) {

      // Read sensors one by one
      uint32_t data[2];

      // Read data from sensor
      read_parse_sensor_data(data, address, 100, 4, cb);

      // Assign values
      temperature_data[i].i = data[0];
      humidity_data[i].i = data[1];

      // Increase the address
      address++;

      // Delay before calling next device
      delay(500);

    }

    // Set out on publishing the data
    publish_message(temperature_data, humidity_data);

    // Update timer
    previous_record_time = millis();

  }


  /*
  Fail-safe for WiFi and MTConnect 
  */ 
  // Get the status
  WIFI_CONNECTED = WiFi.status() == WL_CONNECTED;
  MQTT_CONNECTED = client.connected();

  if (!WIFI_CONNECTED || !MQTT_CONNECTED) {

    Serial.println("Handling the disconnect, either in WiFi or MQTT!!");

    // The WIFI needs to be disconnected and connected
    WiFi.disconnect();
    client.disconnect();
    delay(60000);
    // Connect back to AWS
    connect_AWS();

  }


  /*
  Maintain the MQTT connection
  */
  client.loop();

}