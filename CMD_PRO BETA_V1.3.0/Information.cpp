#include "include/Information.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <intrin.h>
#include <psapi.h> // Cho hàm driverList

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")

using namespace std;

Information::Information(SystemCore &s) : sc(s) {}
Information::~Information() {}

// ================= NÂNG CẤP API HIỂN THỊ TRỰC TIẾP =================

void Information::showDashboard() {
    double totalRAM, freeRAM;
    getRAMInfo(totalRAM, freeRAM);
    double usedPercent = ((totalRAM - freeRAM) / totalRAM) * 100;

    cout << "\n================ SYSTEM INFO ================\n";
    cout << "CPU      : " << getCPUModel() << endl;
    cout << "CPU Core : " << getCPUCores() << " cores | " << fixed << setprecision(2) << getCPUSpeed() << " GHz" << endl;
    cout << "GPU      : " << getGPUModel() << " (" << getGPUMemory() << ")" << endl;
    cout << "RAM      : " << fixed << setprecision(1) << totalRAM << " GB (Dùng: " << (totalRAM - freeRAM) << " GB, " << (int)usedPercent << "%)" << endl;
    cout << "Disk C   : " << getDiskCStatus() << endl;
    cout << "Hostname : " << getHostname() << endl;
    cout << "IPv4     : " << getIPv4Address() << endl;
    cout << "OS       : " << getWindowsVersion() << endl;
    cout << "Uptime   : " << getUptime() << endl;
    cout << "User     : " << getenv("USERNAME") << endl;
    cout << "Device   : " << getDeviceType() << endl;
    cout << "==============================================\n"<< endl;
}

// Thay thế hoàn toàn PowerShell (chạy siêu nhanh)
void Information::showSoftware() {
    sc.cls();
    cout << "================ DANH SÁCH PHẦN MỀM ĐÃ CÀI ĐẶT ================\n";
    cout << "[System] Đang quét Registry...\n\n";

    const char* subkeys[] = {
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    };

    int count = 0;
    for (int i = 0; i < 2; ++i) {
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subkeys[i], 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char subKeyName[256];
            DWORD index = 0, nameSize = sizeof(subKeyName);
            while (RegEnumKeyExA(hKey, index, subKeyName, &nameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                HKEY hSubKey;
                if (RegOpenKeyExA(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    char displayName[256];
                    DWORD dataSize = sizeof(displayName);
                    if (RegQueryValueExA(hSubKey, "DisplayName", NULL, NULL, (LPBYTE)displayName, &dataSize) == ERROR_SUCCESS) {
                        char displayVersion[128] = "Unknown";
                        DWORD verSize = sizeof(displayVersion);
                        RegQueryValueExA(hSubKey, "DisplayVersion", NULL, NULL, (LPBYTE)displayVersion, &verSize);
                        
                        cout << "[-] " << displayName << " (Ver: " << displayVersion << ")\n";
                        count++;
                    }
                    RegCloseKey(hSubKey);
                }
                index++;
                nameSize = sizeof(subKeyName);
            }
            RegCloseKey(hKey);
        }
    }
    cout << "\n[OK] Tìm thấy " << count << " phần mềm.\n";
}

// Thay thế lệnh driverquery (Load cực nhanh bằng Psapi)
void Information::driverList() {
    sc.cls();
    LPVOID drivers[1024];
    DWORD cbNeeded;
    int cDrivers, i;

    cout << "================ DANH SÁCH DRIVER HỆ THỐNG ================\n";
    if (EnumDeviceDrivers(drivers, sizeof(drivers), &cbNeeded) && cbNeeded < sizeof(drivers)) {
        char szDriver[1024];
        cDrivers = cbNeeded / sizeof(drivers[0]);
        cout << "[System] Đang chạy " << cDrivers << " drivers:\n\n";

        for (i = 0; i < cDrivers; i++) {
            if (GetDeviceDriverBaseNameA(drivers[i], szDriver, sizeof(szDriver))) {
                cout << "[" << i + 1 << "] " << szDriver << "\n";
            }
        }
    } else {
        cout << "[!] Không thể lấy thông tin Driver bằng API.\n";
    }
}

// Bổ sung các thông số nâng cao thay vì chờ systeminfo
void Information::systemInfo() {
    sc.cls();
    cout << "================ CHI TIẾT HỆ THỐNG GỐC ================\n";
    
    // Thư mục hệ thống
    char sysDir[MAX_PATH];
    GetSystemDirectoryA(sysDir, MAX_PATH);
    cout << "System Directory : " << sysDir << "\n";

    // Độ phân giải màn hình
    int screenX = GetSystemMetrics(SM_CXSCREEN);
    int screenY = GetSystemMetrics(SM_CYSCREEN);
    cout << "Screen Resolution: " << screenX << " x " << screenY << "\n";

    // Số luồng xử lý chi tiết
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    cout << "Page Size        : " << sysInfo.dwPageSize << " Bytes\n";
    cout << "Processor Arch   : " << (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? "x64" : "x86/ARM") << "\n";
    
    // Gọi Dashboard cũ để bù thông tin
    showDashboard();
}

// ================= CÁC HÀM API CHUYỂN TỪ SYSTEMCORE =================

string Information::getCPUModel() {
    int cpuInfo[4] = {-1};
    char cpuBrand[49];
    __cpuid(cpuInfo, 0x80000002);
    memcpy(cpuBrand, cpuInfo, sizeof(cpuInfo));
    __cpuid(cpuInfo, 0x80000003);
    memcpy(cpuBrand + 16, cpuInfo, sizeof(cpuInfo));
    __cpuid(cpuInfo, 0x80000004);
    memcpy(cpuBrand + 32, cpuInfo, sizeof(cpuInfo));
    return string(cpuBrand);
}

string Information::getGPUModel() {
    HKEY hKey;
    char value[256] = "Integrated Graphics";
    DWORD vSize = sizeof(value);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WinSAT", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "PrimaryAdapterString", NULL, NULL, (LPBYTE)value, &vSize);
        RegCloseKey(hKey);
    }
    return string(value);
}

