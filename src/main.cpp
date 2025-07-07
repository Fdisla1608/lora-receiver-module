#include "main.h"

Storage storage;

NetworkManager networkManager(MQTTCallback);
WebConfigServer webConfigServer(&networkManager, &storage);

ModuleInfo moduleInfo;
DisplayLCD lcdDisplay;
Custom_LoRa *customLoRa;
uint32_t lastTime = 0;

void setup()
{
    Serial.begin(115200);
    lcdDisplay.Initialize();

    customLoRa = new Custom_LoRa(ss, rst, dio0);
    customLoRa->onReceive(receivePayload);

    if (!storage.init())
    {
        Serial.println("❌ Error al leer la configuracion de la unidad");
        while (1)
            ;
    }

    storage.getInformation(moduleInfo);

    if (moduleInfo.boardType.isEmpty() || moduleInfo.wifiSSID.isEmpty())
    {
        changeState(SystemState::ERROR_MODULE_NOT_CONFIGURED, &lcdDisplay);
        Serial.println("Initializing module configuration...");
        webConfigServer.begin(moduleInfo);
        Serial.println("Module configuration initialized.");
        while (1)
            ;
    }
    else
    {
        networkManager.connect(networkType::WIFI, moduleInfo.wifiSSID.c_str(), moduleInfo.wifiPassword.c_str());
        uint64_t currentTime = millis();
        while (getCurrentState() != SystemState::WIFI_GOT_IP && getCurrentState() != SystemState::ETHERNET_GOT_IP)
        {
            if (millis() - currentTime > 10000)
            {
                changeState(SystemState::ERROR_NETWORK_TIMEOUT, &lcdDisplay);
                return;
            }
        }

        networkManager.initializeMQTT(moduleInfo.idModule.c_str());
    }

    Serial.println("************** VSoftware Pulse Firmware **************");
    Serial.printf("- Device ID: %s\n", moduleInfo.idModule.c_str());
    Serial.printf("- Board Type: %s\n", moduleInfo.boardType.c_str() ? moduleInfo.boardType.c_str() : "Not Configured");
    Serial.printf("- Firmware Version: %s\n", VERSION);
    Serial.printf("- WiFi SSID: %s\n", moduleInfo.wifiSSID.c_str() ? moduleInfo.wifiSSID.c_str() : "Not Configured");
    Serial.printf("- Ip Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("- LoRa Initializing: %s\n", customLoRa->begin(433E6) ? "Success" : "Failed");
    Serial.println("******************************************************");
    delay(1000);
}

void loop()
{
    customLoRa->loop();

    if (millis() - lastTime > 5000)
    {
        lastTime = millis();
        String payload = sendPayload();
        customLoRa->sendPayload(payload.c_str());
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
    doc["id"] = moduleInfo.idModule;
    doc["type"] = "test";
    doc["data"] = "Hello World!";
    doc["date"] = millis();
    return doc.as<String>();
}

void MQTTCallback(char *topic, byte *payload, unsigned int length)
{
    JsonDocument payloadReceived;
    DeserializationError error = deserializeJson(payloadReceived, payload, length);

    if (error)
        return;

    Serial.printf("Payload Receive: %s", payloadReceived.as<String>().c_str());
}