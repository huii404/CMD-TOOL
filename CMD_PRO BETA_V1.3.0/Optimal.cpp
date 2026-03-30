#include "include/Optimal.h"
#include <iostream>

using namespace std;

Optimal::Optimal(SystemCore &s, Internet &net, Maintenance &mnt, Extension &exten) 
    : sc(s), n(net), m(mnt), e(exten) {}

void Optimal::optimizeSystemPRO() {
    cout << "[...] Bat dau toi uu he thong toan dien...\n\n";
    cout << "[Dọn rác]\n";          m.cleanDisk();
    cout << "[Giảm delay tat app]\n"; m.reduceShutdownTime();
    cout << "[Chặn cài app ngầm]\n";  m.Consumer_Content();
    cout << "[Tắt telemetry]\n";      m.windowsTelemetry();
    cout << "[Tắt hibernate]\n";      m.Hibernate();
    cout << "[OK] Hệ thống đã được tối ưu!\n";
}

void Optimal::enableSecurityPRO() {
    cout << "[...] Đang kích hợp bảo mật...\n";
    sc.runAdmin("netsh advfirewall set allprofiles state on", true);
    sc.runAdmin("powershell -Command \"Set-MpPreference -DisableRealtimeMonitoring $false\"", true);
    sc.runAdmin("powershell -Command \"Set-MpPreference -EnableControlledFolderAccess Enabled\"", true);
    cout << "[OK] 1 số chức năng đã được bật!\n";
}

void Optimal::optimizeNetworkPRO() {
    cout << "[...] Dang toi uu ket noi mang...\n";
    cout << "[Flush DNS]\n";    n.flushdns();
    cout << "[Reset TCP/IP]\n"; n.netsh_tcpIP();
    cout << "[Tat telemetry]\n";m.windowsTelemetry();
    cout << "[OK] Mang da duoc thiet lap lai!\n";
}

string Optimal::getCurrentOS() {
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

void Optimal::upgradeWindowsEditionPRO() {
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
            sc.runAdmin("changepk.exe /ProductKey " + key, true);
        }
    }
}