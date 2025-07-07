#include "storage.h"
#include "../utils/ESPUtils.h"


Storage::Storage()
{
}

Storage::~Storage()
{
    if (eEPROMState)
    {
        eEPROM.end();
    }
}

uint8_t Storage::init()
{
    if (eEPROM.begin("terminal"))
    {
        eEPROMState = true;
        return 0x01; // Indica que la EEPROM se ha inicializado correctamente
    }
    else
    {
        eEPROMState = false;
        return 0x00; // Indica que la EEPROM no se ha inicializado correctamente
    }
}

uint8_t Storage::writeInformation(ModuleInfo &info)
{
    if (!eEPROMState)
        return false;
    eEPROM.putString("boardType", info.boardType);
    eEPROM.putString("wifiSSID", info.wifiSSID);
    eEPROM.putString("wifiPassword", info.wifiPassword);
    eEPROM.putString("staticIp", info.staticIp.toString());
    eEPROM.putString("subnetMask", info.subnetMask.toString());
    eEPROM.putString("gateway", info.gateway.toString());
    return true;
}

uint8_t Storage::getInformation(ModuleInfo &info)
{

    if (eEPROMState)
    {
        info.idModule = ESPUtils::getDeviceId();
        info.boardType = eEPROM.getString("boardType", "");
        info.wifiSSID = eEPROM.getString("wifiSSID", "");
        info.wifiPassword = eEPROM.getString("wifiPassword", "");

        String ipStr = eEPROM.getString("staticIp", "");
        IPAddress ip;
        if (ip.fromString(ipStr))
            info.staticIp = ip;

        ipStr = eEPROM.getString("subnetMask", "");
        if (ip.fromString(ipStr))
            info.subnetMask = ip;

        ipStr = eEPROM.getString("gateway", "");
        if (ip.fromString(ipStr))
            info.gateway = ip;

        return true;
    }
    return false;
}
