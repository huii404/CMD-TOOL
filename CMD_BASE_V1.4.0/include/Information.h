#ifndef INFORMATION_H
#define INFORMATION_H

#include "SystemCore.h"
#include <string>

class Information {
private:
    SystemCore &sc;

public:
    Information(SystemCore &s);
    ~Information();
    std::string getCPUModel();
    std::string getGPUModel();
    std::string getDiskCStatus();
    void getRAMInfo(double &total, double &free);
    std::string getUptime();
    std::string getHostname();
    std::string getIPv4Address();
    std::string getWindowsVersion();
    int getCPUCores();
    double getCPUSpeed();
    std::string getGPUMemory();
    std::string getDeviceType(); // <-- Hàm này đã được chuyển xuống public

    // --- CÁC HÀM TƯƠNG TÁC GIAO DIỆN ---
    void showDashboard();
    void showSoftware(); 
    void driverList();   
    void systemInfo();   
};

#endif // INFORMATION_H