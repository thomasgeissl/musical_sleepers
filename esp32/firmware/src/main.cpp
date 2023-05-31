#include <Arduino.h>
#include <WiFi.h>
#include <OSCMessage.h>
#include "GY521.h"
#include "RunningAverage.h"
#include <Ewma.h>

#include "./config.h"

xTaskHandle networkTaskHandle;
bool _locked;

WiFiUDP Udp;
GY521 _sensor(0x68);

unsigned long _frameCounter = 0;
// piezo
int _piezoValue;
bool _piezoOnsetDetected = false;
unsigned long _piezoOnsetTimestamp;
int _piezo2Value;
bool _piezo2OnsetDetected = false;
unsigned long _piezo2OnsetTimestamp;
// touch
RunningAverage _touchValue(4);
RunningAverage _touch2Value(4);
bool _touched = false;
bool _touched2 = false;

// angle
float _angleX;
float _angleY;
float _angleZ;

// accel
float _accelX;
float _accelY;
float _accelZ;
Ewma _accelXFilter(0.1); // Less smoothing - faster to detect changes, but more prone to noise
Ewma _accelYFilter(0.1); // More smoothing - less prone to noise, but slower to detect changes
Ewma _accelZFilter(0.1); // More smoothing - less prone to noise, but slower to detect changes

bool _accelOnsetDetected = false;
unsigned long accelOnsetTimestamp;
// temperature
float _temperature;

void readValues()
{
  // piezo
  _piezoValue = analogRead(PIEZO_PIN);
  _piezo2Value = analogRead(PIEZO2_PIN);
  // touch
  _touchValue.addValue(touchRead(TOUCH_PIN));
  _touch2Value.addValue(touchRead(TOUCH2_PIN));

#if defined(USE_ACCEL) || defined(USE_ANGLE) || defined(USE_ACCEL_ONSET) || defined(USE_TEMPERATURE)
  _sensor.read();
  // angle
  _angleX = _sensor.getAngleX();
  _angleY = _sensor.getAngleY();
  _angleZ = _sensor.getAngleZ();
  // accel
  _accelX = _sensor.getAccelX();
  _accelY = _sensor.getAccelY();
  _accelZ = _sensor.getAccelZ();
#endif
  // temperature
  _temperature = _sensor.getTemperature();
}

void routeConfig(OSCMessage &msg, int addrOffset)
{
    if (msg.isInt(0)) {
    _piezoOnsetThreshold = msg.getInt(0);
  }
  if (msg.isInt(1)) {
    _piezoOnsetDebounceTime = msg.getInt(1);
  }
  if (msg.isInt(2)) {
    _touchThreshold = msg.getInt(2);
  }
  if (msg.isInt(3)) {
    _accelOnsetThreshold = msg.getInt(3);
  }
  if (msg.isInt(4)) {
    _accelOnsetDebounceTime = msg.getInt(4);
  }
}
void readOSC()
{
  OSCBundle bundleIN;
  int size;

  if ((size = Udp.parsePacket()) > 0)
  {
    while (size--)
    {
      bundleIN.fill(Udp.read());
    }

    if (!bundleIN.hasError())
    {
      bundleIN.route("/config", routeConfig);
    }
  }
}

void processValues()
{
  auto timestamp = millis();
#ifdef USE_ACCEL_ONSET
  int16_t accelMagnitude = sqrt(sq(accelX) + sq(accelY) + sq(accelZ));
  if (_accelMagnitude > _accelOnsetThreshold && _accelOnsetThreshold + timestamp > _accelOnsetDebounceTime)
  {
    _accelOnsetDetected = true;
    _accelOnsetTimestamp = timestamp;
  }
#endif
#ifdef USE_PIEZO_ONSET
  if (_piezoValue > _piezoOnsetThreshold && timestamp > _piezoOnsetTimestamp + _piezoOnsetDebounceTime)
  {
    _piezoOnsetDetected = true;
    _piezoOnsetTimestamp = timestamp;
  }
  if (_piezo2Value > _piezoOnsetThreshold && timestamp > _piezo2OnsetTimestamp + _piezoOnsetDebounceTime)
  {
    _piezo2OnsetDetected = true;
    _piezo2OnsetTimestamp = timestamp;
  }
#endif
#ifdef USE_TOUCH
  _touched = _touchValue.getAverage() < _touchThreshold;
  _touched2 = _touch2Value.getAverage() < _touchThreshold;
#endif
}

