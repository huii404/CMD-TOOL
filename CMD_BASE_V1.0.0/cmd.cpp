#include <iostream>
#include <windows.h>
#include <fstream>
#include <ctime>
#include <iomanip>
using namespace std;

class SYSTEM{
public:
    void getTimeV1(){
        time_t now = time(0);
        tm *t = localtime(&now);
        cout <<"["<< setfill('0') << setw(2) << t->tm_mday << "/" << setw(2) << t->tm_mon + 1 << "/" << t->tm_year + 1900 << " - " << setw(2) << t->tm_hour << ":" << setw(2) << t->tm_min << ":" << setw(2) << t->tm_sec<<"]";
    }

    string getTimeV2(){
        time_t now = time(0);
        tm *t = localtime(&now);

        stringstream ss;
        ss << "["<< setfill('0') << setw(2) << t->tm_mday << "/"<< setw(2) << t->tm_mon + 1 << "/"<< t->tm_year + 1900 << " - "<< setw(2) << t->tm_hour << ":"<< setw(2) << t->tm_min << ":"<< setw(2) << t->tm_sec<< "]";
        return ss.str();
    }

    void TextColor(int color) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color); }

    // animation loading...
    void Loading(){
        int totalWidth = 20;
        char block = 219; // Ký tự đậm █
        char light = 176; // Ký tự nhạt ░

        cout << "\n\tDang xu ly: \n\t";

        // Bước 1: Vẽ thanh nhạt trước (tạo khung)
        for (int i = 0; i < totalWidth; i++){cout << light;}

        // Bước 2: Dùng \r để quay lại đầu dòng và vẽ đè thanh đậm
        for (int i = 0; i <= totalWidth; i++){
            string back(totalWidth, '\b'); // Tạo chuỗi backspace để lùi con trỏ
            cout << "\r\t";                // Quay về đầu dòng

            // Vẽ phần đã hoàn thành
            for (int j = 0; j < i; j++)cout << block;
            // Vẽ phần còn lại nhạt
            for (int j = 0; j < totalWidth - i; j++)cout << light;

            // Tính phần trăm
            int percent = (i * 100) / totalWidth;
            cout << " " << percent << "%";

            cout.flush(); // Đẩy dữ liệu ra màn hình ngay lập tức
            Sleep(50);   // Tốc độ chạy (100ms)
        }
        cout << "\n\t[HOAN THANH!]\n";
    }

    void History(int choice){
        ofstream openfile("History.txt",ios::app);
        openfile<<getTimeV2()<<" | "<<choice<<" : ✓"<<endl;
        openfile.close();
    }
    // tạo cửa sổ chạy lệnh riêng
    // bản 1:
    int N_cmd(int n, int timeout, const string &command){
        if (n <= 0)return -1;
        for (int i = 0; i < n; i++){
            string cmd = "start cmd /c \"" + command + " & timeout /t " + to_string(timeout) + " >nul\"";
            system(cmd.c_str());  Sleep(200);
        }
        return 0;
    }
    // bản 2:
    int N_cmd_keep(int n, const string &command){
        for (int i = 0; i < n; i++){
            string cmd = "start cmd /k \"" + command + "\"";
            system(cmd.c_str());  Sleep(200);
        }
        return 0;
    }

    // thay cho lệnh system
    int run(const string &cmd){
        int ret = system(cmd.c_str());
        if (ret != 0){ cout << "\n[LỖI] Lệnh thất bại!\n"; }
        return ret;
    }
};

class LENH_CMD{
public: 
    SYSTEM p;

    void software(int choice){
        switch (choice){
        case 1:system("cls");p.N_cmd_keep(1,"wmic product get name,version");break;
        case 2:system("cls");p.N_cmd_keep(1,"tasklist");system("pause");break;
        case 3:p.run("control");break;
        }
    }

