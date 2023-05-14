#include <Arduino.h>
#include <WiFi.h>
#include <OSCMessage.h>
#include "GY521.h"
#include "./config.h"

int ID = 1;

WiFiUDP Udp;
GY521 sensor(0x68);

// piezo
int piezoValue;
// angle
float angleX;
float angleY;
float angleZ;
// accel
float accelX;
float accelY;
float accelZ;
// temperature
float temperature;

void readValues()
{
  // piezo
  piezoValue = analogRead(PIEZO_PIN);
  sensor.read();
  // angle
  angleX = sensor.getAngleX();
  angleY = sensor.getAngleY();
  angleZ = sensor.getAngleZ();
  // accel
  accelX = sensor.getAccelX();
  accelY = sensor.getAccelY();
  accelZ = sensor.getAccelZ();
  // temperature
  temperature = sensor.getTemperature();
}

void processValues()
{
  // TODO: do some onset detection based on piezoValue
  // or maybe even accelerometer data
}

void sendOSC()
{
  // piezo
  auto piezoAddress = String("/sleeper/") + String(ID) + String("/piezo");
  OSCMessage piezoMessage(piezoAddress.c_str());
  piezoMessage.add(piezoValue);
  Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
  piezoMessage.send(Udp);
  Udp.endPacket();
  piezoMessage.empty();

  // angle
  auto angleAddress = String("/sleeper/") + String(ID) + String("/angle");
  OSCMessage angleMessage(angleAddress.c_str());
  angleMessage.add(angleX);
  angleMessage.add(angleY);
  angleMessage.add(angleZ);
  Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
  angleMessage.send(Udp);
  Udp.endPacket();
  angleMessage.empty();

  // accel
  auto accelAddress = String("/sleeper/") + String(ID) + String("/accel");
  OSCMessage accelMessage(accelAddress.c_str());
  accelMessage.add(accelX);
  accelMessage.add(accelY);
  accelMessage.add(accelZ);
  Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
  accelMessage.send(Udp);
  Udp.endPacket();
  accelMessage.empty();

  // temperature
  auto temperatureAddress = String("/sleeper/") + String(ID) + String("/temperature");
  OSCMessage temperatureMessage(temperatureAddress.c_str());
  temperatureMessage.add(temperature);
  Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
  temperatureMessage.send(Udp); // Send the bytes to the SLIP stream
  Udp.endPacket();              // Mark the end of the OSC Packet
  temperatureMessage.empty();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("setting up wifi with the following credentials");
  String output = "SSID: " + String(SSID) + "\npassword: " + String(PASSWORD);
  Serial.println(output);

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  Serial.println();
  Serial.println(__FILE__);
  Serial.print("GY521_LIB_VERSION: ");
  Serial.println(GY521_LIB_VERSION);

  Wire.begin();

  delay(100);
  while (sensor.wakeup() == false)
  {
    Serial.print(millis());
    Serial.println("\tCould not connect to GY521");
    delay(1000);
  }
  sensor.setAccelSensitivity(2); //  8g
  sensor.setGyroSensitivity(1);  //  500 degrees/s

  sensor.setThrottle();
  Serial.println("start...");

  //  set calibration values from calibration sketch.
  sensor.axe = 0.574;
  sensor.aye = -0.002;
  sensor.aze = -1.043;
  sensor.gxe = 10.702;
  sensor.gye = -6.436;
  sensor.gze = -0.676;
}

void loop()
{
  readValues();
  processValues();
  sendOSC();
  delay(10);
}