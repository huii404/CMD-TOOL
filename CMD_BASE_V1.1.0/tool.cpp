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

    void run(const std::string &cmd){
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
};

/* ================= 2. WIN FEATURES ================= */
class WinFeatures {
public:
    SystemCore core;

    /* ---------- THÔNG TIN ---------- */
    void showSoftware(){core.run("wmic product get name,version"); } 
    void systemInfo(){core.run("systeminfo");} 
    void driverList(){core.run("driverquery");} 
    void showHistory(){
        ifstream f("History.txt");
        string line;
        while(getline(f,line)) cout << line << endl;
        f.close(); 
    }

    /* ---------- QUẢN LÝ ---------- */
    void controlPanel(){ core.run("control"); } 
    void taskManager(){ core.run("taskmgr"); } 
    void computerMgmt(){ core.run("compmgmt.msc"); } 
    void services(){ core.run("services.msc"); } 
    void registry(){ core.run("regedit"); } 
    void deviceManager(){ core.run("devmgmt.msc"); } 
    void lockPC(){ core.run("rundll32.exe user32.dll,LockWorkStation"); } 

    /* ---------- MẠNG ---------- */
    void showIP(){ core.run("ipconfig /all"); } 
    void renewIP(){ core.run("ipconfig /renew"); } 
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

    /* ---------- BẢO TRÌ ---------- */
    void cleanDisk(){core.run("cleanmgr");} 
    void clearRecent(){core.run("del /f /s /q %AppData%\\Microsoft\\Windows\\Recent\\*");} 
    void scanVirus(){core.run("\"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 1");} 
    void sfcScan(){core.run("sfc /scannow");} 
    void checkDisk(){core.run("chkdsk");} 
    void restart(){core.run("shutdown /r /t 0");} 

    /* ---------- PHẦN CỨNG ---------- */
    void brightness(){
        int level;
        cout << "Nhập độ sáng (1-100): ";cin >> level;
        if(level<1 || level>100) return;
        string cmd ="powershell -Command \"(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1,"+ to_string(level) + ")\"";
        core.run(cmd); 
    }
    void turnOffMonitor(){core.run("powershell (Add-Type '[DllImport(\"user32.dll\")]^public static extern int SendMessage(int hWnd,int hMsg,int wParam,int lParam);' -name a -pas)::SendMessage(-1,0x0112,0xF170,2)");} 
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

    void intro(){
        system("cls");
        cout<<"====================================================\n";
        cout<<"DEVICE: ";cout<<core.getDeviceType();cout<<"\n";
        cout<<"VERSION: 1.1.0 BASE\n";
        cout<<"Github: Huii404\n";
        cout<<"====================================================\n"; 
    }

    void mainMenu(){
        system("title HOME");
        cout << "\n\t[1] Thông tin hệ thống       [2] Quản lý hệ thống";
        cout << "\n\t[3] Mạng                     [4] Bảo trì";
        cout << "\n\t[5] Phần cứng                [6] Auto Tọa độ & Văn bản (MỚI)";
        cout << "\n\t[0] Thoát";
        cout << "\n\n\t[Chọn]: "; 
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
                    cout << "\t[1] Phần mềm  [2] System Info\n\t[3] Driver    [4] Lịch sử\n\t[0] Back\n\tChọn: "; cin >> sub;
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
                    cout << "\t[1] Control Panel    [2] Task Manager\n\t[3] Computer Mgmt    [4] Services\n\t[5] Registry         [6] Device Manager\n\t[7] Lock             [0] Back\n\t[Chọn]: "; cin >> sub;
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
                    cout << "\t[1] Xem IP   [2] Renew IP   [3] WiFi Password\n\t[0] Back\nChọn: "; cin >> sub;
                    if(sub==0) break;
                    if(sub==1) f.showIP();
                    if(sub==2) f.renewIP();
                    if(sub==3) f.wifiPassword();
                    core.log(mainChoice,sub); core.waitEnter();
                } break; 

            case 4:
                while(true){
                    system("cls");
                    cout << "\t[1] Dọn rác      [2] Clear Recent\n\t[3] Quét Virus   [4] SFC\n\t[5] Check Disk   [6] Restart\n\t[0] Back\n\tChọn: "; cin >> sub;
                    if(sub==0) break;
                    if(sub==1) f.cleanDisk();
                    if(sub==2) f.clearRecent();
                    if(sub==3) f.scanVirus();
                    if(sub==4) f.sfcScan();
                    if(sub==5) f.checkDisk();
                    if(sub==6) f.restart();
                    core.log(mainChoice,sub); core.waitEnter();
                } break; 

            case 5:
                while(true){
                    system("cls");
                    cout << "\n\t[1] Độ sáng   [2] Tắt màn hình   [0] Back\nChọn: "; cin >> sub;
                    if(sub==0) break;
                    if(sub==1) f.brightness();
                    if(sub==2) f.turnOffMonitor();
                    core.log(mainChoice,sub); core.waitEnter();
                } break; 

            case 6: 
                while(true){
                    system("cls");
                    cout << "\t[1] Auto Click tại điểm đang trỏ\n\t[2] Spam Text (1 nội dung)\n\t[3] Auto Dán Data (Duyệt danh sách)\n\t[0] Back\n\tChọn: "; cin >> sub;
                    if(sub==0) break;
                    if(sub==1) a.autoClickPoint();
                    if(sub==2) a.spamText();
                    if(sub==3) a.autoPasteData();
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