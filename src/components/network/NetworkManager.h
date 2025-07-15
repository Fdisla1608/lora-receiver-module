#pragma once

#ifndef INTERFACES_H
#define INTERFACES_H

#include <ETH.h>
#include <WiFi.h>
#include <stdio.h>
#include <states.h>

#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "esp_task_wdt.h"
#include "./MQTT.h"

enum class networkType
{
    WIFI,
    ETHERNET
};

struct WifiNetwork
{
    char ssid[32]; // o 64, tamaño máximo que soportes
    int32_t rssi;
    uint8_t channel;
    uint8_t encryptionType;
};

class NetworkManager
{
private:
    MQTT *mqtt;
    uint64_t currentTime{0};
    const char *serverUrl = "broker.emqx.io";
    const uint16_t serverPort = 3000;
    const uint16_t mqttPort = 1883;

    // Helper methods
    std::function<void(char *topic, byte *payload, unsigned int length)> MQTTCallbackFunc;

    const char *getRemoteServerUrl(const char *endpoint);
    uint16_t ProcessHttpResponse(uint16_t httpCode, JsonDocument *jsonResponse);
    static void NETEvent(WiFiEvent_t event);

public:
    NetworkManager(std::function<void(char *topic, byte *payload, unsigned int length)> MQTTCallbackFunc);
    ~NetworkManager();

    void initializeAP(const char *ssid, const char *password, IPAddress localIp = IPAddress(192, 168, 4, 1), IPAddress gateway = IPAddress(192, 168, 4, 1), IPAddress subnet = IPAddress(255, 255, 255, 0));
    void scanNetworks();
    uint8_t scanNetworksComplete(JsonArray *networksArray, uint8_t maxNetworks);
    const char *getLocalIP();

    bool connect(networkType type, const char *ssid = "", const char *password = "", IPAddress Ip = IPAddress(0, 0, 0, 0), IPAddress Gateway = IPAddress(0, 0, 0, 0), IPAddress Subnet = IPAddress(0, 0, 0, 0));
    void initializeMQTT(const char *clientId);

    void MQTTPublish(const char *topic, const char *message);
    uint16_t httpGetRequest(const char *url, const char *queryParams, JsonDocument *jsonResponse);
    uint16_t httpPostRequest(const char *url, const char *jsonBody, JsonDocument *jsonResponse);
};

NetworkManager::NetworkManager(std::function<void(char *topic, byte *payload, unsigned int length)> MQTTCallbackFunc) : MQTTCallbackFunc(MQTTCallbackFunc)
{
}

NetworkManager::~NetworkManager()
{
}

/*
    * Maneja eventos de red de WiFi
   @param event Evento de red
   @param timeout Tiempo máximo de espera para la conexión
   @param ssid SSID de la red WiFi
   @param password Contraseña de la red WiFi
   @param Ip Dirección IP estática
   @param Gateway Puerta de enlace
   @param Subnet Máscara de subred
*/
bool NetworkManager::connect(networkType type, const char *ssid, const char *password, IPAddress Ip, IPAddress Gateway,
                             IPAddress Subnet)
{
    if (type == networkType::WIFI)
    {
        if (Ip != IPAddress(0, 0, 0, 0) && Gateway != IPAddress(0, 0, 0, 0) && Subnet != IPAddress(0, 0, 0, 0))
            WiFi.config(Ip, Gateway, Subnet);

        WiFi.mode(WIFI_AP_STA);
        WiFi.onEvent(NETEvent);
        return WiFi.begin(ssid, password) != WL_CONNECT_FAILED ? true : false;
    }
    else if (type == networkType::ETHERNET)
    {
        ETH.config(Ip, Gateway, Subnet);
        // ETH.onEvent(NETEvent);
        ETH.begin();
        return true;
    }

    return false;
}

void NetworkManager::initializeMQTT(const char *clientId)
{
    if (mqtt)
    {
        delete mqtt; // Asegúrate de liberar el recurso si ya existe
    }
    Serial.printf("Initializing MQTT with server: %s, port: %u, clientId: %s\n", serverUrl, mqttPort, clientId);
    mqtt = new MQTT(serverUrl, mqttPort, clientId, MQTTCallbackFunc);
    mqtt->InitNetworkLoopTask();
}

void NetworkManager::initializeAP(const char *ssid, const char *password, IPAddress localIp, IPAddress gateway, IPAddress subnet)
{
    WiFi.disconnect();
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(localIp, gateway, subnet);
    Serial.printf("Config Mode SSID: %s IP: %s \n", ssid, WiFi.softAPIP().toString().c_str());
}

