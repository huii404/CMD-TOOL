#ifndef MAINTENANCE_H
#define MAINTENANCE_H

#include "SystemCore.h"

class Maintenance {
private:
    SystemCore &sc;
public:
    Maintenance(SystemCore &s);
    void cleanDisk();
    void QuickScanVirus();
    void FullScanVirus();
    void restart();
    void Consumer_Content();
    void clearEventLogs();
    void Hibernate();
    void windowsTelemetry();
    void reduceShutdownTime();
    void disableAllStartupApps();
};

#endif