string Information::getDiskCStatus() {
    ULARGE_INTEGER free, total, tFree;
    if (GetDiskFreeSpaceExA("C:\\", &free, &total, &tFree)) {
        double totalGB = (double)total.QuadPart / (1024 * 1024 * 1024);
        double usedGB = totalGB - ((double)tFree.QuadPart / (1024 * 1024 * 1024));
        stringstream ss;
        ss << fixed << setprecision(1) << "C: " << usedGB << "/" << totalGB << " GB";
        return ss.str();
    }
    return "C: Unknown";
}

void Information::getRAMInfo(double &total, double &free) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    total = (double)memInfo.ullTotalPhys / (1024 * 1024 * 1024);
    free = (double)memInfo.ullAvailPhys / (1024 * 1024 * 1024);
}

string Information::getUptime() {
    ULONGLONG ms = GetTickCount64();
    int days = ms / (24 * 3600 * 1000);
    int hours = (ms / (3600 * 1000)) % 24;
    int mins = (ms / (60 * 1000)) % 60;
    return to_string(days) + "d " + to_string(hours) + "h " + to_string(mins) + "m";
}

string Information::getHostname() {
    static string cached = "";
    if (!cached.empty()) return cached;
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(hostname);
    if (GetComputerNameA(hostname, &size)) {
        cached = string(hostname);
        return cached;
    }
    cached = "Unknown";
    return cached;
}

string Information::getIPv4Address() {
    static string cached = "";
    if (!cached.empty()) return cached;

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cached = "0.0.0.0";
        return cached;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    addr.sin_addr.s_addr = inet_addr("8.8.8.8");

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0) {
        sockaddr_in local;
        int len = sizeof(local);
        if (getsockname(sock, (sockaddr*)&local, &len) == 0) {
            cached = inet_ntoa(local.sin_addr);
            closesocket(sock);
            return cached;
        }
    }
    closesocket(sock);
    cached = "127.0.0.1";
    return cached;
}

string Information::getWindowsVersion() {
    static string cached = "";
    if (!cached.empty()) return cached;

    HKEY hKey;
    char value[256] = "Unknown";
    DWORD vSize = sizeof(value);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)value, &vSize);
        RegCloseKey(hKey);
    }
    cached = string(value);
    return cached;
}

int Information::getCPUCores() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

double Information::getCPUSpeed() {
    HKEY hKey;
    DWORD speed = 0;
    DWORD vSize = sizeof(speed);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "~MHz", NULL, NULL, (LPBYTE)&speed, &vSize);
        RegCloseKey(hKey);
    }
    return speed > 0 ? (double)speed / 1000.0 : 0.0;
}

string Information::getGPUMemory() {
    HKEY hKey;
    DWORD memory = 0;
    DWORD vSize = sizeof(memory);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Intel\\GMM", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "DedicatedSegmentSize", NULL, NULL, (LPBYTE)&memory, &vSize);
        RegCloseKey(hKey);
    }

    if (memory > 0) {
        double memGB = (double)memory / (1024 * 1024 * 1024);
        stringstream ss;
        ss << fixed << setprecision(1) << memGB << " GB";
        return ss.str();
    }
    return "Shared";
}

string Information::getDeviceType() {
    static string cached = "";
    if (!cached.empty()) return cached;
    SYSTEM_POWER_STATUS sps;
    if (GetSystemPowerStatus(&sps)) {
        if (sps.BatteryFlag == 128) { cached = "Desktop"; return cached; }
        if (sps.BatteryLifePercent != 255) {
            stringstream ss;
            ss << "Laptop: " << (int)sps.BatteryLifePercent << "%";
            cached = ss.str();
            return cached;
        }
        cached = "Laptop - Battery Unknown";
        return cached;
    }
    cached = "Unknown";
    return cached;
}