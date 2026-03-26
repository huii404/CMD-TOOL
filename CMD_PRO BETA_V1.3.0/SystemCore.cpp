#include "SystemCore.h"
#include <intrin.h>
#include <winternl.h>

#pragma comment(lib, "ws2_32.lib")

// ==================== Constructor & Destructor ====================

SystemCore::SystemCore() {
    hJob = CreateJobObjectA(NULL, NULL);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {0};
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
}

SystemCore::~SystemCore() {
    if (hJob) CloseHandle(hJob);
}

// ==================== Basic Utilities ====================

void SystemCore::setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void SystemCore::cls() { 
    system("cls"); 
}

string SystemCore::getTime() {
    time_t now = time(0);
    tm *t = localtime(&now);
    stringstream ss;
    ss << "[" << setfill('0') << setw(2) << t->tm_mday << "/" << setw(2) << t->tm_mon + 1
       << "/" << t->tm_year + 1900 << "-" << setw(2) << t->tm_hour << ":" << setw(2)
       << t->tm_min << ":" << setw(2) << t->tm_sec << "]";
    return ss.str();
}

void SystemCore::waitEnter() {
    cout << "\n\nNHẤN ENTER ĐỂ QUAY LẠI...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

int SystemCore::readInt(const string &prompt) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val) return val;
        cout << "[!] Vui lòng nhập số nguyên!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// ==================== Command Execution ====================

