#include "include/Manager.h"
#include <iostream>
#include <vector>

using namespace std;

Manager::Manager(SystemCore &s) : sc(s) {}

void Manager::controlPanel()  { sc.runCMD("control"); }
void Manager::taskManager()   { sc.runCMD("taskmgr"); }
void Manager::computerMgmt()  { sc.runCMD("compmgmt.msc"); }
void Manager::services()      { sc.runCMD("services.msc"); }
void Manager::registry()      { sc.runCMD("regedit"); }
void Manager::deviceManager() { sc.runCMD("devmgmt.msc"); }
void Manager::lockPC()        { sc.runCMD("rundll32.exe user32.dll,LockWorkStation"); }

bool Manager::ServiceControlAPI(string serviceName, DWORD startupType, bool stopService) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm) return false;

    SC_HANDLE svc = OpenServiceA(scm, serviceName.c_str(), SERVICE_CHANGE_CONFIG | SERVICE_STOP | SERVICE_START);
    if (!svc) { CloseServiceHandle(scm); return false; }

    bool configSuccess = ChangeServiceConfigA(svc, SERVICE_NO_CHANGE, startupType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    if (stopService) {
        SERVICE_STATUS status;
        ControlService(svc, SERVICE_CONTROL_STOP, &status);
    }
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return configSuccess;
}

void Manager::turnOffServicesMenu() {
    struct SvcInfo { string name; string desc; };
    vector<SvcInfo> targetSvcs = {
        // Windows 11 Widgets & Notification
        {"WpnService", "Windows Push Notifications System (Widgets)"},
        {"WpcSvc", "Parental Controls (Quản lý trẻ em/Widgets)"},

        {"XblAuthManager", "Xbox Live Auth Manager (Game)"},
        {"XblGameSave", "Xbox Live Game Save (Lưu game)"},
        {"XboxNetApiSvc", "Xbox Live Networking Service"},

        // Thêm vào danh sách tối ưu
        {"WpnService", "Windows Push Notifications (Triệt tiêu Widgets)"},
        {"WpnUserService", "Windows Push Notifications User (Tắt WebView2 ngầm)"},
        {"wisvc", "Windows Insider Service (Dịch vụ dùng thử - không cần thiết)"},

        // Telemetry & Tracking (Thu thập dữ liệu)
        {"DiagTrack", "Connected User Experiences (Telemetry)"},
        {"dmwappushservice", "WAP Push Message Routing Service"},

        // Maps & Location
        {"MapsBroker", "Downloaded Maps Manager (Bản đồ)"},
        // Remote & Printing (Nếu không dùng máy in/điều khiển từ xa)
        {"RemoteRegistry", "Điều chỉnh Registry từ xa"},
        // Khác
        {"SysMain", "Superfetch (Tối ưu ổ đĩa - Nên tắt nếu dùng SSD)"},
        {"WalletService", "Wallet Service (Ví điện tử Windows)"}
    };

    cout << "====================================================\n";
    cout << "   DANH SÁCH CÁC DỊCH VỤ SẼ ĐƯỢC TÓI ƯU (MANUAL):\n";
    for (const auto &s : targetSvcs) cout << "   - " << s.desc << " [" << s.name << "]\n";
    cout << "====================================================\n";

    cout << "[?] Bạn muốn tối ưu cách nào?\n";
    cout << "[1] Tối ưu thụ động (Manual - Có thể bật khi cần)\n";
    cout << "[2] Tắt hoàn toàn   (Disabled - Tiết kiệm tài nguyên)\n";
    cout << "[0] Hủy bỏ\n";
    int choice = sc.readInt("Chon: ");
    if (choice == 0) return;

    DWORD startType = (choice == 1) ? SERVICE_DEMAND_START : SERVICE_DISABLED;
    string modeName = (choice == 1) ? "MANUAL" : "DISABLED";

    for (const auto &s : targetSvcs) {
        if (ServiceControlAPI(s.name, startType, true)) {
            cout << "[OK] Đã thiết lập " << modeName << " cho: " << s.name << "\n";
        } else {
            cout << "[!] Thất bại: " << s.name << " (Kiem tra quyen Admin)\n";
        }
    }
    cout << "\n[SUCCESS] Hoàn tất tiến trình tối ưu dịch vụ!\n";
}