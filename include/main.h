#include <Arduino.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "components/utils/ESPUtils.h"
#include "components/LoRa/LoRa.h"
#include "components/network/NetworkManager.h"
#include "components/network/WebConfig.h"
#include "components/drivers/DisplayLCD.h"
#include "components/utils/ModuleInfo.h"
#include "components/storage/storage.h"

#define VERSION "V0.0.1-Alpha"

#define ss 5
#define rst 14
#define dio0 2

void receivePayload(const char *payload, int rssi);
void MQTTCallback(char *topic, byte *payload, unsigned int length);
String sendPayload();
