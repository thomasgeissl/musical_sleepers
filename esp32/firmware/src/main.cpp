#include <Arduino.h>
#include <WiFi.h>
#include <OSCMessage.h>
#include "GY521.h"
#include "RunningAverage.h"
#include "./config.h"


WiFiUDP Udp;
GY521 sensor(0x68);

unsigned long frameCounter = 0;
// piezo
int piezoValue;
bool piezoOnsetDetected = false;
unsigned long piezoOnsetTimestamp;
// touch
RunningAverage touchValue(10);
bool touched = false;

// angle
float angleX;
float angleY;
float angleZ;
// accel
float accelX;
float accelY;
float accelZ;
bool accelOnsetDetected = false;
unsigned long accelOnsetTimestamp;
// temperature
float temperature;

void readValues()
{
  // piezo
  piezoValue = analogRead(PIEZO_PIN);
  // touch
  touchValue.addValue(touchRead(TOUCH_PIN));
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
  auto timestamp = millis();
  int16_t accelMagnitude = sqrt(sq(accelX) + sq(accelY) + sq(accelZ));
  if (accelMagnitude > accelOnsetThreshold && accelOnsetThreshold + timestamp > accelOnsetDebounceTime)
  {
    accelOnsetDetected = true;
    accelOnsetTimestamp = timestamp;
  }
  if (piezoValue > piezoOnsetThreshold && timestamp > piezoOnsetTimestamp + piezoOnsetDebounceTime)
  {
    piezoOnsetDetected = true;
    piezoOnsetTimestamp = timestamp;
  }
  touched = touchValue.getAverage() < touchThreshold;
}

void sendOSC()
{

#ifdef USE_PIEZO
  // piezo
  OSCMessage piezoMessage("/sleeper/piezo");
  piezoMessage.add(ID);
  piezoMessage.add(piezoValue);
  Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
  piezoMessage.send(Udp);
  Udp.endPacket();
  piezoMessage.empty();
#endif

#ifdef USE_TOUCH
  // touch
  OSCMessage touchMessage("/sleeper/touch");
  touchMessage.add(ID);
  touchMessage.add(touched);
  Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
  touchMessage.send(Udp);
  Udp.endPacket();
  touchMessage.empty();
#endif

#ifdef USE_ANGLE
  // angle
  OSCMessage angleMessage("/sleeper/angle");
  angleMessage.add(ID);
  angleMessage.add(angleX);
  angleMessage.add(angleY);
  angleMessage.add(angleZ);
  Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
  angleMessage.send(Udp);
  Udp.endPacket();
  angleMessage.empty();
#endif

#ifdef USE_ACCEL
  // accel
  OSCMessage accelMessage("/sleeper/accel");
  accelMessage.add(ID);
  accelMessage.add(accelX);
  accelMessage.add(accelY);
  accelMessage.add(accelZ);
  Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
  accelMessage.send(Udp);
  Udp.endPacket();
  accelMessage.empty();
#endif

#ifdef USE_TEMPERATURE
  // temperature
  OSCMessage temperatureMessage("/sleeper/temperature");
  temperatureMessage.add(ID);
  temperatureMessage.add(temperature);
  Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
  temperatureMessage.send(Udp);
  Udp.endPacket();
  temperatureMessage.empty();
#endif

#ifdef USE_PIEZO_ONSET
  // piezoOnset
  if (piezoOnsetDetected)
  {
    piezoOnsetDetected = false;
    OSCMessage piezoOnsetMessage("/sleeper/piezo_onset");
    piezoOnsetMessage.add(ID);
    Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
    piezoOnsetMessage.send(Udp);
    Udp.endPacket();
    piezoOnsetMessage.empty();
  }
#endif

#ifdef USE_ACCEL_ONSET
  // accelOnset
  if (accelOnsetDetected)
  {
    accelOnsetDetected = false;
    OSCMessage accelOnsetMessage("/sleeper/accel_onset");
    accelOnsetMessage.add(ID);
    Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
    accelOnsetMessage.send(Udp);
    Udp.endPacket();
    accelOnsetMessage.empty();
  }
#endif
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
  delay(5000);

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

  // touchAttachInterrupt(TOUCH_PIN, handleTouch, touchThreshold);
  touchValue.clear();
}

void loop()
{
  frameCounter++;
  readValues();
  processValues();
  sendOSC();
}