#include <Arduino.h>
#include <LoRa.h>
#include <cstdint>
#include <functional>
#include <unordered_set>
#include <utility> // std::pair

struct MeshPacket
{
    uint64_t id;
    uint64_t nodeId;
    uint64_t destinationId;
    uint64_t routeTable[10];
    uint8_t hopCount;
    uint8_t temperature;
    uint8_t humidity;
    uint8_t soilMoisture;
    double latitude;
    double longitude;
};

struct PairHash
{
    std::size_t operator()(const std::pair<uint64_t, uint64_t> &p) const
    {
        return std::hash<uint64_t>()(p.first) ^ (std::hash<uint64_t>()(p.second) << 1);
    }
};

class Custom_LoRa
{
  private:
    uint8_t _ss;
    uint8_t _rst;
    uint8_t _dio0;

    String LoRaData;

    std::function<void(MeshPacket packet)> callback;
    std::unordered_set<std::pair<uint64_t, uint64_t>, PairHash> receivedPackets;

    void emit(MeshPacket packet)
    {
        callback(packet);
    }

  public:
    Custom_LoRa(uint8_t ss, uint8_t rst, uint8_t dio0);
    ~Custom_LoRa();

    bool begin(uint32_t frequency);
    void onReceive(std::function<void(MeshPacket packet)> callback);
    void loop();

    uint8_t transmitPacket(uint8_t *data, uint8_t size);
    uint8_t retransmitPacket(MeshPacket packet);
    bool hasSeen(const MeshPacket &packet);
};
