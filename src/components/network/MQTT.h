#pragma once

#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "../utils/ESPUtils.h"
#include <states.h>

class MQTT
{
private:
    WiFiClient netClient;   // Encapsulado, no global
    PubSubClient Pubclient; // Cliente MQTT usando netClient

    const char *MQTTServer;
    int MQTTPort{0};
    const char *MQTTClientId;
    const char *SubscribeTopic = "soilion/gateways";

    int timerReconnect{0};
    uint8_t attempts{0};

    void Connect();
    void Subscribe();
    static void Loop(void *pvParameters);
    std::function<void(char *topic, byte *payload, unsigned int length)> MQTTCallbackFunc;

public:
    TaskHandle_t MQTTTaskHandle = NULL;

    MQTT(const char *server, int port, const char *clientId, std::function<void(char *topic, byte *payload, unsigned int length)> MQTTCallbackFunc);
    ~MQTT();

    void InitNetworkLoopTask();
    boolean Publish(const char *topic, const char *message);
    boolean IsConnected();
};

MQTT::MQTT(const char *server, int port, const char *clientId, std::function<void(char *topic, byte *payload, unsigned int length)> MQTTCallbackFunc)
    : Pubclient(netClient), MQTTServer(server), MQTTPort(port), MQTTClientId(clientId), MQTTCallbackFunc(MQTTCallbackFunc)
{
    Pubclient.setServer(MQTTServer, MQTTPort);
    Pubclient.setCallback(MQTTCallbackFunc);
}

MQTT::~MQTT()
{
    // Limpieza si fuese necesario
}

void MQTT::Connect()
{
    changeState(SystemState::NETWORK_MQTT_CONNECTING);

    if (Pubclient.connect(MQTTClientId)) // Usa el ID real del cliente
    {
        changeState(SystemState::NETWORK_MQTT_CONNECTED);
        Subscribe();
    }
    else if (millis() - timerReconnect > 5000)
    {
        timerReconnect = millis();
        changeState(SystemState::ERROR_MQTT_CONNECTION_FAILED);
    }
}

void MQTT::Subscribe()
{
    if (Pubclient.subscribe(SubscribeTopic))
    {
        changeState(SystemState::NETWORK_MQTT_SUBSCRIBE_SUCCESS);
    }
    else
    {
        changeState(SystemState::ERROR_MQTT_SUBSCRIPTION_FAILED);
    }
}

boolean MQTT::IsConnected()
{
    return Pubclient.connected();
}

void MQTT::InitNetworkLoopTask()
{
    xTaskCreatePinnedToCore(
        Loop,
        "Mqtt_Loop_Task",
        8192, // Tamaño de stack aumentado
        this,
        1, // Prioridad
        &MQTTTaskHandle,
        1 // Core 1
    );
}

void MQTT::Loop(void *pvParameters)
{
    MQTT *instance = static_cast<MQTT *>(pvParameters);
    if (!instance)
    {
        vTaskDelete(NULL);
        return;
    }

    while (true)
    {
        if (!instance->IsConnected())
        {
            changeState(SystemState::ERROR_MQTT_DISCONNECTED);
            instance->Connect();
        }

        instance->Pubclient.loop();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

boolean MQTT::Publish(const char *topic, const char *message)
{
    return Pubclient.publish(topic, message);
}

#endif
