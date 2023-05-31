#include "./credentials.h"
#define DESTINATION_IP "255.255.255.255"
// #define DESTINATION_IP "192.168.0.18"
#define DESTINATION_PORT 8000
#define PORT 9000
#define PIEZO_PIN 34
#define PIEZO2_PIN 35
#define TOUCH_PIN 4
#define TOUCH2_PIN 2

const int _accelOnsetThreshold = 5000;
const int _accelOnsetDebounceTime = 50;
const int _piezoOnsetThreshold = 250;
const int _piezoOnsetDebounceTime = 50;
const int _touchThreshold = 50;