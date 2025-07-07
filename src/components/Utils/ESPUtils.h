#pragma once

#ifndef ESPUtils_H
#define ESPUtils_H

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

class ESPUtils
{
private:
    /* data */
public:
    ESPUtils(/* args */);
    ~ESPUtils();

    static const char *getDeviceId();

    const char *getModuleId();
    String getDateTime();
};

ESPUtils::ESPUtils(/* args */)
{
}

ESPUtils::~ESPUtils()
{
}

const char *ESPUtils::getDeviceId()
{
    static char deviceID[21]; 
    uint64_t mac = ESP.getEfuseMac();
    snprintf(deviceID, sizeof(deviceID), "%llu", mac);
    return deviceID;
}

const char *ESPUtils::getModuleId()
{
    return "N/A"; // Placeholder for module ID, replace with actual logic if needed
}

String ESPUtils::getDateTime()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        http.begin("https://api.bitget.com/api/v2/public/time");
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0)
        {
            String payload = http.getString();
            if (httpResponseCode == 200)
            {
                // Parse the JSON response
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, payload);
                if (error)
                {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(error.f_str());
                    return String();
                }

                // Extract the date and time
                String dateTime = doc["data"]["serverTime"].as<String>();
                return dateTime;
            }
            else
            {
                Serial.println("GET failed!");
            }
        }
        else
        {
            Serial.print("Error on sending GET: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    }
    else
    {
        Serial.println("WiFi not connected");
    }

    return String();
}

#endif
