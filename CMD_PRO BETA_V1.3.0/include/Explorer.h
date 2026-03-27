#ifndef EXPLORER_H
#define EXPLORER_H

#include "SystemCore.h"
#include <string>

class Explorer {
private:
    SystemCore &sc;
    char strongByteProcess(char byte, std::string key, int position, bool isEncrypt);
public:
    Explorer(SystemCore &s);
    void processFileBinary(bool isEncrypt);
    void clearBrowserCache();
    void nguytrangFolder();
};

#endif