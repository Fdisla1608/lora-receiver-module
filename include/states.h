
#pragma once
#ifndef STATES_H
#define STATES_H

#include "./components/drivers/DisplayLCD.h"


enum class SystemState
{
    NAH,
    INITIALIZING,
    CONFIGURATION_MODE_STARTED,
    CONFIGURATION_SUCCESS,
    IDLE,

    // Network states
    WIFI_STARTING,
    WIFI_MODE_STA,
    WIFI_MODE_AP,
    WIFI_MODE_AP_STA,
    WIFI_SCANNING,
    WIFI_SCANNING_COMPLETED,
    WIFI_CONNECTED,
    WIFI_GOT_IP,
    ETHERNET_STARTING,
    ETHERNET_CONNECTED,
    ETHERNET_GOT_IP,


    // Server states
    NETWORK_PING_REQUEST,
    NETWORK_HTTP_GET_REQUEST,
    NETWORK_HTTP_POST_REQUEST,
    NETWORK_HTTP_REQUEST_SUCCESS,

    // MQTT states
    NETWORK_MQTT_CONNECTING,
    NETWORK_MQTT_CONNECTED,
    NETWORK_MQTT_PUBLISHING,
    NETWORK_MQTT_SUBSCRIBING,
    NETWORK_MQTT_UNSUBSCRIBING,
    NETWORK_MQTT_RECEIVING_MESSAGE,
    NETWORK_MQTT_PROCESSING_MESSAGE,
    NETWORK_MQTT_PUBLISH_SUCCESS,
    NETWORK_MQTT_SUBSCRIBE_SUCCESS,
    NETWORK_MQTT_UNSUBSCRIBE_SUCCESS,

    // Error states
    ERROR_MODULE_NOT_CONFIGURED,
    ERROR_CONFIGURATION_FAILED,

    ERROR_DISPLAY_NOT_INITIALIZED,
    ERROR_DISPLAY_DISCONNECTED,

    ERROR_NETWORK_NOT_FOUND,
    ERROR_NETWORK_TIMEOUT,
    ERROR_NETWORK_DISCONNECTED,

    ERROR_SERVER_UNAVAILABLE,
    ERROR_SERVER_BAD_REQUEST,
    ERROR_SERVER_TIMEOUT,
    ERROR_SERVER_UNAUTHORIZED,
    ERROR_SERVER_INTERNAL,

    ERROR_MQTT_CONNECTION_FAILED,
    ERROR_MQTT_SUBSCRIPTION_FAILED,
    ERROR_MQTT_PUBLISH_FAILED,
    ERROR_MQTT_UNEXPECTED_RESPONSE,
    ERROR_MQTT_DISCONNECTED,

    ERROR_UNEXPECTED,
};

SystemState currentState = SystemState::NAH;

