#include "include/AutoActions.h"
#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <limits>

using namespace std;

AutoActions::AutoActions(SystemCore &s) : sc(s) {}

void AutoActions::autoClickPoint() {
    cout << "--- AUTO CLICK TAI VI TRI ---\n";
    int times      = sc.readInt("Số lần click: ");
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

void AutoActions::spamText() {
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

void AutoActions::autoPasteData() {
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