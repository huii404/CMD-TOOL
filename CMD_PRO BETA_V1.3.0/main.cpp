#include "include/Internet.h"
#include <windows.h>
#include <iostream>
#include "include/SystemCore.h"
#include "include/Information.h"
#include "include/Manager.h"
#include "include/Maintenance.h"
#include "include/Optimal.h"
#include "include/AutoActions.h"
#include "include/Extension.h"
#include "include/Explorer.h"

using namespace std;

class AppUI : public SystemCore {
    Internet   internet;
    Maintenance main_;
    Information info;
    Manager    manager;
    Optimal    op;
    AutoActions a;
    Extension  exten;
    Explorer exp;

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
        setColor(3); cout << getTime() << " | "<<info.getDeviceType()<<" | Github: Huii404 \n";
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