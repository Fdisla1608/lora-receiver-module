#include <Arduino.h>
#include <LoRa.h>
#include <functional>

class Custom_LoRa
{
  private:
    uint8_t _ss;
    uint8_t _rst;
    uint8_t _dio0;

    String LoRaData;

    std::function<void(const char *, int rrsi)> callback;
    void emit(const char *data, int rssi)
    {
        callback(data, rssi);
    }

  public:
    Custom_LoRa(uint8_t ss, uint8_t rst, uint8_t dio0);
    ~Custom_LoRa();

    bool begin(uint32_t frequency);
    uint8_t sendPackage(uint8_t *data, uint8_t size);
    void sendPayload(const char *payload);
    void onReceive(std::function<void(const char *, int)> callback);
    void loop();
};

Custom_LoRa::Custom_LoRa(uint8_t ss, uint8_t rst, uint8_t dio0) : _ss(ss), _rst(rst), _dio0(dio0)
{
}

Custom_LoRa::~Custom_LoRa()
{
}

bool Custom_LoRa::begin(uint32_t frequency)
{
    LoRa.setPins(_ss, _rst, _dio0); // setup LoRa transceiver module

    uint32_t start = millis();
    while (!LoRa.begin(frequency)) // 433E6 - Asia, 866E6 - Europe, 915E6 - North America
    {
        if (millis() - start > 10000)
        {
            return false;
        }
        Serial.println(".");
        delay(500);
    }

    LoRa.setSyncWord(0xA5);
    Serial.println("LoRa Initializing OK!");

    return true;
}

uint8_t Custom_LoRa::sendPackage(uint8_t *data, uint8_t size)
{
    LoRa.beginPacket();
    LoRa.write(data, size);
    LoRa.endPacket();
    return 0;
}

void Custom_LoRa::sendPayload(const char *payload)
{
    LoRa.beginPacket();
    LoRa.print(payload);
    LoRa.endPacket();
}

void Custom_LoRa::onReceive(std::function<void(const char *, int)> callback)
{
    this->callback = callback;
}

void Custom_LoRa::loop()
{
    int packetSize = LoRa.parsePacket(); // try to parse packet
    if (packetSize)
    {
        while (LoRa.available()) // read packet
        {
            LoRaData += (char)LoRa.read();
        }
        emit(LoRaData.c_str(), LoRa.packetRssi());
        LoRaData = "";
    }
}
