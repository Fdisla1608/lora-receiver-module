#include "ESPUtils.h"

ESPUtils::ESPUtils() {}

ESPUtils::~ESPUtils() {}

const char *ESPUtils::getDeviceId()
{
    static char deviceID[21]; 
    uint64_t mac = ESP.getEfuseMac();
    snprintf(deviceID, sizeof(deviceID), "%llu", mac);
    return deviceID;
}

const char *ESPUtils::getModuleId()
{
    return "N/A"; // Placeholder for module ID
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
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, payload);
                if (error)
                {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(error.f_str());
                    return String();
                }

                return doc["data"]["serverTime"].as<String>();
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