void SystemCore::runCMD(const string &cmd) {
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi = {};
    string fullCmd = "cmd.exe /c " + cmd;
    vector<char> commandLine(fullCmd.begin(), fullCmd.end());
    commandLine.push_back('\0');

    if (CreateProcessA(NULL, commandLine.data(), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        AssignProcessToJobObject(hJob, pi.hProcess);
        ResumeThread(pi.hThread);
        
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

bool SystemCore::runAdmin(const string &cmd, bool silent) {
    if (!silent) {
        string answer;
        cout << "Chạy quyền Admin cho lệnh [" << cmd << "] (Y/N): ";cin >> answer;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (answer != "y" && answer != "Y") {
            cout << "Bỏ qua lệnh.\n";
            return false;
        }
    }
    int wlen = MultiByteToWideChar(CP_UTF8, 0, cmd.c_str(), -1, NULL, 0);
    wstring wCmd(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, cmd.c_str(), -1, &wCmd[0], wlen);
    wstring params = L"/k " + wCmd;

    SHELLEXECUTEINFOW sei = {sizeof(sei)};
    sei.lpVerb    = L"runas";
    sei.lpFile    = L"cmd.exe";
    sei.lpParameters = params.c_str();
    sei.nShow     = SW_SHOWNORMAL;
    sei.fMask     = SEE_MASK_NOCLOSEPROCESS;

    if (ShellExecuteExW(&sei)) {
        cout << "[OK] Dang chay lenh voi quyen Admin...\n";
        if (sei.hProcess) {
            WaitForSingleObject(sei.hProcess, INFINITE);
            CloseHandle(sei.hProcess);
        }
        return true;
    } else {
        DWORD err = GetLastError();
        if (err == ERROR_CANCELLED) cout << "[!] Nguoi dung tu choi cap quyen Admin.\n";
        else                        cout << "[!] Khong the lay quyen Admin. (Ma loi: " << err << ")\n";
        return false;
    }
}

// ==================== Mouse & Keyboard ====================

void SystemCore::leftClick() {
    INPUT input[2] = {};
    input[0].type = INPUT_MOUSE; input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input[1].type = INPUT_MOUSE; input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, input, sizeof(INPUT));
}

void SystemCore::setClipboard(const string &text) {
    if (!OpenClipboard(nullptr)) return;
    EmptyClipboard();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (hMem) {
        memcpy(GlobalLock(hMem), text.c_str(), text.size() + 1);
        GlobalUnlock(hMem);
        if (!SetClipboardData(CF_TEXT, hMem)) GlobalFree(hMem);
    }
    CloseClipboard();
}

void SystemCore::pressCtrlV() {
    INPUT inputs[4] = {};
    for (int i = 0; i < 4; i++) inputs[i].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[1].ki.wVk = 'V';
    inputs[2].ki.wVk = 'V'; inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].ki.wVk = VK_CONTROL; inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(4, inputs, sizeof(INPUT));
}

void SystemCore::pressEnter() {
    keybd_event(VK_RETURN, 0, 0, 0);
    keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
}

// ==================== File Operations ====================

void SystemCore::ZIP(string source, string destination, int level) {
    if (level < 0) level = 0;
    if (level > 9) level = 9;
    if (destination.find(".zip") == string::npos) destination += ".zip";
    string cmd = "7za.exe a -tzip \"" + destination + "\" \"" + source + "\" -mx" + to_string(level) + " -mmt=on";
    cout << "\n[System] Đang khởi tạo tiến trình nền...\n[i] Chế độ: ";
    if      (level == 0) cout << "Copy (Store)\n";
    else if (level <= 3) cout << "Nhanh\n";
    else if (level <= 6) cout << "Tiêu chuẩn\n";
    else                 cout << "Nén sau\n";
    int result = system(cmd.c_str());
    if (result == 0) cout << "[OK] Nén hòan tất!\n";
    else             cout << "[!] Lỗi thực thi (Ma loi: " << result << ")\n";
}

void SystemCore::UNZIP(string zipFile, string extractPath) {
    if (zipFile.empty() || extractPath.empty()) { cout << "[!] Duong dan khong hop le.\n"; return; }
    string cmd = "7za.exe x \"" + zipFile + "\" -o\"" + extractPath + "\" -y";
    cout << "\n[System] Đang giải nén: " << zipFile << "...\n";
    int result = system(cmd.c_str());
    if (result == 0) cout << "[OK] Giải nén xong vào: " << extractPath << "\n";
    else             cout << "[X] Giải nén thất bại! Mã lỗi: " << result << "\n";
}

// ==================== Utility ====================

string SystemCore::trim(string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ')) s.pop_back();
    while (!s.empty() && s.front() == ' ') s.erase(0, 1);
    return s;
}

// ==================== System Info - Hardware (11 hàm API) ====================

string SystemCore::getCPUModel() {
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

string SystemCore::getGPUModel() {
    HKEY hKey;
    char value[256] = "Integrated Graphics";
    DWORD vSize = sizeof(value);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WinSAT", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "PrimaryAdapterString", NULL, NULL, (LPBYTE)value, &vSize);
        RegCloseKey(hKey);
    }
    return string(value);
}

string SystemCore::getDiskCStatus() {
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

void SystemCore::getRAMInfo(double &total, double &free) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    total = (double)memInfo.ullTotalPhys / (1024 * 1024 * 1024);
    free = (double)memInfo.ullAvailPhys / (1024 * 1024 * 1024);
}

string SystemCore::getUptime() {
    ULONGLONG ms = GetTickCount64();
    int days = ms / (24 * 3600 * 1000);
    int hours = (ms / (3600 * 1000)) % 24;
    int mins = (ms / (60 * 1000)) % 60;
    return to_string(days) + "d " + to_string(hours) + "h " + to_string(mins) + "m";
}

string SystemCore::getHostname() {
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

string SystemCore::getIPv4Address() {
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

string SystemCore::getWindowsVersion() {
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

int SystemCore::getCPUCores() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

double SystemCore::getCPUSpeed() {
    HKEY hKey;
    DWORD speed = 0;
    DWORD vSize = sizeof(speed);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "~MHz", NULL, NULL, (LPBYTE)&speed, &vSize);
        RegCloseKey(hKey);
    }
    return speed > 0 ? (double)speed / 1000.0 : 0.0;
}

string SystemCore::getGPUMemory() {
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

string SystemCore::getDeviceType() {
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
