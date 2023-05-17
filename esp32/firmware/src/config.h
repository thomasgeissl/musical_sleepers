#include "./credentials.h"
#define DESTINATION_IP "255.255.255.255"
// #define DESTINATION_IP "192.168.0.18"
#define DESTINATION_PORT 8000
#define PIEZO_PIN 34
#define TOUCH_PIN 4

const int accelOnsetThreshold = 5000;
const int accelOnsetDebounceTime = 50;
const int piezoOnsetThreshold = 150;
const int piezoOnsetDebounceTime = 50;
const int touchThreshold = 50;