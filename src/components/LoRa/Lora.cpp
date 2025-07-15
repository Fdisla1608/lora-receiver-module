#include "Lora.h"

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

    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.setTxPower(20);

    LoRa.setSyncWord(0xA5);
    return true;
}

bool Custom_LoRa::hasSeen(const MeshPacket &packet)
{
    auto key = std::make_pair(packet.id, packet.nodeId);
    if (receivedPackets.find(key) != receivedPackets.end())
    {
        return true; // Ya fue recibido
    }
    receivedPackets.insert(key);
    return false;
}

void Custom_LoRa::onReceive(std::function<void(MeshPacket packet)> callback)
{
    this->callback = callback;
}

void Custom_LoRa::loop()
{
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
        MeshPacket packet;
        LoRa.readBytes((uint8_t *)&packet, sizeof(packet));
        if (!hasSeen(packet))
        {
            emit(packet);
        }
    }
}

uint8_t Custom_LoRa::transmitPacket(uint8_t *data, uint8_t size)
{
    LoRa.beginPacket();
    LoRa.write(data, size);
    LoRa.endPacket();
    return 0;
}

uint8_t Custom_LoRa::retransmitPacket(MeshPacket packet)
{
    delay(random(100, 500)); // Delay aleatorio para evitar colisiones
    return transmitPacket((uint8_t *)&packet, sizeof(packet));
}