void send()
// void networkTask(void *parameter)
{
  // if (_locked)
  // {
  //   return;
  // }

  // _locked = true;
  auto piezoValue = _piezoValue;
  auto piezo2Value = _piezo2Value;
  auto accelX = _accelX;
  auto accelY = _accelY;
  auto accelZ = _accelZ;
  auto angleX = _accelX;
  auto angleY = _angleY;
  auto angleZ = _angleZ;
#if defined(COMBINED)
  OSCMessage message("/sleeper/all");
  message.add(ID);
#ifdef USE_PIEZO
  message.add(_piezoValue);
  message.add(_piezo2Value);
#endif
#ifdef USE_TOUCH
  message.add(_touched ? 1 : 0);
  message.add(_touched2 ? 1 : 0);
#endif
#ifdef USE_ANGLE
  message.add(angleX);
  message.add(angleY);
  message.add(angleZ);
#endif
#ifdef USE_ACCEL
  message.add(accelX);
  message.add(accelY);
  message.add(accelZ);
#endif
  Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
  message.send(Udp);
  Udp.endPacket();
  message.empty();
#else
  // _locked = false;
#ifdef USE_PIEZO
  // piezo
  OSCMessage piezoMessage("/sleeper/piezo");
  piezoMessage.add(ID);
  piezoMessage.add(_piezoValue);
  Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
  piezoMessage.send(Udp);
  Udp.endPacket();
  piezoMessage.empty();
#endif

#ifdef USE_TOUCH
  // touch
  if (_frameCounter % 20 == 0)
  {
    OSCMessage touchMessage("/sleeper/touch");
    touchMessage.add(ID);
    touchMessage.add(_touched ? 1 : 0);
    touchMessage.add(_touched2 ? 1 : 0);
    Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
    touchMessage.send(Udp);
    Udp.endPacket();
    touchMessage.empty();
  }
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
  float x = (_accelXFilter.filter(accelX));
  float y = (_accelYFilter.filter(accelY));
  float z = (_accelZFilter.filter(accelZ));
  accelMessage.add(x);
  accelMessage.add(y);
  accelMessage.add(z);
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

#ifdef USE_ACCEL_ONSET
  // accelOnset
  if (_accelOnsetDetected)
  {
    _accelOnsetDetected = false;
    OSCMessage accelOnsetMessage("/sleeper/accel_onset");
    accelOnsetMessage.add(ID);
    Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
    accelOnsetMessage.send(Udp);
    Udp.endPacket();
    accelOnsetMessage.empty();
  }
#endif
#endif // COMBINED

// onset is sent not combined
#ifdef USE_PIEZO_ONSET
  // piezoOnset
  if (_piezoOnsetDetected)
  {
    _piezoOnsetDetected = false;
    OSCMessage piezoOnsetMessage("/sleeper/piezo_onset");
    piezoOnsetMessage.add(ID);
    piezoOnsetMessage.add(piezoValue);
    Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
    piezoOnsetMessage.send(Udp);
    Udp.endPacket();
    piezoOnsetMessage.empty();
  }
  if (_piezo2OnsetDetected)
  {
    _piezo2OnsetDetected = false;
    OSCMessage piezoOnsetMessage("/sleeper/piezo2_onset");
    piezoOnsetMessage.add(ID);
    piezoOnsetMessage.add(piezo2Value);
    Udp.beginPacket(DESTINATION_IP, DESTINATION_PORT);
    piezoOnsetMessage.send(Udp);
    Udp.endPacket();
    piezoOnsetMessage.empty();
  }
#endif
}

void setup()
{
  Serial.begin(115200);
  Serial.println("setting up wifi with the following credentials:");
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

  Udp.begin(PORT);

#if defined(USE_ACCEL) || defined(USE_ANGLE) || defined(USE_ACCEL_ONSET) || defined(USE_TEMPERATURE)
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("GY521_LIB_VERSION: ");
  Serial.println(GY521_LIB_VERSION);

  Wire.begin();

  delay(100);
  while (_sensor.wakeup() == false)
  {
    Serial.print(millis());
    Serial.println("\tCould not connect to GY521");
    delay(1000);
  }
  _sensor.setAccelSensitivity(2); //  8g
  _sensor.setGyroSensitivity(1);  //  500 degrees/s

  _sensor.setThrottle();
  Serial.println("start...");

  //  set calibration values from calibration sketch.
  _sensor.axe = 0.574;
  _sensor.aye = -0.002;
  _sensor.aze = -1.043;
  _sensor.gxe = 10.702;
  _sensor.gye = -6.436;
  _sensor.gze = -0.676;
#endif

  // touchAttachInterrupt(TOUCH_PIN, handleTouch, touchThreshold);
  _touchValue.clear();

  // xTaskCreatePinnedToCore(networkTask, "networkTask", 10000, NULL, 1, &networkTaskHandle, 0);
}

void loop()
{
  _frameCounter++;
  readValues();
  readOSC();
  processValues();
  send();
}