const char *SystemStateToString(SystemState state)
{
    switch (state)
    {
    case SystemState::NAH:
        return "NAH";
    case SystemState::INITIALIZING:
        return "INIT";
    case SystemState::CONFIGURATION_MODE_STARTED:
        return "CFG_START";
    case SystemState::CONFIGURATION_SUCCESS:
        return "CFG_OK";
    case SystemState::IDLE:
        return "IDLE";

    // Network
    case SystemState::WIFI_STARTING:
        return "WIFI_START";
    case SystemState::WIFI_MODE_STA:
        return "WIFI_STA";
    case SystemState::WIFI_MODE_AP:
        return "WIFI_AP";
    case SystemState::WIFI_MODE_AP_STA:
        return "WIFI_AP_STA";
    case SystemState::WIFI_SCANNING:
        return "WIFI_SCAN";
    case SystemState::WIFI_SCANNING_COMPLETED:
        return "SCAN_DONE";
    case SystemState::WIFI_CONNECTED:
        return "WIFI_CONN";
    case SystemState::WIFI_GOT_IP:
        return "WIFI_IP_OK";
    case SystemState::ETHERNET_STARTING:
        return "ETH_START";
    case SystemState::ETHERNET_CONNECTED:
        return "ETH_CONN";
    case SystemState::ETHERNET_GOT_IP:
        return "ETH_IP_OK";



    // Server
    case SystemState::NETWORK_PING_REQUEST:
        return "SRV_PING";
    case SystemState::NETWORK_HTTP_GET_REQUEST:
        return "SRV_GET";
    case SystemState::NETWORK_HTTP_POST_REQUEST:
        return "SRV_POST";
    case SystemState::NETWORK_HTTP_REQUEST_SUCCESS:
        return "SRV_OK";

    // MQTT
    case SystemState::NETWORK_MQTT_CONNECTING:
        return "MQTT_CONN";
    case SystemState::NETWORK_MQTT_CONNECTED:
        return "MQTT_OK";
    case SystemState::NETWORK_MQTT_PUBLISHING:
        return "MQTT_PUB";
    case SystemState::NETWORK_MQTT_SUBSCRIBING:
        return "MQTT_SUB";
    case SystemState::NETWORK_MQTT_UNSUBSCRIBING:
        return "MQTT_UNSUB";
    case SystemState::NETWORK_MQTT_RECEIVING_MESSAGE:
        return "MQTT_RX";
    case SystemState::NETWORK_MQTT_PROCESSING_MESSAGE:
        return "MQTT_PROC";
    case SystemState::NETWORK_MQTT_PUBLISH_SUCCESS:
        return "MQTT_PUB_OK";
    case SystemState::NETWORK_MQTT_SUBSCRIBE_SUCCESS:
        return "MQTT_SUB_OK";
    case SystemState::NETWORK_MQTT_UNSUBSCRIBE_SUCCESS:
        return "MQTT_UNSUB_OK";

    // Errors
    case SystemState::ERROR_MODULE_NOT_CONFIGURED:
        return "ERR_CFG";


    case SystemState::ERROR_DISPLAY_NOT_INITIALIZED:
        return "ERR_LCD_INIT";
    case SystemState::ERROR_DISPLAY_DISCONNECTED:
        return "ERR_LCD_DC";

    case SystemState::ERROR_NETWORK_NOT_FOUND:
        return "ERR_NET_404";
    case SystemState::ERROR_NETWORK_TIMEOUT:
        return "ERR_NET_TO";
    case SystemState::ERROR_NETWORK_DISCONNECTED:
        return "ERR_NET_DC";

    case SystemState::ERROR_SERVER_UNAVAILABLE:
        return "ERR_SRV_DOWN";
    case SystemState::ERROR_SERVER_BAD_REQUEST:
        return "ERR_SRV_400";
    case SystemState::ERROR_SERVER_TIMEOUT:
        return "ERR_SRV_TO";
    case SystemState::ERROR_SERVER_UNAUTHORIZED:
        return "ERR_SRV_401";
    case SystemState::ERROR_SERVER_INTERNAL:
        return "ERR_SRV_500";

    case SystemState::ERROR_MQTT_CONNECTION_FAILED:
        return "ERR_MQTT_CONN";
    case SystemState::ERROR_MQTT_SUBSCRIPTION_FAILED:
        return "ERR_MQTT_SUB";
    case SystemState::ERROR_MQTT_PUBLISH_FAILED:
        return "ERR_MQTT_PUB";
    case SystemState::ERROR_MQTT_UNEXPECTED_RESPONSE:
        return "ERR_MQTT_RESP";
    case SystemState::ERROR_MQTT_DISCONNECTED:
        return "ERR_MQTT_DC";

    case SystemState::ERROR_UNEXPECTED:
        return "ERR_UNKNOWN";

    default:
        return "UNKNOWN";
    }
}


void changeState(SystemState newState, DisplayLCD *lcdDisplay = nullptr, const char *message = "")
{
    if (currentState != newState)
    {
        Serial.printf("State Changed: %s -> %s\n", SystemStateToString(currentState), SystemStateToString(newState));
        currentState = newState;

        if (lcdDisplay)
            lcdDisplay->printScreen(String("State: " + String(SystemStateToString(newState))).c_str(), message);
    }
}

SystemState getCurrentState()
{
    return currentState;
}

#endif // STATES_H