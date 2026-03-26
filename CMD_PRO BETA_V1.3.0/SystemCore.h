#ifndef SYSTEMCORE_H
#define SYSTEMCORE_H

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <limits>
#include <vector>
#include <string>

using namespace std;

class SystemCore {
private:
    HANDLE hJob;

public:
    // Constructor & Destructor
    SystemCore();
    ~SystemCore();

    // Basic utilities
    void setColor(int color);
    void cls();
    string getTime();
    void waitEnter();
    int readInt(const string &prompt);

    // Command execution
    void runCMD(const string &cmd);
    bool runAdmin(const string &cmd, bool silent = false);

    // Logging
    template <typename... Args>
    void log(Args... args){
        ofstream f("History.txt", ios::app);
        if (!f.is_open()) return;

        f << getTime() << " : ";

        vector<int> path = {args...};
        size_t n = path.size();

        for (size_t i = 0; i < n; ++i){
            bool isLast = (i == n - 1);

            if (isLast){
                if (path[i] == 0)f << "[0.EXIT]";
                else             f << "[" << path[i] << ".]";
            }else{
                f << "[" << path[i] << "]";
            }

            if (!isLast)f << "->";
        }f << "\n";
        f.close();
    }

    // Mouse & Keyboard
    void leftClick();
    void setClipboard(const string &text);
    void pressCtrlV();
    void pressEnter();

    // File operations
    void ZIP(string source, string destination, int level);
    void UNZIP(string zipFile, string extractPath);

    // Utility function
    string trim(string s);

    // System Info - Hardware (11 hàm API tức thì)
    string getCPUModel();
    string getGPUModel();
    string getDiskCStatus();
    void getRAMInfo(double &total, double &free);
    string getUptime();
    string getHostname();
    string getIPv4Address();
    string getWindowsVersion();
    int getCPUCores();
    double getCPUSpeed();
    string getGPUMemory();
    string getDeviceType();
};

#endif // SYSTEMCORE_H
