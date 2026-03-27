#include "include/Extension.h"
#include <iostream>
#include <vector>
#include <limits>
#include <windows.h>

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