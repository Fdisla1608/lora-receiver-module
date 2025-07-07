
#ifndef MODULE_INFO_H
#define MODULE_INFO_H

#include <Arduino.h>

struct ModuleInfo
{
    String idModule;
    String boardType;
    String wifiSSID;
    String wifiPassword;
    IPAddress staticIp;
    IPAddress subnetMask;
    IPAddress gateway;
    String mqttServer;
    int mqttPort;
};

#endif