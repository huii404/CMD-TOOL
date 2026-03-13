#include <iostream>
#include <windows.h>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <limits>
#include <vector>
#include <string>
using namespace std;


class SystemCore {
public:
    void setColor(int color) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    }

    void cls() { system("cls"); }

    string getTime() {
        time_t now = time(0);
        tm *t = localtime(&now);
        stringstream ss;
        ss << "[" << setfill('0') << setw(2) << t->tm_mday << "/" << setw(2) << t->tm_mon + 1
           << "/" << t->tm_year + 1900 << "-" << setw(2) << t->tm_hour << ":" << setw(2)
           << t->tm_min << ":" << setw(2) << t->tm_sec << "]";
        return ss.str();
    }

    void log(int mainChoice, int subChoice) {
        ofstream f("History.txt", ios::app);
        if (f.is_open())
            f << getTime() << ": [" << mainChoice << "]-[" << subChoice << "]\n";
    }

    void runCMD(const string &cmd) {
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi = {};
        string fullCmd = "cmd.exe /c " + cmd;
        vector<char> commandLine(fullCmd.begin(), fullCmd.end());
        commandLine.push_back('\0');
        if (CreateProcessA(NULL, commandLine.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            cout << "[!] Khong the chay lenh. Error code: " << GetLastError() << "\n";
        }
    }

    bool runAdmin(const string &cmd, bool silent = false) {
        if (!silent) {
            string answer;
            cout << "CHAY QUYEN ADMIN CHO LENH [" << cmd << "] (Y/N): ";
            cin >> answer;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (answer != "y" && answer != "Y") {
                cout << "Bo qua lenh.\n";
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


    string getDeviceType() {
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

    void waitEnter() {
        cout << "\nNHAN ENTER DE QUAY LAI...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }

    int readInt(const string &prompt) {
        int val;
        while (true) {
            cout << prompt;
            if (cin >> val) return val;
            cout << "[!] Vui long nhap so nguyen!\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    void leftClick() {
        INPUT input[2] = {};
        input[0].type = INPUT_MOUSE; input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        input[1].type = INPUT_MOUSE; input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(2, input, sizeof(INPUT));
    }

    void setClipboard(const string &text) {
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

    void pressCtrlV() {
        INPUT inputs[4] = {};
        for (int i = 0; i < 4; i++) inputs[i].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_CONTROL;
        inputs[1].ki.wVk = 'V';
        inputs[2].ki.wVk = 'V'; inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
        inputs[3].ki.wVk = VK_CONTROL; inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(4, inputs, sizeof(INPUT));
    }

    void pressEnter() {
        keybd_event(VK_RETURN, 0, 0, 0);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
    }

    void ZIP(string source, string destination, int level) {
        if (level < 0) level = 0;
        if (level > 9) level = 9;
        if (destination.find(".zip") == string::npos) destination += ".zip";
        string cmd = "7za.exe a -tzip \"" + destination + "\" \"" + source + "\" -mx" + to_string(level) + " -mmt=on";
        cout << "\n[System] Dang khoi tao tien trinh nen...\n[i] Che do: ";
        if      (level == 0) cout << "Copy (Store)\n";
        else if (level <= 3) cout << "Nhanh\n";
        else if (level <= 6) cout << "Tieu chuan\n";
        else                 cout << "Nen sau\n";
        int result = system(cmd.c_str());
        if (result == 0) cout << "[OK] Nen hoan tat!\n";
        else             cout << "[!] Loi thuc thi (Ma loi: " << result << ")\n";
    }

    void UNZIP(string zipFile, string extractPath) {
        if (zipFile.empty() || extractPath.empty()) { cout << "[!] Duong dan khong hop le.\n"; return; }
        string cmd = "7za.exe x \"" + zipFile + "\" -o\"" + extractPath + "\" -y";
        cout << "\n[System] Dang giai nen: " << zipFile << "...\n";
        int result = system(cmd.c_str());
        if (result == 0) cout << "[OK] Giai nen xong vao: " << extractPath << "\n";
        else             cout << "[X] Giai nen that bai! Ma loi: " << result << "\n";
    }

    string trim(string s) {
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ')) s.pop_back();
        while (!s.empty() && s.front() == ' ') s.erase(0, 1);
        return s;
    }
};

class Function {
public:
    class Infomation {
        SystemCore &sc;
    public:
        Infomation(SystemCore &s) : sc(s) {}
        void showSoftware() { sc.runCMD("powershell -NoProfile -Command \"Get-ItemProperty 'HKLM:\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\*' | Select-Object DisplayName, DisplayVersion\""); }
        void systemInfo()   { sc.runCMD("systeminfo"); }
        void driverList()   { sc.runCMD("driverquery /v"); }
    };

    class Manager {
        SystemCore &sc;
    public:
        Manager(SystemCore &s) : sc(s) {}
        void controlPanel()  { sc.runCMD("control"); }
        void taskManager()   { sc.runCMD("taskmgr"); }
        void computerMgmt()  { sc.runCMD("compmgmt.msc"); }
        void services()      { sc.runCMD("services.msc"); }
        void registry()      { sc.runCMD("regedit"); }
        void deviceManager() { sc.runCMD("devmgmt.msc"); }
        
        // api win + tương tác serivces
        bool ServiceControlAPI(string serviceName, DWORD startupType, bool stopService){
            SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
            if (!scm) return false;

            SC_HANDLE svc = OpenServiceA(scm, serviceName.c_str(), SERVICE_CHANGE_CONFIG | SERVICE_STOP | SERVICE_START);
            if (!svc){
                CloseServiceHandle(scm);
                return false;
            }

            // 1. Thay đổi Startup Type (Manual hoặc Disabled)
            bool configSuccess = ChangeServiceConfigA(svc, SERVICE_NO_CHANGE, startupType,SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

            // 2. Nếu cần dừng ngay lập tức
            if (stopService){
                SERVICE_STATUS status;
                ControlService(svc, SERVICE_CONTROL_STOP, &status);
            }

            CloseServiceHandle(svc);
            CloseServiceHandle(scm);
            return configSuccess;
        }
        // logic tắt/bật/in các gói dịch vụ
        void turnOffServicesMenu(){
            // Danh sách minh bạch để tạo sự tin tưởng
            struct SvcInfo{
                string name;
                string desc;
            };
            vector<SvcInfo> targetSvcs = {
                {"XblAuthManager", "Xbox Live Auth Manager (Game)"},
                {"XblGameSave", "Xbox Live Game Save (Game)"},
                {"XboxNetApiSvc", "Xbox Live Networking Service (Game)"},
                {"XboxGipSvc", "Xbox Accessory Management (Game)"},
                {"Spooler", "Print Spooler (Máy in - Ít dùng)"},
                {"MapsBroker", "Downloaded Maps Manager (Bản đồ)"}
            };

            cout << "====================================================\n";
            cout << "   DANH SACH CAC DICH VU SE DUOC TOI UU (MANUAL):\n";
            for (const auto &s : targetSvcs){
                cout << "   - " << s.desc << " [" << s.name << "]\n";
            }
            cout << "====================================================\n";

            cout << "[?] Bạn muốn tối ưu cách nào?\n";
            cout << "[1] Tối ưu thụ động (Manual - Có thể bật khi cần)\n";
            cout << "[2] Tắt hoàn toàn   (Disabled - Tiết kiệm tài nguyên)\n";
            cout << "[0] Hủy bỏ\n";
            int choice = sc.readInt("Chon: ");

            if (choice == 0)
                return;

            DWORD startType = (choice == 1) ? SERVICE_DEMAND_START : SERVICE_DISABLED;
            string modeName = (choice == 1) ? "MANUAL" : "DISABLED";

            for (const auto &s : targetSvcs)
            {
                if (ServiceControlAPI(s.name, startType, true))
                {
                    cout << "[OK] Da thiet lap " << modeName << " cho: " << s.name << "\n";
                }
                else
                {
                    cout << "[!] That bai: " << s.name << " (Kiem tra quyen Admin)\n";
                }
            }
            cout << "\n[SUCCESS] Hoan tat tien trinh toi uu dich vu!\n";
        }
       
        void lockPC()        { sc.runCMD("rundll32.exe user32.dll,LockWorkStation"); }
    };

    class Internet {
        SystemCore &sc;
        string getField(const string &line) {
            size_t pos = line.find(":");
            if (pos != string::npos && pos + 2 < line.size()) return sc.trim(line.substr(pos + 2));
            return "";
        }
    public:
        Internet(SystemCore &s) : sc(s) {}
        void showIP()      { sc.runCMD("netsh interface ip show config"); }
        void renewIP()     { sc.runCMD("ipconfig /renew"); }
        void flushdns()    { sc.runCMD("ipconfig /flushdns"); }
        void netsh_tcpIP() { sc.runAdmin("netsh int ip reset"); }

        void wifiAudit() {
            FILE *pipe = _popen("netsh wlan show profiles", "r");
            if (!pipe) { cout << "[!] Khong the chay lenh WiFi.\n"; return; }
            char buffer[512];
            int index = 1;
            while (fgets(buffer, sizeof(buffer), pipe)) {
                string line = buffer;
                if (line.find("All User Profile") != string::npos) {
                    string wifiName = getField(line);
                    cout << "\n[" << index++ << "] WiFi: " << wifiName << "\n";
                    string cmd = "netsh wlan show profile \"" + wifiName + "\" key=clear";
                    FILE *pipe2 = _popen(cmd.c_str(), "r");
                    if (!pipe2) { cout << "   [!] Khong lay duoc thong tin.\n"; continue; }
                    char buffer2[512];
                    string auth = "", cipher = "", pass = "(Open network)";
                    while (fgets(buffer2, sizeof(buffer2), pipe2)) {
                        string info = buffer2;
                        if (info.find("Authentication") != string::npos) auth   = getField(info);
                        if (info.find("Cipher")         != string::npos) cipher = getField(info);
                        if (info.find("Key Content")    != string::npos) pass   = getField(info);
                    }
                    _pclose(pipe2);
                    cout << "   Password : " << pass << "\n   Auth     : " << auth << "\n   Cipher   : " << cipher << "\n------------------------------------\n";
                }
            }
            _pclose(pipe);
        }
    };

    class Maintenance {
        SystemCore &sc;
    public:
        Maintenance(SystemCore &s) : sc(s) {}

        void cleanDisk() {
            string ans;
            cout << "[CANH BAO] Se xoa Temp, Prefetch, Recent. Tiep tuc? (Y/N): "; cin >> ans;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (ans != "y" && ans != "Y") { cout << "Bo qua.\n"; return; }
            cout << "[...] Dang don rac...\n";
            sc.runCMD("cmd /c del /s /f /q \"%temp%\\*\" & rd /s /q \"%temp%\" & md \"%temp%\"");
            sc.runCMD("cmd /c del /s /f /q \"%systemroot%\\temp\\*\" & rd /s /q \"%systemroot%\\temp\" & md \"%systemroot%\\temp\"");
            sc.runCMD("cmd /c del /s /f /q \"%systemroot%\\Prefetch\\*\"");
            sc.runCMD("cleanmgr /sagerun:1");
            sc.runAdmin("dism /online /cleanup-image /startcomponentcleanup");
            sc.runCMD("cmd /c del /f /s /q \"%AppData%\\Microsoft\\Windows\\Recent\\*\"");
            cout << "[OK] Don rac hoan tat.\n";
        }

        void QuickScanVirus() { sc.runCMD("\"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 1"); }
        void FullScanVirus()  { sc.runCMD("\"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 2"); }

        void restart() {
            string ans;
            cout << "Xac nhan RESTART may? (Y/N): "; cin >> ans;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (ans == "y" || ans == "Y") sc.runCMD("shutdown /r /t 5");
            else                          cout << "Huy restart.\n";
        }

        void Consumer_Content()  { sc.runCMD("reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager\" /v \"SilentInstalledAppsEnabled\" /t REG_DWORD /d 0 /f"); }
        void clearEventLogs()    { sc.runAdmin("powershell -Command \"Get-EventLog -LogName * | ForEach { Clear-EventLog $_.Log }\""); }
        void Hibernate()         { sc.runAdmin("powercfg -h off"); }
        void windowsTelemetry()  { sc.runAdmin("reg add \"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection\" /v AllowTelemetry /t REG_DWORD /d 0 /f"); }
        void reduceShutdownTime(){ sc.runCMD("reg add \"HKCU\\Control Panel\\Desktop\" /v \"WaitToKillAppTimeout\" /t REG_SZ /d \"2000\" /f"); }

        void customize_Registry() {
            sc.runCMD("powershell Set-ItemProperty -Path \"HKCU:\\Control Panel\\Desktop\" -Name \"VisualFXSetting\" -Value 2");
            sc.runCMD("powershell Set-ItemProperty -Path \"HKCU:\\Control Panel\\Desktop\\WindowMetrics\" -Name \"MinAnimate\" -Value 0");
            sc.runCMD("powershell Set-ItemProperty -Path \"HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" -Name \"TaskbarAnimations\" -Value 0");
        }
    };

    class Hardware {
        SystemCore &sc;
    public:
        Hardware(SystemCore &s) : sc(s) {}

        void brightness() {
            int level = sc.readInt("Nhap do sang (1-100): ");
            if (level < 1 || level > 100) { cout << "[!] Gia tri phai tu 1-100.\n"; return; }
            string cmd = "powershell -Command \"(Get-CimInstance -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1," + to_string(level) + ")\"";
            sc.runCMD(cmd);
        }

        void turnOffMonitor() {
            sc.runCMD("powershell (Add-Type '[DllImport(\"user32.dll\")]^public static extern int SendMessage(int hWnd,int hMsg,int wParam,int lParam);' -name a -pas)::SendMessage(-1,0x0112,0xF170,2)");
        }
    };

    class Extension;
    class optimal {
        SystemCore &sc;
        Internet &n;
        Maintenance &m;
        Extension &e;
    public:
        optimal(SystemCore &s, Internet &net, Maintenance &mnt, Extension &exten) : sc(s), n(net), m(mnt), e(exten) {}

        void optimizeSystemPRO() {
            cout << "[...] Bat dau toi uu he thong toan dien...\n\n";
            cout << "[Don rac]\n";          m.cleanDisk();
            cout << "[Giam UI lag]\n";      m.customize_Registry();
            cout << "[Giam delay tat app]\n"; m.reduceShutdownTime();
            cout << "[Chan cai app ngam]\n";  m.Consumer_Content();
            cout << "[Tat telemetry]\n";    m.windowsTelemetry();
            cout << "[Tat hibernate]\n";    m.Hibernate();
            cout<<"[Xóa app rác]\n";e.uninstallBloatware();
            cout << "[OK] He thong da duoc toi uu!\n";
        }

        void optimizeNetworkPRO() {
            cout << "[...] Dang toi uu ket noi mang...\n";
            cout << "[Flush DNS]\n";   n.flushdns();
            cout << "[Reset TCP/IP]\n"; n.netsh_tcpIP();
            cout << "[Tat telemetry]\n"; m.windowsTelemetry();
            cout << "[OK] Mang da duoc thiet lap lai!\n";
        }

        string getCurrentOS() {
            FILE *pipe = _popen("systeminfo | findstr /B /C:\"OS Name\"", "r");
            if (!pipe) return "Unknown";
            char buffer[256];
            string result = "";
            if (fgets(buffer, sizeof(buffer), pipe)) result = buffer;
            _pclose(pipe);
            size_t pos = result.find("Windows");
            if (pos != string::npos) return sc.trim(result.substr(pos));
            return "Unknown";
        }

        void upgradeWindowsEditionPRO() {
            string currentOS = getCurrentOS();
            sc.cls();
            cout << "====================================================\n";
            cout << " PHIEN BAN HIEN TAI: " << currentOS << "\n";
            cout << "====================================================\n\n";

            string key = "";
            int choice;

            if (currentOS.find("Home") != string::npos) {
                cout << "[HE THONG DANG O BAN HOME]\n1. Home -> Pro\n2. Home -> Education\n3. Home -> Enterprise\n0. Quay lai\n";
                choice = sc.readInt("Chon: ");
                if (choice == 1)      key = "VK7JG-NPHTM-C97JM-9MPGT-3V66T";
                else if (choice == 2) key = "YNMGQ-8RYV3-4PGQ3-C8XTP-7CFBY";
                else if (choice == 3) key = "XGVPP-NMH47-7TTHJ-W3FW7-8HV2C";
            } else if (currentOS.find("Pro") != string::npos && currentOS.find("Workstation") == string::npos) {
                cout << "[HE THONG DANG O BAN PRO]\n1. Pro -> Enterprise\n2. Pro -> Education\n3. Pro -> Pro Workstation\n0. Quay lai\n";
                choice = sc.readInt("Chon: ");
                if (choice == 1)      key = "NPPR9-FWDCX-D2C8J-H872K-2YT43";
                else if (choice == 2) key = "NW6C2-QMPVW-D7KKK-3GKT6-VCFB2";
                else if (choice == 3) key = "DXG7C-N36C4-C4HTG-X4T3X-2YV77";
            } else if (currentOS.find("Education") != string::npos) {
                cout << "[HE THONG DANG O BAN EDUCATION]\n1. Education -> Enterprise\n0. Quay lai\n";
                choice = sc.readInt("Chon: ");
                if (choice == 1) key = "XGVPP-NMH47-7TTHJ-W3FW7-8HV2C";
            } else {
                cout << "[!] PHIEN BAN NAY CHUA DUOC HO TRO CHUYEN DOI NHANH.\n";
                return;
            }

            if (!key.empty()) {
                cout << "\n[CANH BAO] QUA TRINH NANG CAP SE MAT VAI PHUT.\n";
                string confirm;
                cout << "XAC NHAN VOI KEY: [" << key << "]? (Y/N): "; cin >> confirm;
                if (confirm == "y" || confirm == "Y") {
                    string cmd = "changepk.exe /ProductKey " + key;
                    sc.runAdmin(cmd, true);
                }
            }
        }
    };

    class AutoActions {
        SystemCore &sc;
    public:
        AutoActions(SystemCore &s) : sc(s) {}

        void autoClickPoint() {
            cout << "--- AUTO CLICK TAI VI TRI ---\n";
            int times      = sc.readInt("So lan click: ");
            int intervalMs = sc.readInt("Delay giua cac lan (ms): ");
            int delaySec   = sc.readInt("Thoi gian chuan bi - di chuyen chuot (giay): ");
            if (times <= 0 || intervalMs < 0 || delaySec < 0) { cout << "[!] Gia tri khong hop le.\n"; return; }
            cout << "\nDua chuot den vi tri can click...\n";
            for (int i = delaySec; i > 0; i--) { cout << i << "... "; cout.flush(); Sleep(1000); }
            POINT p; GetCursorPos(&p);
            cout << "\nBat dau click tai: (" << p.x << ", " << p.y << ")\n";
            for (int i = 0; i < times; i++) { SetCursorPos(p.x, p.y); sc.leftClick(); Sleep(intervalMs); }
            cout << "Da xong!\n";
        }

        void spamText() {
            string content;
            cout << "\nText: ";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, content);
            if (content.empty()) { cout << "[!] Text trong.\n"; return; }
            int times   = sc.readInt("So lan: ");
            int delayMs = sc.readInt("Delay (ms): ");
            int countdown = 3;
            cout << "\nClick vao o nhap lieu trong " << countdown << " giay...\n";
            for (int i = countdown; i > 0; i--) { cout << i << "... "; cout.flush(); Sleep(1000); }
            cout << "\n";
            for (int i = 0; i < times; i++) { sc.setClipboard(content); sc.pressCtrlV(); sc.pressEnter(); Sleep(delayMs); }
        }

        void autoPasteData() {
            int n = sc.readInt("So dong du lieu: ");
            int delayMs = sc.readInt("Delay (ms): ");
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (n <= 0) { cout << "[!] So dong phai > 0.\n"; return; }
            vector<string> dataList(n);
            for (int i = 0; i < n; i++) { cout << "Dong [" << i + 1 << "]: "; getline(cin, dataList[i]); }
            int countdown = 3;
            cout << "\nClick vao o nhap lieu trong " << countdown << " giay...\n";
            for (int i = countdown; i > 0; i--) { cout << i << "... "; cout.flush(); Sleep(1000); }
            cout << "\n";
            for (const string &data : dataList) { sc.setClipboard(data); sc.pressCtrlV(); sc.pressEnter(); Sleep(delayMs); }
        }
    };

    class Extension {
        SystemCore &sc;
    public:
        Extension(SystemCore &s) : sc(s) {}

        bool text_processing(const string &text) {
            if (text.empty()) return true;
            if (text.length() > 99) { cout << "[!] Van ban qua dai (Max 99 ky tu).\n"; return true; }
            return false;
        }

        void ShowQR(string text) {
            if (text.length() > 99) { cout << "[!] Text qua dai!\n"; return; }
            for (char &c : text) if (c == ' ') c = '+';
            sc.runCMD("curl qrenco.de/" + text);
        }

        void ShowN_QR(int number) {
            if (number >= 15 || number <= 0) { cout << "[!] So luong khong hop le!\n"; return; }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            int count = 1;
            while (count <= number) {
                string text;
                cout << "[" << count << "/" << number << "] Nhap noi dung QR: ";
                if (!getline(cin, text) || text.empty()) continue;
                if (text_processing(text)) continue;
                cout << "Dang lay ma QR... (Cho 4s chong Spam)\n";
                Sleep(4000);
                for (char &c : text) if (c == ' ') c = '+';
                sc.runCMD("curl qrenco.de/" + text);
                cout << "\n--------------------------------------------\n";
                count++;
            }
        }

        void uninstallBloatware(){
            // 1. Danh sách "Cấm xóa" (Whitelist) - Những app hệ thống cực kỳ quan trọng
            // Mình đã thêm các thành phần login và cài đặt để máy không bị lỗi sau khi quét
            vector<string> whitelist = {
                "Store",               // Microsoft Store (Chợ ứng dụng)
                "Calculator",          // Máy tính
                "Photos",              // Xem ảnh
                "StickyNotes",         // Ghi chú
                "Paint",               // Vẽ
                "WindowsTerminal",     // Terminal mới
                "AAD.BrokerPlugin",    // Đăng nhập tài khoản Microsoft
                "DesktopAppInstaller", // Bộ cài đặt app .appxbundle
                "Windows.CBS",         // Thành phần hệ thống quan trọng
                "Edge"                 // Trình duyệt (Nếu bạn muốn giữ)
            };

            // 2. Chuyển vector thành chuỗi regex cho PowerShell
            string listCam = "";
            for (size_t i = 0; i < whitelist.size(); ++i){
                listCam += whitelist[i];
                if (i < whitelist.size() - 1)
                    listCam += "|";
            }

            cout << "====================================================\n";
            cout << "[SYSTEM] DANG QUET VA GO BO APP RAC (BLOATWARE)...\n";
            cout << "[LOGIC] Giu lai: " << listCam << "\n";
            cout << "====================================================\n";

            // 3. Thực thi lệnh PowerShell thông qua hàm runAdmin có sẵn của bạn
            // Dùng dấu nháy đơn ' ' quanh listCam để PowerShell hiểu là chuỗi regex
            string cmd = "powershell -Command \"Get-AppxPackage | Where-Object {$_.Name -notmatch '" + listCam + "'} | Remove-AppxPackage\"";

            // Gọi hàm runAdmin từ SystemCore
            if (sc.runAdmin(cmd, true)){
                cout << "\n[OK] Da hoan tat qua trinh don dep App rác!\n";
            }else{
                cout << "\n[!] Co loi xay ra hoac nguoi dung tu choi quyen Admin.\n";
            }
        }
    };
};


class AppUI : public SystemCore {
    Function::Internet   internet;
    Function::Maintenance main_;
    Function::Infomation info;
    Function::Manager    manager;
    Function::Hardware   h;
    Function::optimal    op;
    Function::AutoActions a;
    Function::Extension  exten;

    bool titleSet = false;

public:
// list khởi tạo thành viên
//(*this) con trỏ đến chính object hiện tại | liên kết con-cha
// dùng chung 1 vùng nhớ-> load app nhanh+giảm % ram tiêu hao
    AppUI(): internet(*this), main_(*this), info(*this), manager(*this), h(*this), op(*this, internet, main_, exten), a(*this), exten(*this) {}

    void intro() {
        cls();
        if (!titleSet) {
            system("title Toolkit by huii404");
            titleSet = true;
        }
        cout << "====================================================\n";
        setColor(4); cout << "DEVICE : " << getDeviceType() << "\n";
        setColor(5); cout << "VERSION: 1.3.0 PRO | Github: Huii404\n";
        setColor(6); cout << "Gmail  : hcao84539@gmail.com\n";
        setColor(7);
        cout << "====================================================\n";
    }

    void mainMenu() {
        cout << "\n[1] Thống tin hệ thống         [2] Quản lý hệ thống";
        cout << "\n[3] Mạng                       [4] Bảo trì";
        cout << "\n[5] Phần cứng                  [6] Tiện ích";
        cout << "\n[7] Tối ưu hóa PRO             [8] Chuyển phiên bản windows";
        cout << "\n[9] Cài Win bằng cmd(comming soon)       [0] Thoát\n";
        cout << "\n[Chon]: ";
    }

    void menuThongTin()  { cls(); cout << "[1] Phần mềm   [2] System Info   [3] Driver   [0] Back\n[Chon]: "; }
    void menuQuanLy()    { cls(); cout << "[1] Control Panel  [2] Task Manager  [3] Computer Mgmt\n[4] Services      [5] Registry      [6] Device Manager\n[7] Lock           [0] Back\n[Chon]: "; }
    void menuMang()      { cls(); cout << "[1] Xem IP      [2] Renew IP       [3] WiFi Password\n[4] Flush DNS   [5] Reset TCP/IP   [0] Back\n[Chon]: "; }
    void menuBaoTri()    { cls(); cout << "[1]  Dọn rác                [2]  Quet Virus\n[3]  SFC                    [4]  Check Disk\n[5]  Xoa lich su bao tri    [6]  Chan tai app ngam\n[7]  Giam UI-lag            [8]  Tat hibernate\n[9]  Restart                [10] Tat telemetry\n[11] Xóa app rác\n[0]  Back\nChon: "; }
    void menuPhanCung()  { cls(); cout << "[1] Độ sáng     [0] Back\n[Chon]: "; }
    void menuTienIch()   { cls(); cout << "[1] Auto Click\n[2] Spam Text\n[3] Auto Dan Data\n[4] Nen file/folder\n[5] Giai nen file/folder\n[6] Tao QR\n[0] Back\n[Chon]: "; }
    void menuToiUu()     { cls(); cout << "[1] Tối ưu hệ thống PRO   [2] Toi uu mang   [0] Back\n[Chon]: "; }

    int readSub() {
        int sub;
        if (!(cin >> sub)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); return -1; }
        return sub;
    }

    void run() {
        int mainChoice;
        while (true) {
            intro(); mainMenu();
            mainChoice = readSub();
            if (mainChoice <= 0) { if (mainChoice == 0) break; continue; }

            int sub;
            switch (mainChoice) {

            case 1:
                while (true) {
                    menuThongTin(); sub = readSub(); log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) info.showSoftware();
                    if (sub == 2) info.systemInfo();
                    if (sub == 3) info.driverList();
                    waitEnter();
                } break;

            case 2:
                while (true) {
                    menuQuanLy(); sub = readSub(); log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) manager.controlPanel();
                    if (sub == 2) manager.taskManager();
                    if (sub == 3) manager.computerMgmt();
                    if (sub == 4){
                        int ans = readInt("\n[1] Mở services   [2] Tắt dịch vụ services [0] Exit ");
                        if(sub==1) manager.services();
                        if(sub==2)manager.turnOffServicesMenu();
                        if(sub==0) break;
                    }
                    if (sub == 5) manager.registry();
                    if (sub == 6) manager.deviceManager();
                    if (sub == 7) manager.lockPC();
                    waitEnter();
                } break;

            case 3:
                while (true) {
                    menuMang(); sub = readSub(); log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) internet.showIP();
                    if (sub == 2) internet.renewIP();
                    if (sub == 3) internet.wifiAudit();
                    if (sub == 4) internet.flushdns();
                    if (sub == 5) internet.netsh_tcpIP();
                    waitEnter();
                } break;

            case 4:
                while (true) {
                    menuBaoTri(); sub = readSub(); log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) main_.cleanDisk();
                    if (sub == 2) {
                        int ans = readInt("[1] QuickScan  [2] FullScan: ");
                        if (ans == 1) main_.QuickScanVirus();
                        else          main_.FullScanVirus();
                    }
                    if (sub == 3)  runAdmin("sfc /scannow");
                    if (sub == 4)  runAdmin("chkdsk C: /f /r");
                    if (sub == 5)  main_.clearEventLogs();
                    if (sub == 6)  main_.Consumer_Content();
                    if (sub == 7)  main_.customize_Registry();
                    if (sub == 8)  main_.Hibernate();
                    if (sub == 9)  main_.restart();
                    if (sub == 10) main_.windowsTelemetry();
                    if (sub == 11)exten.uninstallBloatware();
                    waitEnter();
                } break;

            case 5:
                while (true) {
                    menuPhanCung(); sub = readSub(); log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) h.brightness();
                    waitEnter();
                } break;

            case 6:
                while (true) {
                    menuTienIch(); sub = readSub(); log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) a.autoClickPoint();
                    if (sub == 2) a.spamText();
                    if (sub == 3) a.autoPasteData();
                    if (sub == 4) {
                        string path, path2;
                        int level;
                        while (true) {
                            level = readInt("Level ZIP (0=Store, 1=Nhanh, 9=Toi da): ");
                            if (level >= 7) {
                                string confirm;
                                cout << "[CANH BAO] Level " << level << " rat cham, ton RAM/CPU. Tiep tuc? (Y/N): "; cin >> confirm;
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                if (confirm != "y" && confirm != "Y") continue;
                            } break;
                        }
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Duong dan nguon: "; getline(cin, path);
                        cout << "Duong dan dich (.zip): "; getline(cin, path2);
                        ZIP(path, path2, level);
                    }
                    if (sub == 5) {
                        string zipPath, destPath;
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "File .zip: "; getline(cin, zipPath);
                        cout << "Duong dan dich: "; getline(cin, destPath);
                        UNZIP(zipPath, destPath);
                    }
                    if (sub == 6) {
                        string line;
                        cout << "[1] 1 ma QR\n[2] N ma QR\n[0] BACK\n[chon]: "; cin >> sub;
                        if (sub == 0) break;
                        if (sub == 1) { cin.ignore(); cout << "Nhap text: "; getline(cin, line); exten.ShowQR(line); waitEnter(); }
                        if (sub == 2) { cout << "So luong QR: "; cin >> sub; exten.ShowN_QR(sub); waitEnter(); }
                    } else {
                        waitEnter();
                    }
                } break;

            case 7:
                while (true) {
                    menuToiUu(); sub = readSub(); log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) op.optimizeSystemPRO();
                    if (sub == 2) op.optimizeNetworkPRO();
                    waitEnter();
                } break;

            case 8:
                while (true) {
                    int choice; cls();
                    cout << "[1] Chay kieu tra       [0] Exit\n[chon]: "; cin >> choice;
                    log(mainChoice, choice);
                    if (choice == 1) op.upgradeWindowsEditionPRO();
                    if (choice == 0) break;
                    waitEnter();
                } break;
            }
        }
    }
};

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    AppUI app;
    app.run();
    return 0;
}