    void hardware(int choice){int number;
        switch (choice){
        case 5:p.N_cmd(1,7,"systeminfo");break;
        case 6:p.N_cmd(1,7,"driverquery");break;
        case 7:{int number;
            do{system("cls");
                cout << "\nĐỘ SÁNG (1 --> 100) | Thoát = 0 : ";cin >> number;
                cout << "\n";

                if (number == 0)break;

                if (number < 1 || number > 100){
                    cout << "Sai giá trị! Nhập từ 1 đến 100.\n\n";
                    Sleep(1000); continue;
                }
                string cmd = "powershell -Command \"(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1," + to_string(number) + ")\"";
                system(cmd.c_str());
            } while (true);
            break;
        }
        case 8:p.run("powershell (Add-Type '[DllImport(\"user32.dll\")]^public static extern int SendMessage(int hWnd, int hMsg, int wParam, int lParam);' -name a -pas)::SendMessage(-1, 0x0112, 0xF170, 2)");break;
        }
    }

    void networking(int choice){
        switch (choice){
        case 9:p.N_cmd_keep(1,"ipconfig/all");break;
        case 10:p.N_cmd_keep(1,"ipconfig/renew");break;
        case 11:{system("cls");
            string wifi;
            cout << "\nNhập tên WiFi muốn xem mật khẩu: ";cin >> ws;
            getline(cin, wifi);
            string cmd = "netsh wlan show profiles name=\"" + wifi + "\" key=clear";
            system(cmd.c_str());

            string read_data = "netsh wlan show profiles name=\"" + wifi + "\" key=clear | findstr \"Key Content\" > pass.txt";
            system(read_data.c_str());

            system("cls");
            ifstream openfile("pass.txt");
            string line;
            if (!openfile.is_open())cout << "khong mo duoc!\n";

            while (getline(openfile, line)){cout << line << endl;}
            openfile.close();
            system("pause");
        }break;
        }
    }

    void systemMaintenance(int choice){
        switch (choice){
        case 12:{
            cout<<"\n\t\tLoading\n";
            cout<<"XÓA RÁC Ổ CỨNG\n";
            p.run("cleanmgr /sagerun:1");
            Sleep(1000);
            p.run("rd /s /q \"%TEMP%\" && md \"%TEMP%\"");
            Sleep(1000);
            cout<<"XÓA RÁC TỰ ĐỘNG (Storage Sense)\n";
            p.run(
                    "reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\StorageSense\\Parameters\\StoragePolicy\" "
                    "/v 01 /t REG_DWORD /d 1 /f");
            p.run("echo Storage Sense da duoc bat!");
            cout<<"successfull\n";Sleep(1000);
        }break;
        case 13:p.run("del /f /s /q %AppData%\\Microsoft\\Windows\\Recent\\*");break;
        case 14:p.run("shutdown /r /t 0");break;
        case 15:{
            cout << "\n1. QUÉT NHANH VIRUS | 2. QUÉT TOÀN BỘ FILE\n";
            cout << "--> ";cin >> choice;
            switch (choice){
            case 1:p.N_cmd_keep(1,"cmd /c \"\"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 1\"");break;
            case 2:p.N_cmd_keep(1,"cmd /c \"\"%ProgramFiles%\\Windows Defender\\MpCmdRun.exe\" -Scan -ScanType 2\"");break;
            }
        }break;
        case 16:{
            cout << "\n1. QUÉT-SỬA LỖI FILE | 2. QUÉT Ổ CỨNG\n";
            cout << "--> ";cin >> choice;
            switch (choice){
            case 1:{
                string command = "sfc/scannow";
                string powershell_cmd = "powershell -Command \"Start-Process cmd -Verb RunAs -ArgumentList '/k " + command + "'\"";
                system(powershell_cmd.c_str());
            }break;
            case 2:{
                string command = "chkdsk";
                string powershell_cmd = "powershell -Command \"Start-Process cmd -Verb RunAs -ArgumentList '/k " + command + "'\"";
                system(powershell_cmd.c_str());
            }break;
            }
        }
        break;
        }
    }

