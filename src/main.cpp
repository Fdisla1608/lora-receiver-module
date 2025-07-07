#include "main.h"

Custom_LoRa *custom_LoRa;
uint32_t lastTime = 0;

void setup()
{
    Serial.begin(115200);
    custom_LoRa = new Custom_LoRa(ss, rst, dio0);

    custom_LoRa->onReceive(receivePayload);

    if (!custom_LoRa->begin(433E6))
    {
        Serial.println("LoRa Initialization Failed!");
        while (1)
            ;
    }

    Serial.println("LoRa Initializing OK!");
}

void loop()
{
    custom_LoRa->loop();
    if (millis() - lastTime > 5000)
    {
        lastTime = millis();
        String payload = sendPayload();
        custom_LoRa->sendPayload(payload.c_str());
        Serial.printf("Sending packet: %s\n", payload.c_str());
    }
}

void receivePayload(const char *payload, int rssi)
{
    Serial.printf("Received Package with RSSI %d: %s\n", rssi, payload);
}

String sendPayload()
{
    JsonDocument doc;
    doc["id"] = ESPUtils::getDeviceId();
    doc["type"] = "test";
    doc["data"] = "Hello World!";
    doc["date"] = millis();
    return doc.as<String>();
}
