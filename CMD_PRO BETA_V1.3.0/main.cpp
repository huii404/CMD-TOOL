#include <winsock2.h>
#include <winternl.h>
#include <intrin.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <map>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <sstream>
#include <limits>
#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include <filesystem>
#include "SystemCore.h"

namespace fs = std::filesystem;
#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct HTTPRequest {
    string method, path, httpVersion;
    map<string, string> headers;
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

        void showDashboard(){
            double totalRAM, freeRAM;
            sc.getRAMInfo(totalRAM, freeRAM);
            double usedPercent = ((totalRAM - freeRAM) / totalRAM) * 100;

            cout << "\n================ SYSTEM INFO ================\n";
            cout << "CPU      : " << sc.getCPUModel() << endl;
            cout << "CPU Core : " << sc.getCPUCores() << " cores | " << fixed << setprecision(2) << sc.getCPUSpeed() << " GHz" << endl;
            cout << "GPU      : " << sc.getGPUModel() << " (" << sc.getGPUMemory() << ")" << endl;
            cout << "RAM      : " << fixed << setprecision(1) << totalRAM << " GB (Dùng: " << (totalRAM - freeRAM) << " GB, " << (int)usedPercent << "%)" << endl;
            cout << "Disk C   : " << sc.getDiskCStatus() << endl;
            cout << "Hostname : " << sc.getHostname() << endl;
            cout << "IPv4     : " << sc.getIPv4Address() << endl;
            cout << "OS       : " << sc.getWindowsVersion() << endl;
            cout << "Uptime   : " << sc.getUptime() << endl;
            cout << "User     : " << getenv("USERNAME") << endl;
            cout << "Device   : " << sc.getDeviceType() << endl;
            cout << "==============================================\n"<< endl;
        }
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

            // Thay đổi Startup Type (Manual hoặc Disabled)
            bool configSuccess = ChangeServiceConfigA(svc, SERVICE_NO_CHANGE, startupType,SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

            //  disable
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
            struct SvcInfo{
                string name;
                string desc;
            };
            vector<SvcInfo> targetSvcs = {
                // ===== GAME & XBOX (Không cần nếu không chơi game) =====
                {"XblAuthManager", "Xbox Live Auth Manager (Game)"},
                {"XblGameSave", "Xbox Live Game Save (Game)"},
                {"XboxNetApiSvc", "Xbox Live Networking Service (Game)"},
                {"XboxGipSvc", "Xbox Accessory Management (Game)"},
                
                // ===== BẢN ĐỒ & VỊTRỊ TRÍ =====
                {"MapsBroker", "Downloaded Maps Manager (Bản đồ)"},
                
                // ===== TELEMETRY & TRACKING =====
                {"DiagTrack", "Thu thập dữ liệu ngầm (Telemetry)"},
                {"TrkWks", "Tracking Service (Theo dõi - Lỗi thời)"},
                {"dmwappushservice", "Device Management Push (Lỗi thời)"},
                
                // ===== REGISTRY & REMOTE =====
                {"RemoteRegistry", "Điều chỉnh Registry từ xa"},
                {"RemoteAccess", "Routing and Remote Access (VPN - Ít dùng)"},
                
                // ===== FAX & ĐIỆN THOẠI =====
                {"Fax", "Dịch vụ gửi/nhận fax"},
                {"PhoneSvc", "Dịch vụ điện thoại"},
                
                // ===== CHẾ ĐỘ DEMO & KIOSK =====
                {"RetailDemo", "Chế độ máy trưng bày"},
                
                // ===== NGƯỜI DÙNG & ĐỒNG BỘ HÓA =====
                {"OneSyncSvc", "Sync Host (Microsoft Account Sync)"},
                
                // ===== ĐẦU VÀO BẢNG GHI & CẢUNG CẤP =====
                {"TabletInputService", "Tablet Input Service (Bảng ghi - Lỗi thời)"},
                
                // ===== CHIA SẺ & KẾT NỐI =====
                {"WMPNetworkSvc", "Windows Media Player Network Sharing"},
                {"SCardSvr", "Smart Card (Thẻ thông minh - Ít dùng)"},
                
                // ===== TẬP TIN & ĐỒNG BỘ =====
                {"FileSyncSvc", "OneDrive Sync (OneDrive - Tuỳ chọn)"},
                
                // ===== TỐI ƯU HÓA & CÁCH QUẢN LÝ =====
                {"DoSvc", "Delivery Optimization (Windows Update - Có thể tắt)"},
                {"QWAVE", "Quality Windows Audio Video (Ít dùng)"},
                
                // ===== QUẢN LÝ THIẾT BỊ =====
                {"DmEnrollmentSvc", "Device Management Enrollment (Lỗi thời)"},
                {"TieringEngineService", "Storage Tiers Management (Ít dùng)"},
                
                // ===== REMOTE ACCESS MANAGER =====
                {"RasAuto", "Remote Access Auto Connection Manager (Ít dùng)"},
                
                // ===== ỨNG DỤNG ẢOAO =====
                {"AppVClient", "Application Virtualization Client (Ít dùng)"},
                
                // ===== HYPER-V & DISK =====
                {"vmms", "Hyper-V Virtual Machine Management (Ít dùng)"},
                {"vds", "Virtual Disk Service (Ít dùng)"},
                
                // ===== BLUETOOTH & NFC =====
                {"SEMgrSvc", "Payments and NFC/SE Manager (NFC - Ít dùng)"},
            };

            cout << "====================================================\n";
            cout << "   DANH SÁCH CÁC DỊCH VỤ SẼ ĐƯỢC TÓI ƯU (MANUAL):\n";
            for (const auto &s : targetSvcs){
                cout << "   - " << s.desc << " [" << s.name << "]\n";
            }
            cout << "====================================================\n";

            cout << "[?] Bạn muốn tối ưu cách nào?\n";
            cout << "[1] Tối ưu thụ động (Manual - Có thể bật khi cần)\n";
            cout << "[2] Tắt hoàn toàn   (Disabled - Tiết kiệm tài nguyên)\n";
            cout << "[0] Hủy bỏ\n";
            int choice = sc.readInt("Chon: ");

            if (choice == 0) return;

            DWORD startType = (choice == 1) ? SERVICE_DEMAND_START : SERVICE_DISABLED;
            string modeName = (choice == 1) ? "MANUAL" : "DISABLED";

            for (const auto &s : targetSvcs){
                if (ServiceControlAPI(s.name, startType, true)){
                    cout << "[OK] Đã thiết lập " << modeName << " cho: " << s.name << "\n";
                }else{
                    cout << "[!] Thất bại: " << s.name << " (Kiem tra quyen Admin)\n";
                }
            }
            cout << "\n[SUCCESS] Hoàn tất tiến trình tối ưu dịch vụ!\n";
        }
       
        void lockPC()        { sc.runCMD("rundll32.exe user32.dll,LockWorkStation"); }
    };

    class Internet{
        SystemCore &sc;
        SOCKET listenSocket;
        int httpPort;
        string sharePath;
        string shareName;
        long long shareSize;
        int dlCount;
        
        //HẰNG SỐ GIỚI HẠN KÍCH THƯỚC 
        const long long MAX_FILE_SIZE = 1500000000LL; // 1.5 GB

        string getField(const string &line){
            size_t pos = line.find(":");
            if (pos != string::npos && pos + 2 < line.size()) return sc.trim(line.substr(pos + 2));
            return "";
        }

        string getContentType(const string &fpath){
            string ext = sc.trim(fpath.substr(fpath.find_last_of(".") != string::npos ? fpath.find_last_of(".") : fpath.length()));
            transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            map<string, string> types = {{".pdf", "application/pdf"}, {".txt", "text/plain"}, {".html", "text/html"}, {".zip", "application/zip"}, {".mp4", "video/mp4"}, {".mp3", "audio/mpeg"}, {".jpg", "image/jpeg"}, {".png", "image/png"}, {".exe", "application/octet-stream"}};
            return types.count(ext) ? types[ext] : "application/octet-stream";
        }

        string getLocalIP(){
            FILE *pipe = _popen("powershell -NoProfile -Command \"(Get-NetIPAddress -AddressFamily IPv4 | Where-Object {$_.IPAddress -like '192.168.*' -or $_.IPAddress -like '10.*'} | Select-Object -First 1).IPAddress\"","r");
            if (pipe){
                char buf[32] = {0};
                if (fgets(buf, sizeof(buf), pipe)){
                    string ip = sc.trim(string(buf));
                    _pclose(pipe);
                    if (!ip.empty() && ip != "0.0.0.0") return ip;
                }
                _pclose(pipe);
            }
            return "127.0.0.1";
        }

        void openFW(){
            char cmd[256];
            sprintf_s(cmd, sizeof(cmd), "netsh advfirewall firewall add rule name=\"QuickShare_%d\" dir=in action=allow protocol=tcp localport=%d >nul 2>&1", httpPort, httpPort);
            system(cmd);
        }

        string formatSize(long long b){
            const char *u[] = {"B", "KB", "MB", "GB"};
            double sz = (double)b;
            int i = 0;
            while (sz >= 1024 && i < 3){
                sz /= 1024;
                i++;
            }
            char buf[32];
            sprintf_s(buf, sizeof(buf), "%.2f %s", sz, u[i]);
            return string(buf);
        }

        string getTime(){
            time_t now = time(0);
            tm *t = localtime(&now);
            char buf[100];
            strftime(buf, sizeof(buf), "[%H:%M:%S]", t);
            return string(buf);
        }

        HTTPRequest parseReq(const string &raw){
            HTTPRequest req;
            istringstream iss(raw);
            string line;
            if (getline(iss, line)){
                istringstream ls(line);
                ls >> req.method >> req.path >> req.httpVersion;
            }
            while (getline(iss, line)){
                line = sc.trim(line);
                if (line.empty()) break;
                size_t p = line.find(":");
                if (p != string::npos){
                    req.headers[sc.trim(line.substr(0, p))] = sc.trim(line.substr(p + 1));
                }
            }
            return req;
        }

        void sendFile(SOCKET client){
            ifstream f(sharePath, ios::binary);
            if (!f.good()){
                string err = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 13\r\nConnection: close\r\n\r\nFile not found";
                send(client, err.c_str(), (int)err.length(), 0);
                return;
            }

            string ct = getContentType(sharePath);
            string hdr = "HTTP/1.1 200 OK\r\nContent-Type: " + ct + "\r\nContent-Length: " + to_string(shareSize) +"\r\nContent-Disposition: attachment; filename=\"" + shareName + "\"\r\nConnection: close\r\n\r\n";
            send(client, hdr.c_str(), (int)hdr.length(), 0);

            const int CHUNK = 1024 * 64;
            char buf[CHUNK];
            long long sent = 0;
            int prog = 0;
            cout << getTime() << " | Gửi: ";

            while (f.read(buf, CHUNK) || f.gcount() > 0){
                int toSend = (int)f.gcount();
                if (send(client, buf, toSend, 0) <= 0){
                    cout << "\n⚠️  Gián đoạn\n";
                    f.close();
                    return;
                }
                sent += toSend;
                int np = (int)((sent * 100) / shareSize);
                if (np > prog && np % 10 == 0){
                    cout << np << "% ";
                    cout.flush();
                }
                prog = np;
            }
            f.close();
            dlCount++;
            cout << "100% ✓ [" << dlCount << "]\n";
        }

        void handleClient(SOCKET client){
            const int BUFSIZE = 4096;
            char buf[BUFSIZE];
            int rcv = recv(client, buf, BUFSIZE - 1, 0);

            if (rcv > 0){
                buf[rcv] = '\0';
                HTTPRequest req = parseReq(buf);
                cout << getTime() << " | " << req.method << " " << req.path << "\n";

                if (req.method == "GET" && sharePath[0]){
                    sendFile(client);
                }
            }
            closesocket(client);
        }

        // HÀM KIỂM TRA KÍCH THƯỚC FILE
        bool checkFileSizeAndConfirm(const string &path, long long &outSize) {
            ifstream testFile(path, ios::binary);
            if (!testFile.good()) {
                cout << "[!] File không tồn tại!\n";
                return false;
            }

            testFile.seekg(0, ios::end);
            long long fileSize = testFile.tellg();
            testFile.close();

            if (fileSize == 0) {
                cout << "[!] File trống!\n";
                return false;
            }

            outSize = fileSize;

            // ===== KIỂM TRA GIỚI HẠN 1.5GB =====
            if (fileSize > MAX_FILE_SIZE) {
                cout << "\n" << string(70, '=') << "\n";
                cout << "[⚠️  CẢNH BÁO] File vượt quá giới hạn an toàn!\n";
                cout << "Kích thước file: " << formatSize(fileSize) << "\n";
                cout << "Giới hạn tối đa:  " << formatSize(MAX_FILE_SIZE) << "\n";
                cout << string(70, '=') << "\n";

                string confirm;
                cout << "\n[?] Bạn vẫn muốn tiếp tục? (Y/N): ";
                cin >> confirm;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                if (confirm != "y" && confirm != "Y") {
                    cout << "[i] Hủy bỏ quá trình chia sẻ.\n";
                    return false;
                }
            }

            return true;
        }

        // ===== HÀM LẤY THÔNG TIN DUNG LƯỢNG VÀ HỎI DUYỆT =====
        bool getFileSizeInfoAndPrompt(const string &path, long long &outSize) {
            cout << "\n[...] Đang lấy thông tin dung lượng file...\n";

            if (!checkFileSizeAndConfirm(path, outSize)) {
                return false;
            }

            cout << "\n[✓] Thông tin file:\n";
            cout << "    - Đường dẫn: " << path << "\n";
            cout << "    - Dung lượng: " << formatSize(outSize) << "\n";

            string proceedChoice;
            cout << "\n[?] Tiếp tục với quá trình chia sẻ? (Y/N): ";
            cin >> proceedChoice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (proceedChoice != "y" && proceedChoice != "Y") {
                cout << "[i] Người dùng đã hủy bỏ.\n";
                return false;
            }

            return true;
        }

    public:
        Internet(SystemCore &s) : sc(s), listenSocket(INVALID_SOCKET), httpPort(8080), shareSize(0), dlCount(0) {}

        ~Internet(){
            if (listenSocket != INVALID_SOCKET)
                closesocket(listenSocket);
        }

        void showIP(){sc.runCMD("ipconfig/all");}
        void renewIP() { sc.runCMD("ipconfig /renew"); }
        void flushdns() { sc.runCMD("ipconfig /flushdns"); }
        void netsh_tcpIP() { sc.runAdmin("netsh int ip reset"); }

        void wifiAudit(){
            FILE *pipe = _popen("netsh wlan show profiles", "r");
            if (!pipe){
                cout << "[!] Không chạy lệnh WiFi.\n";
                return;
            }
            char buf[512];
            int idx = 1;
            while (fgets(buf, sizeof(buf), pipe)){
                string line = buf;
                if (line.find("All User Profile") != string::npos){
                    string wifi = getField(line);
                    cout << "\n[" << idx++ << "] WiFi: " << wifi << "\n";
                    string cmd = "netsh wlan show profile \"" + wifi + "\" key=clear";
                    FILE *p2 = _popen(cmd.c_str(), "r");
                    if (!p2) continue;
                    char b2[512];
                    string auth = "", cipher = "", pass = "(Open)";
                    while (fgets(b2, sizeof(b2), p2)){
                        string inf = b2;
                        if (inf.find("Authentication") != string::npos) auth = getField(inf);
                        if (inf.find("Cipher") != string::npos) cipher = getField(inf);
                        if (inf.find("Key Content") != string::npos) pass = getField(inf);
                    }
                    _pclose(p2);
                    cout << "Pass: " << pass << " | Auth: " << auth << " | Cipher: " << cipher << "\n" << string(50, '-');
                }
            }
            _pclose(pipe);
        }

        // ===== HÀM QUICKSHAREPRO CẬP NHẬT VỚI LOGIC MỚI =====
        void quickSharePRO(){
            sc.cls();
            string path;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "File (drag-drop or path): ";
            getline(cin, path);

            if (path.length() >= 2 && path[0] == '"' && path[path.length() - 1] == '"'){
                path = path.substr(1, path.length() - 2);
            }
            path = sc.trim(path);

            // ===== GỌI HÀM KIỂM TRA KÍCH THƯỚC VÀ LẤY THÔNG TIN =====
            if (!getFileSizeInfoAndPrompt(path, shareSize)){
                return; // Người dùng đã hủy bỏ
            }

            sharePath = path;
            shareName = path.substr(path.find_last_of("/\\") + 1);

            cout << "\nPort (mặc định 8080): ";
            string portStr;
            getline(cin, portStr);
            if (!portStr.empty()){
                try{
                    int p = stoi(portStr);
                    if (p >= 1024 && p <= 65535)httpPort = p;
                }catch (...){}
            }

            WSADATA wsa;
            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
                cout << "WSAStartup failed\n";
                return;
            }

            listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (listenSocket == INVALID_SOCKET){
                cout << "Socket failed\n";
                WSACleanup();
                return;
            }

            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons(httpPort);

            if (bind(listenSocket, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR){
                cout << "Bind failed: Port " << httpPort << " đang dùng\n";
                closesocket(listenSocket);
                WSACleanup();
                return;
            }

            listen(listenSocket, SOMAXCONN);

            string ip = getLocalIP();
            openFW();

            cout << "\n"<< string(70, '=') << "\n";
            cout << "Server chạy!\n";
            cout << "[IP]: " << ip << " | Port: " << httpPort << " | File: " << shareName << " (" << formatSize(shareSize) << ")\n";
            cout << "[URL]: http://" << ip << ":" << httpPort << "/" << shareName << "\n";

            cout << "\nCtrl+C để dừng\n";
            cout << string(70, '=') << "\n\n";

            while (true){
                SOCKET client = accept(listenSocket, nullptr, nullptr);
                if (client != INVALID_SOCKET){
                    thread([this, client](){ handleClient(client); }).detach();
                }
            }
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
            sc.runCMD("powershell -NoProfile -Command \"Clear-RecycleBin -Force -ErrorAction SilentlyContinue\"");
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
            cout << "[Dọn rác]\n";          m.cleanDisk();
            cout << "[Giảm delay tat app]\n"; m.reduceShutdownTime();
            cout << "[Chặn cài app ngầm]\n";  m.Consumer_Content();
            cout << "[Tắt telemetry]\n";    m.windowsTelemetry();
            cout << "[Tắt hibernate]\n";    m.Hibernate();
            cout << "[OK] Hệ thống đã được tối ưu!\n";
        }

        void enableSecurityPRO(){
            cout << "[...] Đang kích hợp bảo mật...\n";
            sc.runAdmin("netsh advfirewall set allprofiles state on", true);// Bật Firewall
            sc.runAdmin("powershell -Command \"Set-MpPreference -DisableRealtimeMonitoring $false\"", true);// Bật Defender Real-time qua PowerShell
            sc.runAdmin("powershell -Command \"Set-MpPreference -EnableControlledFolderAccess Enabled\"", true);// Chống Ransomware 
            cout << "[OK] 1 số chức năng đã được bật!\n";
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
            int times      = sc.readInt("Só lần click: ");
            int intervalMs = sc.readInt("Delay giữa các lần (ms): ");
            int delaySec   = sc.readInt("Di chuyển chuột đến đích (giây): ");
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

        void ShowN_QR(int number){
            if (number >= 15 || number <= 0){
                cout << "[!] So luong khong hop le!\n";
                return;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            vector<string> list_qr; 
            int dem = 1;

            while (dem <= number){
                string text;
                cout << "[" << dem << "/" << number << "] Nhap noi dung QR: ";
                if (!getline(cin, text) || text.empty()){
                    cout << "[!] Noi dung khong duoc de trong!\n";
                    continue;
                }
                list_qr.push_back(text); 
                dem++;
            }

            for (int i = 0; i < list_qr.size(); i++){
                string current_text = list_qr[i];
                cout << "Dang lay ma QR thu " << i + 1 << "...\n";Sleep(4000); 
                for (char &c : current_text){
                    if (c == ' ')c = '+';
                }
                sc.runCMD("curl qrenco.de/" + current_text);
                cout << "\n--------------------------------------\n";
            }
        }

        void uninstallBloatware(){
            // Blacklist tập trung vào các app rác phổ biến
            vector<string> blacklist = {
                "BingWeather", "BingNews", "SolitaireCollection", "People",
                "PowerAutomateDesktop", "Todo", "GetHelp", "Getstarted",
                "OfficeHub", "SkypeApp", "YourPhone", "FeedbackHub",
                "ZuneVideo", "ZuneMusic", "MixedReality.Portal", "Clipchamp",
                "Disney", "Spotify", "TikTok", "Instagram" 
            };

            string listRac = "";
            for (size_t i = 0; i < blacklist.size(); ++i){
                listRac += blacklist[i];
                if (i < blacklist.size() - 1)listRac += "|";
            }

            cout << "[SYSTEM] Đang tiến hành xóa app rác\n";
            // 1. Xóa app khỏi User hiện tại và các User khác
            string cmd1 = "powershell -Command \"Get-AppxPackage -AllUsers | Where-Object {$_.Name -match '" + listRac + "'} | Remove-AppxPackage -AllUsers\"";
            // 2. Xóa gói cài đặt dự phòng (để không bị tự cài lại khi tạo User mới)
            string cmd2 = "powershell -Command \"Get-AppxProvisionedPackage -Online | Where-Object {$_.DisplayName -match '" + listRac + "'} | Remove-AppxProvisionedPackage -Online\"";
            sc.runAdmin(cmd1, true);
            sc.runAdmin(cmd2, true);
            cout << "\n[SUCCESS] Hệ thống đã xóa 1 số app\n";
        }
};

    class Explorer{
        SystemCore &sc;
    public:
        Explorer(SystemCore &s) : sc(s) {}

        // Thuật toán Hybrid: XOR kết hợp dịch bit động (Mạnh hơn XOR cũ)
        char strongByteProcess(char byte, string key, int position, bool isEncrypt) {
            if (key.empty()) return byte;
            char k = key[position % key.length()];
            if (isEncrypt) {
                // Mã hóa: XOR rồi dịch chuyển theo vị trí
                return (byte ^ k) + (position % 7);
            } else {
                // Giải mã: Trừ dịch chuyển trước rồi mới XOR ngược
                return (byte - (position % 7)) ^ k;
            }
        }

        void processFileBinary(bool isEncrypt){
            string fileName, key;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "\n--- " << (isEncrypt ? "MÃ HÓA" : "GIẢI MÃ") << " FILE BINARY ---" << endl;
            cout << "Tên file: "; getline(cin, fileName);
            cout << "Mật khẩu: "; getline(cin, key);

            // Mở file ở chế độ nhị phân (ios::binary)
            ifstream inFile(fileName, ios::binary);
            if (!inFile.is_open()) {
                cout << "[!] Lỗi: Không mở được file!\n";
                return;
            }

            string tempFile = "secure_binary.tmp";
            ofstream outFile(tempFile, ios::binary);
            
            char byte;
            int pos = 0;

            cout << "[System] Đang xử lý chính xác từng byte...\n";
            
            // Đọc từng byte cho đến hết file
            while (inFile.get(byte)) {
                char processedByte = strongByteProcess(byte, key, pos, isEncrypt);
                outFile.put(processedByte);
                pos++;
                
                if (pos % 1000 == 0) cout << "-> Đã xử lý: " << pos << " bytes\r";
            }

            inFile.close();
            outFile.close();

            remove(fileName.c_str());
            rename(tempFile.c_str(), fileName.c_str());
            cout << "\n[OK] Đã " << (isEncrypt ? "mã hóa" : "giải mã") << " xong " << pos << " bytes dữ liệu.\n";
        }

        void clearBrowserCache(){
            char *localAppData = std::getenv("LOCALAPPDATA");
            char *appData = std::getenv("APPDATA"); // Cần thêm để lấy đường dẫn Opera cũ
            if (!localAppData)return;

            string baseLocal = string(localAppData);
            string baseRoaming = appData ? string(appData) : "";

            vector<string> cachePaths = {
                // --- Google Chrome ---
                baseLocal + "\\Google\\Chrome\\User Data\\Default\\Cache",
                baseLocal + "\\Google\\Chrome\\User Data\\Default\\Code Cache",
                baseLocal + "\\Google\\Chrome\\User Data\\Default\\GPUCache",

                // --- Microsoft Edge ---
                baseLocal + "\\Microsoft\\Edge\\User Data\\Default\\Cache",
                baseLocal + "\\Microsoft\\Edge\\User Data\\Default\\Code Cache",
                baseLocal + "\\Microsoft\\Edge\\User Data\\Default\\GPUCache",

                // --- Cốc Cốc ---
                baseLocal + "\\CocCoc\\Browser\\User Data\\Default\\Cache",
                baseLocal + "\\CocCoc\\Browser\\User Data\\Default\\Code Cache",
                baseLocal + "\\CocCoc\\Browser\\User Data\\Default\\GPUCache",

                // --- Opera (Thường nằm trong Roaming hoặc Local tùy phiên bản) ---
                baseLocal + "\\Opera Software\\Opera Stable\\Cache",
                baseLocal + "\\Opera Software\\Opera Stable\\Code Cache",
                baseLocal + "\\Opera Software\\Opera Stable\\GPUCache",
                baseRoaming + "\\Opera Software\\Opera Stable\\Service Worker\\CacheStorage"};

            cout << "====================================================\n";
            cout << "[SYSTEM] DANG DON DEP CACHE MULTI-BROWSER...\n";
            cout << "====================================================\n";

            for (const string &path : cachePaths){
                if (path.empty())continue;
                if (fs::exists(path)){
                    cout << "[...] Cleaning: " << path << "\n";
                    try{
                        for (const auto &entry : fs::directory_iterator(path)){
                            fs::remove_all(entry.path());
                        }
                        cout << " [OK]\n";
                    }catch (const fs::filesystem_error &){
                        cout << " [!] Busy (Browser is open)\n";
                    }
                }
            }
            cout << "\n[SUCCESS] Hoan tat dọn dep Cache!\n";
        }

        void nguytrangFolder(){
            string path, name1, nameZip, name3;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Vị trí (vd: D:\\Data): ";getline(cin, path);
            cout << "Ảnh nền (abc.jpg): ";getline(cin, name1);
            cout << "File zip (abc.zip): ";getline(cin, nameZip);
            cout << "Tên đầu ra (abc.png): ";getline(cin, name3);

            string cmd = "cd /d \"" + path + "\" && copy /b \"" + name1 + "\" + \"" + nameZip + "\" \"" + name3 + "\"";
            sc.runCMD(cmd);
            cout << "\n[OK] Đã tạo file ngụy trang tại: " << path << "\\" << name3 << endl;
        }
    };
};

class AppUI : public SystemCore {
    Function::Internet   internet;
    Function::Maintenance main_;
    Function::Infomation info;
    Function::Manager    manager;
    Function::optimal    op;
    Function::AutoActions a;
    Function::Extension  exten;
    Function::Explorer exp;

    bool titleSet = false;

public:
// list khởi tạo thành viên
//(*this) con trỏ đến chính object hiện tại | liên kết con-cha
// dùng chung 1 vùng nhớ-> load app nhanh+giảm % ram tiêu hao
    AppUI(): internet(*this), main_(*this), info(*this), manager(*this), op(*this, internet, main_, exten), a(*this), exten(*this),exp(*this) {}

    void intro() {
        runCMD("title VER 1.3.0 PRO");
        cls();
        cout << string(55, '=') << "\n";
        setColor(3); cout << getTime() << " | "<<getDeviceType()<<" | Github: Huii404 \n";
        setColor(7);
        cout << string(55, '=') << "\n";
    }

    void mainMenu() {
        cout << "\n[1] Thông tin hệ thống           [2] Quản lý hệ thống";
        cout << "\n[3] Mạng                         [4] Bảo trì";
        cout << "\n[5] Tiện ích                     [6] Tối ưu hóa PRO";
        cout << "\n[7] Chuyển bản windows           [8] File/Folder";
        cout << "\n[0] Thoát\n";
        cout << "\n[Chon]: ";
    }

    void menuThongTin()  { cout << "[1] Phần mềm\n[2] System Info\n[3] Driver\n[0] Back\n[Chon]: "; }
    void menuQuanLy()    { cls(); cout << "[1] Control Panel\n[2] Task Manager\n[3] Computer Mgmt\n[4] Services\n[5] Registry\n[6] Device Manager\n[7] Lock\n[0] Back\n[Chon]: "; }
    void menuMang()      { cls(); cout << "[1] Xem IP\n[2] Renew IP\n[3] WiFi Password\n[4] Flush DNS\n[5] Reset TCP/IP\n[6] Chia sẻ File qua Web\n[0] Back\n[Chon]: "; }
    void menuBaoTri()    { cls(); cout << "[1] Dọn rác\n[2] Quét Virus\n[3] SFC\n[4] Check Disk\n[5] Xóa lịch sử bảo trì\n[6] Chặn tải app ngầm\n[7] Tắt hibernate\n[8] Restart\n[9] Tắt telemetry\n[10] Xóa app rác\n[0]  Back\nChon: "; }
    void menuTienIch()   { cls(); cout << "[1] Auto Click\n[2] Spam Text\n[3] Auto Dán Data\n[4] Tạo QR\n[0] Back\n[Chon]: "; }
    void menuToiUu()     { cls(); cout << "[1] Tối ưu hệ thống PRO\n[2] Tối ưu mạng\n[3] Tối ưu bảo mật\n[0] Back\n[Chon]: "; }
    void file_folder()   { cls(); cout << "[1] Ngụy trang thư mục\n[2] Nén file/folder\n[3] Mã hóa nội dung file\n[4] Xóa cache trình duyệt\n[0] Back\n[chon]: ";}

    int readSub() {
        int sub;
        if (!(cin >> sub)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); return -1; }
        return sub;
    }

    void run(){
        int mainChoice;
        while (true) {
            intro(); mainMenu();
            mainChoice = readSub();
            if (mainChoice <= 0) { if (mainChoice == 0) break; continue; }

            int sub;
            switch (mainChoice) {

            case 1:
                while (true) {
                    cls();
                    info.showDashboard();
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
                        int ans = readInt("\n[1] Mở services   [2] Tắt dịch vụ services   [0] Back ");
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
                    if (sub == 6) internet.quickSharePRO();
                    waitEnter();
                } break;

            case 4:
                while (true) {
                    menuBaoTri(); sub = readSub(); log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) main_.cleanDisk();
                    if (sub == 2) {
                        int ans = readInt("[1] QuickScan   [2] FullScan  [0] Back: ");
                        if(ans==0) break;
                        if (ans == 1) main_.QuickScanVirus();
                        else          main_.FullScanVirus();
                    }
                    if (sub == 3)  runAdmin("sfc /scannow");
                    if (sub == 4)  runAdmin("chkdsk C: /f /r");
                    if (sub == 5)  main_.clearEventLogs();
                    if (sub == 6)  main_.Consumer_Content();
                    if (sub == 7)  main_.Hibernate();
                    if (sub == 8)  main_.restart();
                    if (sub == 9) main_.windowsTelemetry();
                    if (sub == 10){string c;
                        cout<<"\nsẽ có 1 vài app chính bị gỡ,bạn có muốn gỡ(y/n): ";cin>>c;
                        if(c=="y")exten.uninstallBloatware();
                    }
                    waitEnter();
                } break;

            case 5:
                while (true) {
                    menuTienIch(); sub = readSub(); log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) a.autoClickPoint();
                    if (sub == 2) a.spamText();
                    if (sub == 3) a.autoPasteData();
                    if (sub == 4) {
                        string line;
                        cout << "\n\n[1] 1 Mã QR\n[2] N mã QR\n[0] BACK\n[chon]: "; sub=readSub();
                        if (sub == 0) break;
                        if (sub == 1) { cin.ignore(); cout << "Nhap text: "; getline(cin, line); exten.ShowQR(line); }
                        if (sub == 2) { cout << "Số lượng QR: "; cin >> sub; exten.ShowN_QR(sub); }
                    }
                    waitEnter();
                } break;

            case 6:
                while (true) {
                    menuToiUu(); sub = readSub(); log(mainChoice, sub);
                    if (sub == 0) break;
                    if (sub == 1) op.optimizeSystemPRO();
                    if (sub == 2) op.optimizeNetworkPRO();
                    if (sub == 3) op.enableSecurityPRO();
                    waitEnter();
                } break;

            case 7:
                while (true) { cls();
                    cout << "\n[1] Chạy kiểm tra       [0] Back\n[chon]: ";sub=readSub();
                    log(mainChoice,sub);
                    if (sub == 1) op.upgradeWindowsEditionPRO();
                    if (sub == 0) break;
                    waitEnter();
                } break;

            case 8:
                while (true){
                    file_folder();  sub = readSub();
                    log(mainChoice, sub);
                    if (sub == 0)break;
                    if (sub == 1)exp.nguytrangFolder();
                    if (sub == 2){
                        cout << "\n[1] Nén   [2] Giải nén   [0] Back\n[CHON]:";sub = readSub();
                        if (sub == 1){
                            string path, path2;
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "Đường dẫn nguồn: ";getline(cin, path);
                            cout << "Đường dẫn đích (.zip): ";getline(cin, path2);
                            ZIP(path, path2, 1);
                        }
                        if (sub == 2){
                            string zipPath, destPath;
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "File .zip: ";getline(cin, zipPath);
                            cout << "Đường đẫn đích: ";getline(cin, destPath);
                            UNZIP(zipPath, destPath);
                        }
                        if(sub==0) break;
                    }
                    if (sub == 3){
                        cout << "\n[1] Mã hóa nội dung(.txt)    [2] Giải mã nội dung(.txt)    [0] Back\n[CHON]: ";sub=readSub();
                        if(sub==1)exp.processFileBinary(true);
                        if(sub==2)exp.processFileBinary(false);
                        if (sub == 0)break;
                    }
                    if(sub==4)exp.clearBrowserCache();
                    waitEnter();
                }break;
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