void NetworkManager::scanNetworks()
{
    if (WiFi.scanComplete() != -2)
        WiFi.scanDelete();
    WiFi.scanNetworks(true); // Inicia el escaneo de redes WiFi
    changeState(SystemState::WIFI_SCANNING);
}

uint8_t NetworkManager::scanNetworksComplete(JsonArray *networksArray, uint8_t maxNetworks)
{
    int8_t count = WiFi.scanComplete();

    if (count < 0)
        return 0;

    if (count == 0)
    {
        changeState(SystemState::ERROR_NETWORK_NOT_FOUND);
        return 0;
    }

    count = (count > maxNetworks) ? maxNetworks : count; // Limita el número de redes a maxNetworks
    networksArray->clear(); // Limpia el array antes de agregar nuevas redes
    
    for (uint8_t i = 0; i < count; i++)
    {
        JsonDocument networkDoc;
        networkDoc["ssid"] = WiFi.SSID(i).c_str();
        networkDoc["rssi"] = WiFi.RSSI(i);
        networkDoc["channel"] = WiFi.channel(i);
        networkDoc["encryptionType"] = WiFi.encryptionType(i);
        networksArray->add(networkDoc);
    }

    WiFi.scanDelete(); // Limpia el escaneo para evitar duplicados en futuros escaneos
    
    return count;
}

uint16_t NetworkManager::httpGetRequest(const char *url, const char *queryParams, JsonDocument *jsonResponse)
{
    HTTPClient http;
    String fullUrl = String(url) + "?" + String(queryParams);
    http.begin(fullUrl);
    uint16_t httpCode = ProcessHttpResponse(http.GET(), jsonResponse);
    if (httpCode == 200)
        deserializeJson(*jsonResponse, http.getString());
    http.end();
    return httpCode;
}

uint16_t NetworkManager::httpPostRequest(const char *url, const char *jsonBody, JsonDocument *jsonResponse)
{
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    changeState(SystemState::NETWORK_HTTP_POST_REQUEST);
    uint16_t httpCode = ProcessHttpResponse(http.POST(jsonBody), jsonResponse);
    if (httpCode == 200)
        deserializeJson(*jsonResponse, http.getString());
    http.end();
    return httpCode;
}

const char *NetworkManager::getRemoteServerUrl(const char *endpoint)
{
    static char remoteServer[100]; // Asegúrate que sea suficientemente grande
    snprintf(remoteServer, sizeof(remoteServer), "http://%s:%u/%s", serverUrl, serverPort, endpoint);
    return remoteServer;
}

const char *NetworkManager::getLocalIP()
{
    static char localIP[16];
    snprintf(localIP, sizeof(localIP), "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    return localIP;
}

uint16_t NetworkManager::ProcessHttpResponse(uint16_t httpCode, JsonDocument *jsonResponse)
{
    switch (httpCode)
    {
    case 200:
        changeState(SystemState::NETWORK_HTTP_REQUEST_SUCCESS);
        break;
    case 400:
        changeState(SystemState::ERROR_SERVER_BAD_REQUEST);
        break;
    case 404:
        changeState(SystemState::ERROR_SERVER_UNAUTHORIZED);
        break;
    case 500:
        changeState(SystemState::ERROR_SERVER_INTERNAL);
        break;
    default:
        changeState(SystemState::ERROR_UNEXPECTED);
        break;
    }
    return httpCode;
}

void NetworkManager::MQTTPublish(const char *topic, const char *message)
{
    if (mqtt && mqtt->IsConnected())
    {
        changeState(SystemState::NETWORK_MQTT_PUBLISHING);
        if (mqtt->Publish(topic, message))
        {
            changeState(SystemState::NETWORK_MQTT_PUBLISH_SUCCESS);
        }
        else
        {
            changeState(SystemState::ERROR_MQTT_PUBLISH_FAILED);
        }
    }
    else
    {
        changeState(SystemState::ERROR_MQTT_CONNECTION_FAILED);
    }
}

void NetworkManager::NETEvent(WiFiEvent_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_START:
        changeState(SystemState::WIFI_STARTING);
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        changeState(SystemState::ERROR_NETWORK_DISCONNECTED);
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        changeState(SystemState::WIFI_CONNECTED);
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        changeState(SystemState::WIFI_GOT_IP);
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        changeState(SystemState::ERROR_NETWORK_DISCONNECTED);
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        changeState(SystemState::ERROR_NETWORK_DISCONNECTED);
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        changeState(SystemState::ETHERNET_GOT_IP);
        break;
    default:
        changeState(SystemState::ERROR_UNEXPECTED);
        break;
    }
}

#endif