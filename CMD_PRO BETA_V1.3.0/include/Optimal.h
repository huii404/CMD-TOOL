#ifndef OPTIMAL_H
#define OPTIMAL_H

#include "SystemCore.h"
#include "Internet.h"
#include "Maintenance.h"
#include "Extension.h"
#include <string>

class Optimal {
private:
    SystemCore &sc;
    Internet &n;
    Maintenance &m;
    Extension &e;
public:
    Optimal(SystemCore &s, Internet &net, Maintenance &mnt, Extension &exten);
    void optimizeSystemPRO();
    void enableSecurityPRO();
    void optimizeNetworkPRO();
    std::string getCurrentOS();
    void upgradeWindowsEditionPRO();
};

#endif