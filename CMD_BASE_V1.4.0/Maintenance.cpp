#include "include/Maintenance.h"
#include <iostream>
#include <limits>

using namespace std;

Maintenance::Maintenance(SystemCore &s) : sc(s) {}

void Maintenance::QuickScanVirus() { sc.runCMD("\"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 1"); }
void Maintenance::FullScanVirus()  { sc.runCMD("\"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 2"); }
void Maintenance::Consumer_Content()  { sc.runCMD("reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager\" /v \"SilentInstalledAppsEnabled\" /t REG_DWORD /d 0 /f"); }
void Maintenance::clearEventLogs()    { sc.runAdmin("powershell -Command \"Get-EventLog -LogName * | ForEach { Clear-EventLog $_.Log }\"", true); }
void Maintenance::Hibernate()         { sc.runAdmin("powercfg -h off", true); }
void Maintenance::windowsTelemetry()  { sc.runAdmin("reg add \"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection\" /v AllowTelemetry /t REG_DWORD /d 0 /f", true); }
void Maintenance::reduceShutdownTime(){ sc.runCMD("reg add \"HKCU\\Control Panel\\Desktop\" /v \"WaitToKillAppTimeout\" /t REG_SZ /d \"2000\" /f"); }

void Maintenance::cleanDisk() {
    string ans;
    cout << "[CANH BAO] Se xoa Temp, Prefetch, Recent. Tiep tuc? (Y/N): "; cin >> ans;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (ans != "y" && ans != "Y") { cout << "Bo qua.\n"; return; }
    cout << "[...] Dang don rac...\n";
    sc.runCMD("cmd /c del /s /f /q \"%temp%\\*\" & rd /s /q \"%temp%\" & md \"%temp%\"");
    sc.runCMD("cmd /c del /s /f /q \"%systemroot%\\temp\\*\" & rd /s /q \"%systemroot%\\temp\" & md \"%systemroot%\\temp\"");
    sc.runCMD("cmd /c del /s /f /q \"%systemroot%\\Prefetch\\*\"");
    sc.runCMD("cleanmgr /sagerun:1");
    sc.runAdmin("dism /online /cleanup-image /startcomponentcleanup", true);
    sc.runCMD("cmd /c del /f /s /q \"%AppData%\\Microsoft\\Windows\\Recent\\*\"");
    sc.runCMD("powershell -NoProfile -Command \"Clear-RecycleBin -Force -ErrorAction SilentlyContinue\"");
    cout << "[OK] Don rac hoan tat.\n";
}

void Maintenance::restart() {
    string ans;
    cout << "Xac nhan RESTART may? (Y/N): "; cin >> ans;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (ans == "y" || ans == "Y") sc.runCMD("shutdown /r /t 5");
    else                          cout << "Huy restart.\n";
}
void Maintenance::disableAllStartupApps() {
    sc.cls();
    cout << "================ TỐI ƯU APP KHỞI ĐỘNG ================\n";
    cout << "[...] Đang quét và dọn dẹp các ứng dụng khởi động cùng Windows...\n\n";

    int removedCount = 0;

    // 1. CÁC NHÁNH REGISTRY CẦN QUÉT
    const struct {
        HKEY hKeyRoot;
        LPCSTR subKey;
        string name;
    } targets[] = {
        {HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", "HKCU"},
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "HKLM"},
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run", "HKLM_WOW64"}
    };

    // DANH SÁCH TRẮNG (Không được xóa để tránh lỗi Windows)
    vector<string> whitelist = {
        "SecurityHealth",  // Windows Defender
        "RtkAudUService",  // Realtek Audio
        "WavesSvc",        // MaxxAudio
        "IgfxTray",        // Intel Graphics
        "NvBackend",       // Nvidia
        "OneDrive"         // Tùy chọn (bạn có thể xóa dòng này nếu ghét OneDrive)
    };

    // Bắt đầu quét Registry
    for (const auto& target : targets) {
        HKEY hKey;
        if (RegOpenKeyExA(target.hKeyRoot, target.subKey, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
            char valueName[256];
            DWORD nameSize, type;
            DWORD index = 0;
            vector<string> toDelete;

            // Liệt kê tất cả các value trong nhánh
            while (true) {
                nameSize = sizeof(valueName);
                if (RegEnumValueA(hKey, index, valueName, &nameSize, NULL, &type, NULL, NULL) == ERROR_SUCCESS) {
                    string vName(valueName);
                    bool isSafe = false;
                    
                    for (const string& safeApp : whitelist) {
                        if (vName.find(safeApp) != string::npos) {
                            isSafe = true; break;
                        }
                    }
                    if (!isSafe) toDelete.push_back(vName);
                    index++;
                } else {
                    break;
                }
            }

            // Xóa các value không nằm trong danh sách trắng
            for (const string& delName : toDelete) {
                if (RegDeleteValueA(hKey, delName.c_str()) == ERROR_SUCCESS) {
                    cout << "[-] Đã tắt: " << delName << " (" << target.name << ")\n";
                    removedCount++;
                }
            }
            RegCloseKey(hKey);
        }
    }

    // 2. DỌN SẠCH THƯ MỤC STARTUP GỐC CỦA WINDOWS
    char* appData = getenv("APPDATA");
    char* programData = getenv("PROGRAMDATA");
    
    vector<string> startupFolders;
    if (appData) startupFolders.push_back(string(appData) + "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup");
    if (programData) startupFolders.push_back(string(programData) + "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup");

    namespace fs = std::filesystem;
    for (const auto& folder : startupFolders) {
        if (fs::exists(folder)) {
            for (const auto& entry : fs::directory_iterator(folder)) {
                if (entry.is_regular_file()) {
                    string fileName = entry.path().filename().string();
                    if (fileName != "desktop.ini") { // Bỏ qua file hệ thống ẩn
                        try {
                            fs::remove(entry.path());
                            cout << "[-] Đã xóa shortcut: " << fileName << " (Startup Folder)\n";
                            removedCount++;
                        } catch (...) {}
                    }
                }
            }
        }
    }

    // TỔNG KẾT
    if (removedCount > 0) {
        cout << "\n[SUCCESS] Đã tắt thành công " << removedCount << " tiến trình khởi động ngầm!\n";
    } else {
        cout << "\n[i] Hệ thống đã sạch sẽ, không có ứng dụng rác nào tự khởi động.\n";
    }
}
void Maintenance::updateAllApps() {
    cout << "[...] Đang kiểm tra và cập nhật toàn bộ ứng dụng qua Winget...\n";
    sc.runCMD("winget upgrade --all --silent"); // --silent để nó tự chạy không cần hỏi
    cout << "[OK] Quá trình cập nhật hoàn tất.\n";
}