    void systemAdmin(int choice){
        switch (choice){
        case 17:p.run("taskmgr");break;
        case 18:p.run("regedit");break;
        case 19:p.run("services.msc");break;
        case 20:p.run("devmgmt.msc");break;
        case 21:p.run("compmgmt.msc");break;
        case 22:p.run("rundll32.exe user32.dll,LockWorkStation");break;
        case 23:{
            cout << "\n1. TURN ON | 2. TURN OFF\n";
            cout << "---> ";cin >> choice;
            switch (choice){
            case 1:{
                p.run("sc config wuauserv start= demand && net start wuauserv");
                p.run("sc config WaaSMedicSvc start= demand && net start WaaSMedicSvc");
                cout << "\n Windows Update đã được BẬT lại.\n";
                system("pause");
                break;
            }
            case 2:{
                p.run("net stop wuauserv && sc config wuauserv start= disabled");
                p.run("net stop WaaSMedicSvc && sc config WaaSMedicSvc start= disabled");
                cout << "\n Windows Update đã được TẮT hoàn toàn.\n";
                system("pause");
                break;
            }
            }
        }
        break;
        case 24:{
            ifstream openfile("History.txt");
            string line;
            while(getline(openfile,line)){cout<<line<<endl;}
            openfile.close();
            system("pause");
        }break;
        }
    }

};

class Gui{
public:
    LENH_CMD a;  SYSTEM b;
    void menu(int &cmd){system("title APP CMD BETA -- BY: HUYNGUYEN");
        cout << "\n-------software-------\n";
        cout << "1. phần mềm đã tải    \n";
        cout << "2. quản lý tiến trình \n";
        cout << "3. control panel    \n";
        cout << "------hardware-------\n";
        cout << "5. thông tin phần cứng\n";
        cout << "6. danh sách drive\n";
        cout << "7. độ sáng màn hình\n";
        cout << "8. tắt màn hình\n";
        cout << "-----networking------\n";
        cout << "9. ip-mạng       \n";
        cout << "10. cập nhật ip  \n";
        cout << "11. wifi         \n";
        cout << "----systemMaintenance---\n";
        cout << "12. xóa rác \n";
        cout << "13. xóa rencent file explorer \n";
        cout << "14. khởi động máy \n";
        cout << "15. quét virus\n";
        cout << "16. quét & sửa ổ cứng\n";
        cout << "-----systemAdmin-----\n";
        cout << "17. task manager     \n";
        cout << "18. registry        \n";
        cout << "19. quản lý dịch vụ  \n";
        cout << "20. quản lý thiết bị \n";
        cout << "21. computer manager\n";
        cout << "22. khóa máy tạm" << endl;
        cout << "23. TẮT/BẬT(WINDOW UPDATE)\n";
        cout<<"------------------------\n";
        cout << "24. xem lịch sử app\n";
        cout << "0.EXIT\n\n";
        cout << "[___]:  ";cin >> cmd;
    }
    
    void command(int cmd){
        if (cmd == 0)exit(0);
        else if (cmd >= 1 && cmd <= 4)a.software(cmd);
        else if (cmd >= 5 && cmd <= 8)a.hardware(cmd);
        else if (cmd >= 9 && cmd <= 11)a.networking(cmd);
        else if (cmd >= 12 && cmd <= 16)a.systemMaintenance(cmd);
        else if (cmd >= 17 && cmd <= 24)a.systemAdmin(cmd);
    }

    void intro(){
        system("cls");
        cout<<"====================================================\n";
        cout<<"VERSION: 1.1.0\n";
        cout<<"Github: Huii404\n";
        cout<<"====================================================\n"; 
    }
     
};

int main(){
    int cmd,choice;
    Gui p;   
    SYSTEM a;
    SetConsoleOutputCP(CP_UTF8); SetConsoleCP(CP_UTF8);
    while (true){
        p.intro();
        p.menu(cmd);
        a.History(cmd);
        p.command(cmd);
    }
    return 0;
}