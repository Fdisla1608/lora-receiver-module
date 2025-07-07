#include <Arduino.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "components/Utils/ESPUtils.h"
#include "components/LoRa/LoRa.h"

#define ss 5
#define rst 14
#define dio0 2

void receivePayload(const char *payload, int rssi);
String sendPayload();
