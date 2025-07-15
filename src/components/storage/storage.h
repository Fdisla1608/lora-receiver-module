#pragma once
#ifndef STORAGE_H
#define STORAGE_H

#include "stdio.h"
#include <Preferences.h>
#include "../utils/ModuleInfo.h"

class Storage
{
private:
  bool eEPROMState = false;
  Preferences eEPROM;

public:
  Storage(/* args */);
  ~Storage();

  uint8_t init();
  uint8_t writeInformation(ModuleInfo &info);
  uint8_t getInformation(ModuleInfo &info);
};

#endif // STORAGE_H