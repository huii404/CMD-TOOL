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
    void setColor(int color){ SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color); } 

    string getTime(){
        time_t now = time(0);
        tm* t = localtime(&now);
        stringstream ss;
        ss << "[" << setfill('0')<< setw(2) << t->tm_mday << "/"<< setw(2) << t->tm_mon+1 << "/"<< t->tm_year+1900 << "-"<< setw(2) << t->tm_hour << ":"<< setw(2) << t->tm_min << ":"<< setw(2) << t->tm_sec << "]";
        return ss.str(); 
    }

    void log(int mainChoice, int subChoice){
        ofstream f("History.txt", ios::app);
        if (f.is_open()){
            f << getTime()<< ": ["<< mainChoice << "]-[" << subChoice<<"]"<< endl;
            f.close();
        } 
    }

    void runCMD(const std::string &cmd){
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        char commandLine[1024];
        strcpy(commandLine, cmd.c_str());
        if (CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)){
            WaitForSingleObject(pi.hProcess, INFINITE); 
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } 
    }

    string getDeviceType(){
        SYSTEM_POWER_STATUS sps;
        if (GetSystemPowerStatus(&sps)){
            // Không có pin
            if (sps.BatteryFlag == 128)    return "Desktop";
            // Có pin
            if (sps.BatteryLifePercent != 255){ // 255 = không xác định
                stringstream ss;
                ss << "Laptop: " << (int)sps.BatteryLifePercent << "%";
                return ss.str();
            }
            else    return "Laptop - Battery Unknown";
        }
        return "Unknown";
    }

    void waitEnter(){
        cout << "\nNHAN ENTER DE QUAY LAI";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get(); 
    }

    void leftClick() {
        INPUT input[2] = {};
        input[0].type = INPUT_MOUSE; input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        input[1].type = INPUT_MOUSE; input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(2, input, sizeof(INPUT)); 
    }

    void setClipboard(const string& text) {
        if (!OpenClipboard(nullptr)) return;
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (hMem) {
            memcpy(GlobalLock(hMem), text.c_str(), text.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
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

    // warning
    void warning_cmdAdmin(string message){  string answer;
        cout << "CHẠY QUYỀN ADMIN CHO LỆNH [" << message << "] (Y/N): ";cin >> answer;

        if (answer == "y" || answer == "Y"){
            // Chuyển string sang wstring để dùng với ShellExecuteExW
            wstring wMsg(message.begin(), message.end());
            SHELLEXECUTEINFOW sei = {sizeof(sei)};
            sei.lpVerb = L"runas"; // Chìa khóa nâng quyền
            sei.lpFile = L"cmd.exe";
            // Dùng /k để giữ cửa sổ lại
            wstring params = L"/k " + wMsg;
            sei.lpParameters = params.c_str();
            sei.nShow = SW_SHOWNORMAL;
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;

            if (ShellExecuteExW(&sei)){
                cout << "[OK] ĐANG CHẠY LỆNH\n";
                WaitForSingleObject(sei.hProcess, INFINITE);
                CloseHandle(sei.hProcess);
            }else{
                cout << "[!] KHÔNG THỂ LẤY QUYỀN ADMIN.\n";
            }
        }else{
            cout << "BỎ QUA LỆNH.\n";
        }
    }

    // ================= ZIP FUNCTIONS =================

    void ZIP(string source, string destination, int level){
        // Chặn cứng giá trị min/max để bảo vệ chương trình
        if (level < 0) level = 0;
        if (level > 9) level = 9;

        string compressionLevel = "-mx" + to_string(level);
        // Thêm hậu tố nén đa luồng để tận dụng tối đa CPU (giảm rủi ro treo do chờ đợi lâu)
        string threading = " -mmt=on";
        if (destination.find(".zip") == string::npos)
            destination += ".zip";
        // Ghép chuỗi hoàn chỉnh với dấu nháy kép bảo vệ đường dẫn
        string cmd = "7za.exe a -tzip \"" + destination + "\" \"" + source + "\" " + compressionLevel + threading;
        cout << "\n[System] Dang khoi tao tien trinh nen...\n";
        cout << "[i] Che do: " << (level == 0 ? "Copy (Store)" : (level <= 3 ? "Nhanh" : (level <= 6 ? "Tieu chuan" : "Nen sau"))) << "\n";
        int result = system(cmd.c_str());
        if (result == 0)  cout << "[OK] Nen hoan tat!\n";
        else              cout << "[!] Loi thuc thi (Ma loi: " << result << ")\n";
    }

    void UNZIP(string zipFile, string extractPath){
        string cmd = "7za.exe x \"" + zipFile + "\" -o\"" + extractPath + "\" -y";
        cout << "\n[System] Dang giai nen file: " << zipFile << "...\n";
        int result = system(cmd.c_str());

        if (result == 0){cout << "[OK] Da giai nen xong vao: " << extractPath << "\n";
        }else{cout << "[X] Giai nen that bai! Ma loi: " << result << "\n";}
    }
};

/* ================= 2. WIN FEATURES ================= */
class WinFeatures {
public:
    SystemCore core;

    // thông tin
    void showSoftware(){core.runCMD("wmic product get name,version"); } 
    void systemInfo(){core.runCMD("systeminfo");} 
    void driverList(){core.runCMD("driverquery");} 
    void showHistory(){
        ifstream f("History.txt");
        string line;
        while(getline(f,line)) cout << line << endl;
        f.close(); 
    }

    // quản lý
    void controlPanel(){ core.runCMD("control"); } 
    void taskManager(){ core.runCMD("taskmgr"); } 
    void computerMgmt(){ core.runCMD("compmgmt.msc"); } 
    void services(){ core.runCMD("services.msc"); } 
    void registry(){ core.runCMD("regedit"); } 
    void deviceManager(){ core.runCMD("devmgmt.msc"); } 
    void lockPC(){ core.runCMD("rundll32.exe user32.dll,LockWorkStation"); } 

    // mạng
    void showIP(){ core.runCMD("ipconfig /all"); } 
    void netsh_tcpIP(){core.warning_cmdAdmin("netsh int ip reset");}
    void renewIP(){ core.runCMD("ipconfig /renew"); } 
    void wifiPassword(){
        system("netsh wlan show profiles > listwifi.txt");
        ifstream in("listwifi.txt");
        string line;
        while (getline(in, line)){
            if (line.find("All User Profile") != string::npos){
                int pos = line.find(":");
                string wifiName = line.substr(pos + 2);
                string command = "netsh wlan show profile \"" + wifiName + "\" key=clear > return.txt";
                system(command.c_str());

                ifstream temp("return.txt");
                string infoLine,name = "",pass="";
                while (getline(temp, infoLine)){
                    if (infoLine.find("Name") != string::npos){
                        int p = infoLine.find(":"); name = infoLine.substr(p + 2);
                    }
                    if (infoLine.find("Key Content") != string::npos){
                        int p = infoLine.find(":"); pass = infoLine.substr(p + 2);
                    }
                }
                temp.close();
                if (name != ""){
                    cout << "WiFi: " << name <<" | ";
                    if (pass != "") cout << "Password: " << pass << endl;
                    else            cout << "Password: (No password / Open network)" << endl;
                    cout << "----------------------------------------------\n";
                }
            }
        }
        in.close(); 
    }

    void flushdns(){core.runCMD("ipconfig/displaydns");}

    //bảo trì
    void cleanDisk(){core.runCMD("cleanmgr");} 
    void clearRecent(){core.runCMD("del /f /s /q %AppData%\\Microsoft\\Windows\\Recent\\*");} 
    void QuickScanVirus(){core.runCMD("\"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 1");} 
    void FullScanVirus(){core.runCMD("\"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 2");} 
    void restart(){core.runCMD("shutdown /r /t 0");} 
    // phần cứng
    void brightness(){ int level;
        cout << "Nhập độ sáng (1-100): ";cin >> level;
        if(level<1 || level>100) return;
        string cmd ="powershell -Command \"(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1,"+ to_string(level) + ")\"";
        core.runCMD(cmd); 
    }
    
    void turnOffMonitor(){core.runCMD("powershell (Add-Type '[DllImport(\"user32.dll\")]^public static extern int SendMessage(int hWnd,int hMsg,int wParam,int lParam);' -name a -pas)::SendMessage(-1,0x0112,0xF170,2)");} 

    // version app
    void reviewApp(){int answer;
        core.runCMD("cls");
        cout<<"\n\n1.version 1.0.0 BASE\n";
        cout<<"2.version 1.1.0 BASE\n";
        cout<<"3.version 1.2.0 BASE(comming soon)\n";
        cout<<"4.version 1.2.0 PRO(installed)\n";
        cout<<"0.Back\n";
        cout<<"[CHON]: ";cin>>answer;
        if(answer==0) return ;
        if(answer==1){system("start https://github.com/huii404/sourcecode/tree/main/c%2B%2B/cmd_box/CMD_V1.0.0");}
        if(answer==2){system("start https://github.com/huii404/sourcecode/tree/main/c%2B%2B/cmd_box/CMD_V1.1.0");}
        if(answer==4){cout<<"\nVERSION 1.2.0 PRIVATE NOT PUBLIC\n";}
    }

};

/* ================= 3. AUTO ACTIONS ================= */

class AutoActions {
public:
    SystemCore core;

    void autoClickPoint() {
        int times, delaySec, intervalMs;
        cout << "--- AUTO CLICK TAI VI TRI ---\n";
        cout << "số lần click: "; cin >> times;
        cout << "Delay các lần (ms): "; cin >> intervalMs;
        cout << "thời gian di chuyển chuột (giay): "; cin >> delaySec;

        cout << "\n Đưa chuột đến vị trí cần click..\n";
        for (int i = delaySec; i > 0; i--) { cout << i << "... "; Sleep(1000); }

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
        int times, delayMs; string content;
        cout << "\nText: ";
        cin.ignore(); getline(cin, content);
        cout << "SỐ LẦN: "; cin >> times;
        cout << "Delay (ms): "; cin >> delayMs;

        cout << "\nClick vào ô nhập... ("<<delayMs<<")\n"; Sleep(3000);

        for (int i = 0; i < times; i++) {
            core.setClipboard(content);
            core.pressCtrlV();
            core.pressEnter();
            Sleep(delayMs);
        } 
    }

    void autoPasteData() {
        int delayMs, n;
        cout << "\n SỐ DÒNG: "; cin >> n;
        cout << "Delay (ms): "; cin >> delayMs; cin.ignore();

        vector<string> dataList(n);
        for (int i = 0; i < n; i++) {
            cout << "Dong [" << i + 1 << "]: ";
            getline(cin, dataList[i]);
        }

        cout << "\nClick  vào ô nhập... ("<<delayMs<<")\n"; Sleep(3000);

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
    WinFeatures f;   
    AutoActions a;
    SystemCore core;
    string displayLicense;

    void intro(){
        system("cls");system("title Toolkit by huii404");
        cout<<"====================================================\n";
        core.setColor(4);
        cout<<"DEVICE: ";cout<<core.getDeviceType();cout<<"\n";
        core.setColor(5);
        cout<<"VERSION: 1.2.0 BASE | Github: Huii404\n";
        core.setColor(6);
        cout<<"LICENSE: " << displayLicense << "\n";
        core.setColor(7);
        cout<<"====================================================\n"; 
    }

    void mainMenu(){
        cout << "\n[1] Thông tin hệ thống       [2] Quản lý hệ thống";
        cout << "\n[3] Mạng                     [4] Bảo trì";
        cout << "\n[5] Phần cứng                [6] Tiện ích";
        cout << "\n[0] Thoát";
        cout << "\n\n[Chọn]: "; 
    }

    void run(){
        int mainChoice, sub;

        while(true){
            intro(); mainMenu();
            if (!(cin >> mainChoice)) {
                cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            if(mainChoice==0) break;
            system("cls");

            switch(mainChoice){
            case 1:
                while(true){
                    system("cls");
                    cout << "[1] Phần mềm  [2] System Info\n[3] Driver    [4] Lịch sử\n[0] Back\nChọn: "; cin >> sub;
                    if(sub==0) break;
                    if(sub==1) f.showSoftware();
                    if(sub==2) f.systemInfo();
                    if(sub==3) f.driverList();
                    if(sub==4) f.showHistory();
                    core.log(mainChoice,sub); core.waitEnter();
                } break; 
            
            case 2:
                while(true){
                    system("cls");
                    cout << "[1] Control Panel    [2] Task Manager\n[3] Computer Mgmt    [4] Services\n[5] Registry         [6] Device Manager\n[7] Lock             [0] Back\n[Chọn]: "; cin >> sub;
                    if(sub==0) break;
                    if(sub==1) f.controlPanel();
                    if(sub==2) f.taskManager();
                    if(sub==3) f.computerMgmt();
                    if(sub==4) f.services();
                    if(sub==5) f.registry();
                    if(sub==6) f.deviceManager();
                    if(sub==7) f.lockPC();
                    core.log(mainChoice,sub); core.waitEnter();
                } break; 

            case 3:
                while(true){
                    system("cls");
                    cout << "[1] Xem IP     [2] Renew IP   [3] WiFi Password\n[4] flushdns   [5]TCP/IP     [0] Back\n";
                    cout<<"[Chọn]: ";cin >> sub;
                    if(sub==0) break;
                    if(sub==1) f.showIP();
                    if(sub==2) f.renewIP();
                    if(sub==3) f.wifiPassword();
                    if(sub==4) f.flushdns();
                    if(sub==5)f.netsh_tcpIP();
                    core.log(mainChoice,sub); core.waitEnter();
                } break; 

            case 4:
                while(true){
                    system("cls");
                    cout << "[1] Dọn rác      [2] Clear Recent\n[3] Quét Virus   [4] SFC\n[5] Check Disk   [6] Restart\n[0] Back\nChọn: "; cin >> sub;
                    if(sub==0) break;
                    if(sub==1) f.cleanDisk();
                    if(sub==2) f.clearRecent();
                    if(sub==3){
                        int answer;
                        cout<<"(\n[1]QuickScan--[2]FullScan): ";cin>>answer;
                        if(answer==1)f.QuickScanVirus();
                        else f.FullScanVirus();
                    }
                    if(sub==4) core.warning_cmdAdmin("sfc/cannow");
                    if(sub==5) core.warning_cmdAdmin("chkdsk");
                    if(sub==6) f.restart();
                    core.log(mainChoice,sub); core.waitEnter();
                } break; 

            case 5:
                while(true){
                    system("cls");
                    cout << "\n[1] Độ sáng   [2] Tắt màn hình   [0] Back\nChọn: "; cin >> sub;
                    if(sub==0) break;
                    if(sub==1) f.brightness();
                    if(sub==2) f.turnOffMonitor();
                    core.log(mainChoice,sub); core.waitEnter();
                } break; 

            case 6: 
                while(true){
                    system("cls"); 
                    cout << "[1] Auto Click\n[2] Spam Text\n[3] Auto Dán Data (Duyệt danh sách)\n[4] Nén file/folder\n[5] Giải nén file/folder\n[6] Version app\n[0] Back\nChọn: "; cin >> sub;
                    if(sub==0) break;
                    if(sub==1) a.autoClickPoint();
                    if(sub==2) a.spamText();
                    if(sub==3) a.autoPasteData();
                    if (sub == 4){
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        string path, path2;
                        int level;
                        // Nhập Level với logic kiểm tra
                        while (true){
                            cout << "Level ZIP (0: Copy, 1: Nhanh nhat, 9: Nen cao nhat): ";
                            if (!(cin >> level)){
                                cout << "[!] Vui long nhap so!\n";
                                cin.clear();
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                continue;
                            }

                            // Cảnh báo mức độ nguy hiểm
                            if (level >= 7){
                                char confirm;
                                cout << "--------------------------------------------------\n";
                                cout << "[CANH BAO] Ban dang chon muc nen rat cao (" << level << ")\n";
                                cout << " - Thoi gian nen se RAT LAU voi file lon.\n";
                                cout << " - RAM va CPU se bi chiem dung toi da.\n";
                                cout << "Ban co muon tiep tuc khong? (y/n): ";cin >> confirm;
                                if (confirm != 'y' && confirm != 'Y'){
                                    cout << "-> Vui long chon lai level thap hon.\n";
                                    continue;
                                }
                            }
                            break; // Thoát vòng lặp khi level hợp lệ và đã xác nhận
                        }

                        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Xóa bộ nhớ đệm sau khi nhập số
                        cout << "Address file: ";
                        getline(cin, path);
                        cout << "Address zip: ";
                        getline(cin, path2);

                        core.ZIP(path, path2, level);
                    }
                    if (sub == 5){ 
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        string zipPath, destPath;
                        cout << "Duong dan file .zip: ";getline(cin, zipPath);
                        cout << "Thu muc giai nen den: ";getline(cin, destPath);
                        core.UNZIP(zipPath, destPath);
                    }
                    if(sub==6) f.reviewApp();
                    core.log(mainChoice,sub); core.waitEnter();
                } break;
            }
        } 
    }

};

int main(){
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    AppUI app;
    app.run();

    return 0; 
}