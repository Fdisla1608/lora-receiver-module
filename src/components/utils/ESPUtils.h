#pragma once
#ifndef ESPUTILS_H
#define ESPUTILS_H

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

class ESPUtils
{
public:
    ESPUtils();
    ~ESPUtils();

    static const char *getDeviceId();

    const char *getModuleId();
    String getDateTime();
};

#endif
