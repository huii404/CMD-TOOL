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

/* ================= 1. SYSTEM CORE ================= */
class SystemCore {
public:

    void setColor(int color) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    }

    void cls() { system("cls"); }

    string getTime() {
        time_t now = time(0);
        tm* t = localtime(&now);
        stringstream ss;
        ss << "[" << setfill('0')<< setw(2) << t->tm_mday << "/"<< setw(2) << t->tm_mon + 1 << "/"<< t->tm_year + 1900 << "-"<< setw(2) << t->tm_hour << ":"<< setw(2) << t->tm_min << ":"<< setw(2) << t->tm_sec << "]";
        return ss.str();
    }

    void log(int mainChoice, int subChoice) {
        ofstream f("History.txt", ios::app);
        if (f.is_open()) {
            f << getTime() << ": [" << mainChoice << "]-[" << subChoice << "]" << endl;
        }
    }

    // ---- Chạy lệnh thường (không cần admin) ----
    void runCMD(const string& cmd) {
        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi = {};
        vector<char> commandLine(cmd.begin(), cmd.end());
        commandLine.push_back('\0');
        if (CreateProcessA(NULL, commandLine.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            cout << "[!] Khong the chay lenh: " << cmd.substr(0, 60) << "...\n";
        }
    }

    // ---- Chạy lệnh với quyền Admin ----
    bool runAdmin(const string& cmd, bool silent = false) {
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

        // Chuyển đổi UTF-8 sang wstring an toàn
        int wlen = MultiByteToWideChar(CP_UTF8, 0, cmd.c_str(), -1, NULL, 0);
        wstring wCmd(wlen, 0);
        MultiByteToWideChar(CP_UTF8, 0, cmd.c_str(), -1, &wCmd[0], wlen);

        wstring params = L"/c " + wCmd; // /c: chạy xong tự đóng, /k: giữ lại

        SHELLEXECUTEINFOW sei = { sizeof(sei) };
        sei.lpVerb       = L"runas";
        sei.lpFile       = L"cmd.exe";
        sei.lpParameters = params.c_str();
        sei.nShow        = SW_SHOWNORMAL;
        sei.fMask        = SEE_MASK_NOCLOSEPROCESS;

        if (ShellExecuteExW(&sei)) {
            cout << "[OK] Dang chay lenh voi quyen Admin...\n";
            if (sei.hProcess) {
                WaitForSingleObject(sei.hProcess, INFINITE);
                CloseHandle(sei.hProcess);
            }
            return true;
        } else {
            DWORD err = GetLastError();
            if (err == ERROR_CANCELLED)
                cout << "[!] Nguoi dung tu choi cap quyen Admin.\n";
            else
                cout << "[!] Khong the lay quyen Admin. (Ma loi: " << err << ")\n";
            return false;
        }
    }

    // ---- Device ----
    string getDeviceType() {
        SYSTEM_POWER_STATUS sps;
        if (GetSystemPowerStatus(&sps)) {
            if (sps.BatteryFlag == 128) return "Desktop";
            if (sps.BatteryLifePercent != 255) {
                stringstream ss;
                ss << "Laptop: " << (int)sps.BatteryLifePercent << "%";
                return ss.str();
            }
            return "Laptop - Battery Unknown";
        }
        return "Unknown";
    }

    // ---- Input helpers ----
    void waitEnter() {
        cout << "\nNHAN ENTER DE QUAY LAI...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }

    // Đọc số nguyên an toàn, tránh vòng lặp vô hạn khi nhập chữ
    int readInt(const string& prompt) {
        int val;
        while (true) {
            cout << prompt;
            if (cin >> val) return val;
            cout << "[!] Vui long nhap so nguyen!\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    // ---- Mouse / Keyboard ----
    void leftClick() {
        INPUT input[2] = {};
        input[0].type = INPUT_MOUSE; input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        input[1].type = INPUT_MOUSE; input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(2, input, sizeof(INPUT));
    }

    // FIX: kiểm tra SetClipboardData thất bại -> free hMem tránh memory leak
    void setClipboard(const string& text) {
        if (!OpenClipboard(nullptr)) return;
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (hMem) {
            memcpy(GlobalLock(hMem), text.c_str(), text.size() + 1);
            GlobalUnlock(hMem);
            if (!SetClipboardData(CF_TEXT, hMem))
                GlobalFree(hMem); // leak fix
        }
        CloseClipboard();
    }

    void pressCtrlV() {
        INPUT inputs[4] = {};
        for (int i = 0; i < 4; i++) inputs[i].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_CONTROL;
        inputs[1].ki.wVk = 'V';
        inputs[2].ki.wVk = 'V';        inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
        inputs[3].ki.wVk = VK_CONTROL; inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(4, inputs, sizeof(INPUT));
    }

    void pressEnter() {
        keybd_event(VK_RETURN, 0, 0, 0);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
    }

    // ---- ZIP / UNZIP ----
    void ZIP(string source, string destination, int level) {
        if (level < 0) level = 0;
        if (level > 9) level = 9;

        if (destination.find(".zip") == string::npos) destination += ".zip";

        string compressionLevel = "-mx" + to_string(level);
        string cmd = "7za.exe a -tzip \"" + destination + "\" \"" + source + "\" " + compressionLevel + " -mmt=on";

        cout << "\n[System] Dang khoi tao tien trinh nen...\n";
        cout << "[i] Che do: ";
        if      (level == 0) cout << "Copy (Store)\n";
        else if (level <= 3) cout << "Nhanh\n";
        else if (level <= 6) cout << "Tieu chuan\n";
        else                 cout << "Nen sau\n";

        int result = system(cmd.c_str());
        if (result == 0) cout << "[OK] Nen hoan tat!\n";
        else             cout << "[!] Loi thuc thi (Ma loi: " << result << ")\n";
    }

    void UNZIP(string zipFile, string extractPath) {
        if (zipFile.empty() || extractPath.empty()) {
            cout << "[!] Duong dan khong hop le.\n";
            return;
        }
        string cmd = "7za.exe x \"" + zipFile + "\" -o\"" + extractPath + "\" -y";
        cout << "\n[System] Dang giai nen: " << zipFile << "...\n";
        int result = system(cmd.c_str());
        if (result == 0) cout << "[OK] Giai nen xong vao: " << extractPath << "\n";
        else             cout << "[X] Giai nen that bai! Ma loi: " << result << "\n";
    }
};

/* ================= 2. FUNCTION ================= */
class Function {
public:
    SystemCore core;

    // ---- Thông tin ----
    void showSoftware() {
        core.runCMD("powershell -NoProfile -Command \"Get-ItemProperty 'HKLM:\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\*' | Select-Object DisplayName, DisplayVersion\"");
    }
    void systemInfo()  { core.runCMD("systeminfo"); }
    void driverList()  { core.runCMD("driverquery /v"); }

    // ---- Quản lý ----
    void controlPanel()  { core.runCMD("control"); }
    void taskManager()   { core.runCMD("taskmgr"); }
    void computerMgmt()  { core.runCMD("compmgmt.msc"); }
    void services()      { core.runCMD("services.msc"); }
    void registry()      { core.runCMD("regedit"); }
    void deviceManager() { core.runCMD("devmgmt.msc"); }
    void lockPC()        { core.runCMD("rundll32.exe user32.dll,LockWorkStation"); }

    // ---- Mạng ----
    void showIP()     { core.runCMD("netsh interface ip show config"); }
    void renewIP()    { core.runCMD("ipconfig /renew"); }
    void flushdns()   { core.runCMD("ipconfig /flushdns"); }
    // FIX: warning_cmdAdmin -> runAdmin, lệnh giờ thực sự được truyền và chạy
    void netsh_tcpIP() { core.runAdmin("netsh int ip reset"); }

    // ---- WiFi Audit ----
    string trim(string s) {
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' '))
            s.pop_back();
        return s;
    }

    string getField(const string& line) {
        size_t pos = line.find(":");
        if (pos != string::npos && pos + 2 < line.size())
            return trim(line.substr(pos + 2));
        return "";
    }

    void wifiAudit() {
        // FIX: kiểm tra pipe != NULL trước khi đọc
        FILE* pipe = _popen("netsh wlan show profiles", "r");
        if (!pipe) { cout << "[!] Khong the chay lenh WiFi.\n"; return; }

        char buffer[512];
        int index = 1;
        while (fgets(buffer, sizeof(buffer), pipe)) {
            string line = buffer;
            if (line.find("All User Profile") != string::npos) {
                string wifiName = getField(line);
                cout << "\n[" << index++ << "] WiFi: " << wifiName << "\n";

                string cmd = "netsh wlan show profile \"" + wifiName + "\" key=clear";
                FILE* pipe2 = _popen(cmd.c_str(), "r");
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

                cout << "   Password : " << pass   << "\n";
                cout << "   Auth     : " << auth   << "\n";
                cout << "   Cipher   : " << cipher << "\n";
                cout << "------------------------------------\n";
            }
        }
        _pclose(pipe);
    }

    // ---- Bảo trì ----
    // FIX: cleanDisk hỏi xác nhận trước khi xóa
    void cleanDisk() {
        string ans;
        cout << "[CANH BAO] Se xoa Temp, Prefetch, Recent. Tiep tuc? (Y/N): ";
        cin >> ans;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (ans != "y" && ans != "Y") { cout << "Bo qua.\n"; return; }

        cout << "[...] Dang don rac...\n";
        core.runCMD("cmd /c del /s /f /q \"%temp%\\*\" & rd /s /q \"%temp%\" & md \"%temp%\"");
        core.runCMD("cmd /c del /s /f /q \"%systemroot%\\temp\\*\" & rd /s /q \"%systemroot%\\temp\" & md \"%systemroot%\\temp\"");
        core.runCMD("cmd /c del /s /f /q \"%systemroot%\\Prefetch\\*\"");
        core.runCMD("cleanmgr /sagerun:1");
        core.runAdmin("dism /online /cleanup-image /startcomponentcleanup");
        core.runCMD("cmd /c del /f /s /q \"%AppData%\\Microsoft\\Windows\\Recent\\*\"");
        cout << "[OK] Don rac hoan tat.\n";
    }

    void QuickScanVirus() { core.runCMD("\"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 1"); }
    void FullScanVirus()  { core.runCMD("\"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 2"); }

    // FIX: restart hỏi xác nhận
    void restart() {
        string ans;
        cout << "Xac nhan RESTART may? (Y/N): ";cin >> ans;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (ans == "y" || ans == "Y") core.runCMD("shutdown /r /t 5");
        else cout << "Huy restart.\n";
    }

    void Consumer_Content() {core.runCMD("reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager\" /v \"SilentInstalledAppsEnabled\" /t REG_DWORD /d 0 /f");}
    void clearEventLogs() {core.runAdmin("powershell -Command \"Get-EventLog -LogName * | ForEach { Clear-EventLog $_.Log }\"");}
    void Hibernate() {core.runAdmin("powercfg -h off");}

    void customize_Registry() {
        core.runCMD("powershell Set-ItemProperty -Path \"HKCU:\\Control Panel\\Desktop\" -Name \"VisualFXSetting\" -Value 2");
        core.runCMD("powershell Set-ItemProperty -Path \"HKCU:\\Control Panel\\Desktop\\WindowMetrics\" -Name \"MinAnimate\" -Value 0");
        core.runCMD("powershell Set-ItemProperty -Path \"HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" -Name \"TaskbarAnimations\" -Value 0");
    }

    void reduceShutdownTime() { core.runCMD("reg add \"HKCU\\Control Panel\\Desktop\" /v \"WaitToKillAppTimeout\" /t REG_SZ /d \"2000\" /f"); }

    void windowsTelemetry() {core.runAdmin("reg add \"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection\" /v AllowTelemetry /t REG_DWORD /d 0 /f");}

    // ---- Phần cứng ----
    void brightness() {
        int level = core.readInt("Nhap do sang (1-100): ");
        if (level < 1 || level > 100) { cout << "[!] Gia tri phai tu 1-100.\n"; return; }
        string cmd = "powershell -Command \"(Get-CimInstance -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1," + to_string(level) + ")\"";
        core.runCMD(cmd);
    }

    void turnOffMonitor() {core.runCMD("powershell (Add-Type '[DllImport(\"user32.dll\")]^public static extern int SendMessage(int hWnd,int hMsg,int wParam,int lParam);' -name a -pas)::SendMessage(-1,0x0112,0xF170,2)");}

    // ---- Tối ưu PRO ----
    void optimizeSystemPRO() {
        cout << "[...] Bat dau toi uu he thong toan dien...\n\n";
        cout << "[Don rac]\n";          cleanDisk();
        cout << "[Giam UI lag]\n";      customize_Registry();
        cout << "[Giam delay tat app]\n"; reduceShutdownTime();
        cout << "[Chan cai app ngam]\n";  Consumer_Content();
        cout << "[Tat telemetry]\n";    windowsTelemetry();
        cout << "[Tat hibernate]\n";    Hibernate();
        cout << "[OK] He thong da duoc toi uu!\n";
    }

    void optimizeNetworkPRO() {
        cout << "[...] Dang toi uu ket noi mang...\n";
        cout << "[Flush DNS]\n";        flushdns();
        cout << "[Reset TCP/IP]\n";     netsh_tcpIP();
        cout << "[Tat telemetry]\n";    windowsTelemetry();
        cout << "[OK] Mang da duoc thiet lap lai!\n";
    }
};

/* ================= 3. AUTO ACTIONS ================= */
class AutoActions {
public:
    SystemCore core;

    void autoClickPoint() {
        cout << "--- AUTO CLICK TAI VI TRI ---\n";
        int times     = core.readInt("So lan click: ");
        int intervalMs = core.readInt("Delay giua cac lan (ms): ");
        int delaySec  = core.readInt("Thoi gian chuan bi - di chuyen chuot (giay): ");

        if (times <= 0 || intervalMs < 0 || delaySec < 0) {
            cout << "[!] Gia tri khong hop le.\n"; return;
        }

        cout << "\nDua chuot den vi tri can click...\n";
        for (int i = delaySec; i > 0; i--) { cout << i << "... "; cout.flush(); Sleep(1000); }

        POINT p; GetCursorPos(&p);
        cout << "\nBat dau click tai: (" << p.x << ", " << p.y << ")\n";

        for (int i = 0; i < times; i++) {
            SetCursorPos(p.x, p.y);
            core.leftClick();
            Sleep(intervalMs);
        }
        cout << "Da xong!\n";
    }

    void spamText() {
        string content;
        cout << "\nText: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, content);
        if (content.empty()) { cout << "[!] Text trong.\n"; return; }

        int times   = core.readInt("So lan: ");
        int delayMs = core.readInt("Delay (ms): ");
        // FIX: hiển thị đúng delay chuẩn bị và dùng biến countdown
        int countdown = 3;
        cout << "\nClick vao o nhap lieu trong " << countdown << " giay...\n";
        for (int i = countdown; i > 0; i--) { cout << i << "... "; cout.flush(); Sleep(1000); }
        cout << "\n";

        for (int i = 0; i < times; i++) {
            core.setClipboard(content);
            core.pressCtrlV();
            core.pressEnter();
            Sleep(delayMs);
        }
    }

    void autoPasteData() {
        int n       = core.readInt("So dong du lieu: ");
        int delayMs = core.readInt("Delay (ms): ");
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (n <= 0) { cout << "[!] So dong phai > 0.\n"; return; }

        vector<string> dataList(n);
        for (int i = 0; i < n; i++) {
            cout << "Dong [" << i + 1 << "]: ";
            getline(cin, dataList[i]);
        }

        int countdown = 3;
        cout << "\nClick vao o nhap lieu trong " << countdown << " giay...\n";
        for (int i = countdown; i > 0; i--) { cout << i << "... "; cout.flush(); Sleep(1000); }
        cout << "\n";

        for (const string& data : dataList) {
            core.setClipboard(data);
            core.pressCtrlV();
            core.pressEnter();
            Sleep(delayMs);
        }
    }
};

/* ================= 4. APP UI ================= */
class AppUI {
public:
    Function    f;
    AutoActions a;
    SystemCore  core; // chỉ dùng cho log/color/waitEnter trong UI

    void intro() {
        core.cls();
        system("title Toolkit by huii404");
        cout << "====================================================\n";
        core.setColor(4); cout << "DEVICE : " << core.getDeviceType() << "\n";
        core.setColor(5); cout << "VERSION: 1.3.0 PRO | Github: Huii404\n";
        core.setColor(6); cout << "Gmail  : hcao84539@gmail.com\n";
        core.setColor(7);
        cout << "====================================================\n";
    }

    // ---- Menu in ----
    void mainMenu() {
        cout << "\n[1] Thong tin he thong     [2] Quan ly he thong";
        cout << "\n[3] Mang                   [4] Bao tri";
        cout << "\n[5] Phan cung              [6] Tien ich";
        cout << "\n[7] Toi uu hoa PRO";
        cout << "\n[0] Thoat";
        cout << "\n\n[Chon]: ";
    }

    void menuThongTin()    { core.cls(); cout << "[1] Phan mem   [2] System Info   [3] Driver   [0] Back\n[Chon]: "; }
    void menuQuanLy()      { core.cls(); cout << "[1] Control Panel  [2] Task Manager  [3] Computer Mgmt\n[4] Services      [5] Registry      [6] Device Manager\n[7] Lock           [0] Back\n[Chon]: "; }
    void menuMang()        { core.cls(); cout << "[1] Xem IP   [2] Renew IP   [3] WiFi Password\n[4] Flush DNS   [5] Reset TCP/IP   [0] Back\n[Chon]: "; }
    void menuBaoTri()      {
        core.cls();
        cout << "[1]  Don rac                [2]  Quet Virus\n";
        cout << "[3]  SFC                    [4]  Check Disk\n";
        cout << "[5]  Xoa lich su bao tri    [6]  Chan tai app ngam\n";
        cout << "[7]  Giam UI-lag            [8]  Tat hibernate\n";
        cout << "[9]  Restart                [10] Tat telemetry\n";
        cout << "[0]  Back\nChon: ";
    }
    void menuPhanCung()    { core.cls(); cout << "[1] Do sang   [2] Tat man hinh   [0] Back\n[Chon]: "; }
    void menuTienIch()     { core.cls(); cout << "[1] Auto Click\n[2] Spam Text\n[3] Auto Dan Data\n[4] Nen file/folder\n[5] Giai nen file/folder\n[0] Back\n[Chon]: "; }
    void menuToiUu()       { core.cls(); cout << "[1] Toi uu he thong PRO   [2] Toi uu mang   [0] Back\n[Chon]: "; }

    // ---- Đọc sub với validate ----
    int readSub() {
        int sub;
        if (!(cin >> sub)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return -1;
        }
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
                    menuThongTin(); sub = readSub();
                    core.log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) f.showSoftware();
                    if (sub == 2) f.systemInfo();
                    if (sub == 3) f.driverList();
                    core.waitEnter();
                } break;

            case 2:
                while (true) {
                    menuQuanLy(); sub = readSub();
                    core.log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) f.controlPanel();
                    if (sub == 2) f.taskManager();
                    if (sub == 3) f.computerMgmt();
                    if (sub == 4) f.services();
                    if (sub == 5) f.registry();
                    if (sub == 6) f.deviceManager();
                    if (sub == 7) f.lockPC();
                    core.waitEnter();
                } break;

            case 3:
                while (true) {
                    menuMang(); sub = readSub();
                    core.log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) f.showIP();
                    if (sub == 2) f.renewIP();
                    if (sub == 3) f.wifiAudit();
                    if (sub == 4) f.flushdns();
                    if (sub == 5) f.netsh_tcpIP();
                    core.waitEnter();
                } break;

            case 4:
                while (true) {
                    menuBaoTri(); sub = readSub();
                    core.log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) f.cleanDisk();
                    if (sub == 2) {
                        int ans = core.readInt("[1] QuickScan  [2] FullScan: ");
                        if (ans == 1) f.QuickScanVirus();
                        else          f.FullScanVirus();
                    }
                    if (sub == 3)  core.runAdmin("sfc /scannow");
                    if (sub == 4)  core.runAdmin("chkdsk C: /f /r");
                    if (sub == 5)  f.clearEventLogs();
                    if (sub == 6)  f.Consumer_Content();
                    if (sub == 7)  f.customize_Registry();
                    if (sub == 8)  f.Hibernate();
                    if (sub == 9)  f.restart();
                    if (sub == 10) f.windowsTelemetry();
                    core.waitEnter();
                } break;

            case 5:
                while (true) {
                    menuPhanCung(); sub = readSub();
                    core.log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) f.brightness();
                    if (sub == 2) f.turnOffMonitor();
                    core.waitEnter();
                } break;

            case 6:
                while (true) {
                    menuTienIch(); sub = readSub();
                    core.log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) a.autoClickPoint();
                    if (sub == 2) a.spamText();
                    if (sub == 3) a.autoPasteData();
                    if (sub == 4) {
                        string path, path2;
                        int level;
                        // Nhập level với validate
                        while (true) {
                            level = core.readInt("Level ZIP (0=Store, 1=Nhanh, 9=Toi da): ");
                            if (level >= 7) {
                                string confirm;
                                cout << "[CANH BAO] Level " << level << " toc do rat cham, ton RAM/CPU. Tiep tuc? (Y/N): ";
                                cin >> confirm;
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                if (confirm != "y" && confirm != "Y") continue;
                            }
                            break;
                        }
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Duong dan nguon: "; getline(cin, path);
                        cout << "Duong dan dich (.zip): "; getline(cin, path2);
                        core.ZIP(path, path2, level);
                    }
                    if (sub == 5) {
                        string zipPath, destPath;
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "File .zip: ";      getline(cin, zipPath);
                        cout << "Thu muc dich: ";   getline(cin, destPath);
                        core.UNZIP(zipPath, destPath);
                    }
                    core.waitEnter();
                } break;

            case 7:
                while (true) {
                    menuToiUu(); sub = readSub();
                    core.log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) f.optimizeSystemPRO();
                    if (sub == 2) f.optimizeNetworkPRO();
                    core.waitEnter();
                } break;

            default:
                cout << "[!] Lua chon khong hop le.\n";
                Sleep(800);
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