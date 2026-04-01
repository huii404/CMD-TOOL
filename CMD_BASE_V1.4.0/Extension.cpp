#include "include/Extension.h"
#include <iostream>
#include <vector>
#include <limits>
#include <windows.h>
#include <iomanip>
using namespace std;


Extension::Extension(SystemCore &s) : sc(s) {}

bool Extension::text_processing(const string &text) {
    if (text.empty()) return true;
    if (text.length() > 99) { cout << "[!] Van ban qua dai (Max 99 ky tu).\n"; return true; }
    return false;
}

void Extension::ShowQR(string text) {
    if (text.length() > 99) { cout << "[!] Text qua dai!\n"; return; }
    for (char &c : text) if (c == ' ') c = '+';
    sc.runCMD("curl qrenco.de/" + text);
}

void Extension::ShowN_QR(int number) {
    if (number >= 15 || number <= 0) { cout << "[!] So luong khong hop le!\n"; return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    vector<string> list_qr;
    
    for (int dem = 1; dem <= number; dem++) {
        string text;
        cout << "[" << dem << "/" << number << "] Nhap noi dung QR: ";
        if (!getline(cin, text) || text.empty()) { cout << "[!] Khong duoc de trong!\n"; dem--; continue; }
        list_qr.push_back(text);
    }

    for (size_t i = 0; i < list_qr.size(); i++) {
        string current_text = list_qr[i];
        cout << "Dang lay ma QR thu " << i + 1 << "...\n"; Sleep(4000);
        for (char &c : current_text) if (c == ' ') c = '+';
        sc.runCMD("curl qrenco.de/" + current_text);
        cout << "\n--------------------------------------\n";
    }
}

void Extension::uninstallBloatware() {
    vector<string> blacklist = {
        "BingWeather", "BingNews", "SolitaireCollection", "People",
        "PowerAutomateDesktop", "Todo", "GetHelp", "Getstarted",
        "OfficeHub", "SkypeApp", "YourPhone", "FeedbackHub",
        "ZuneVideo", "ZuneMusic", "MixedReality.Portal", "Clipchamp",
        "Disney", "Spotify", "TikTok", "Instagram"
    };
    string listRac = "";
    for (size_t i = 0; i < blacklist.size(); ++i) {
        listRac += blacklist[i];
        if (i < blacklist.size() - 1) listRac += "|";
    }
    cout << "[SYSTEM] Đang tiến hành xóa app rác\n";
    string cmd1 = "powershell -Command \"Get-AppxPackage -AllUsers | Where-Object {$_.Name -match '" + listRac + "'} | Remove-AppxPackage -AllUsers\"";
    string cmd2 = "powershell -Command \"Get-AppxProvisionedPackage -Online | Where-Object {$_.DisplayName -match '" + listRac + "'} | Remove-AppxProvisionedPackage -Online\"";
    sc.runAdmin(cmd1, true);
    sc.runAdmin(cmd2, true);
    cout << "\n[SUCCESS] Hệ thống đã xóa 1 số app\n";
}


void Extension::downloadManager() {
    // Gom tất cả link vào vector AppInfo
    std::vector<AppInfo> appList = {
        {"Google Chrome", "https://dl.google.com/tag/s/appname%3DGoogle%2520Chrome/update2/installers/ChromeSetup.exe", "ChromeSetup.exe"},
        {"Coc Coc", "https://files.coccoc.com/browser/coccoc_vi.exe", "CocCocSetup.exe"},
        {"Zalo PC", "https://zalo.me/download/zalo-pc", "ZaloSetup.exe"},
        {"Discord", "https://discord.com/api/downloads/distributions/app/installers/latest?channel=stable&platform=win&arch=x86", "DiscordSetup.exe"},
        {"VS Code", "https://code.visualstudio.com/sha/download?build=stable&os=win32-x64-user", "VSCodeSetup.exe"}
    };

    sc.cls();
    std::cout << "====================================================\n";
    std::cout << "          TRINH TAI & CAI DAT APP TU DONG           \n";
    std::cout << "====================================================\n";
    
    // Hiển thị danh sách app
    for (size_t i = 0; i < appList.size(); i++) {
        std::cout << "[" << i + 1 << "] " << std::left << std::setw(15) 
                  << appList[i].name << " (" << appList[i].fileName << ")\n";
    }
    std::cout << "[A] Tai TOAN BO danh sach\n";
    std::cout << "[0] Quay lai\n";
    std::cout << "----------------------------------------------------\n";
    
    std::string input;
    std::cout << "[Chon]: "; 
    std::cin >> input;

    if (input == "0") return;

    // Xử lý tải toàn bộ
    if (input == "A" || input == "a") {
        std::cout << "\n[!] DANG TAI TOAN BO " << appList.size() << " UNG DUNG...\n";
        for (const auto& app : appList) {
            processDownload(app);
        }
    } 
    // Xử lý tải theo số thứ tự
    else {
        try {
            int idx = std::stoi(input) - 1;
            if (idx >= 0 && idx < (int)appList.size()) {
                processDownload(appList[idx]);
            } else {
                std::cout << "[!] So thu tu khong hop le!\n";
            }
        } catch (...) {
            std::cout << "[!] Lua chon khong hop le!\n";
        }
    }
    std::cout << "\n[DONE] Hoan tat tien trinh.\n";
}

// Hàm bổ trợ thực hiện việc tải và chạy file installer
void Extension::processDownload(const AppInfo &app) {
    std::string tempPath = "%temp%\\" + app.fileName;
    std::cout << "\n[+] Dang tai " << app.name << "...\n";
    
    // Sử dụng curl -L để tự động đuổi theo link redirect (quan trọng cho Zalo/VSCode)
    std::string downloadCmd = "curl -L \"" + app.url + "\" -o \"" + tempPath + "\"";
    sc.runCMD(downloadCmd);

    std::cout << "[>] Dang khoi chay: " << app.fileName << "\n";
    std::string installCmd = "start \"\" \"" + tempPath + "\"";
    sc.runCMD(installCmd);
}