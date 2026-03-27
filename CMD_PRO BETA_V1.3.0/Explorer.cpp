#include "include/Explorer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <limits>

using namespace std;
namespace fs = std::filesystem;

Explorer::Explorer(SystemCore &s) : sc(s) {}

char Explorer::strongByteProcess(char byte, string key, int position, bool isEncrypt) {
    if (key.empty()) return byte;
    char k = key[position % key.length()];
    if (isEncrypt) return (byte ^ k) + (position % 7);
    else           return (byte - (position % 7)) ^ k;
}

void Explorer::processFileBinary(bool isEncrypt) {
    string fileName, key;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "\n--- " << (isEncrypt ? "MÃ HÓA" : "GIẢI MÃ") << " FILE BINARY ---\n";
    cout << "Tên file: "; getline(cin, fileName);
    cout << "Mật khẩu: "; getline(cin, key);

    ifstream inFile(fileName, ios::binary);
    if (!inFile.is_open()) { cout << "[!] Lỗi: Không mở được file!\n"; return; }

    string tempFile = "secure_binary.tmp";
    ofstream outFile(tempFile, ios::binary);
    char byte;
    int pos = 0;

    cout << "[System] Đang xử lý chính xác từng byte...\n";
    while (inFile.get(byte)) {
        outFile.put(strongByteProcess(byte, key, pos, isEncrypt));
        pos++;
        if (pos % 1000 == 0) cout << "-> Đã xử lý: " << pos << " bytes\r";
    }
    inFile.close(); outFile.close();
    remove(fileName.c_str());
    rename(tempFile.c_str(), fileName.c_str());
    cout << "\n[OK] Đã " << (isEncrypt ? "mã hóa" : "giải mã") << " xong " << pos << " bytes dữ liệu.\n";
}

void Explorer::clearBrowserCache() {
    char *localAppData = std::getenv("LOCALAPPDATA");
    char *appData = std::getenv("APPDATA"); 
    if (!localAppData) return;

    string baseLocal = string(localAppData);
    string baseRoaming = appData ? string(appData) : "";
    vector<string> cachePaths = {
        baseLocal + "\\Google\\Chrome\\User Data\\Default\\Cache",
        baseLocal + "\\Microsoft\\Edge\\User Data\\Default\\Cache",
        baseLocal + "\\CocCoc\\Browser\\User Data\\Default\\Cache",
        baseLocal + "\\Opera Software\\Opera Stable\\Cache"
        // (Thêm lại các đường dẫn phụ tùy ý)
    };

    cout << "====================================================\n";
    cout << "[SYSTEM] DANG DON DEP CACHE MULTI-BROWSER...\n";
    for (const string &path : cachePaths) {
        if (!path.empty() && fs::exists(path)) {
            cout << "[...] Cleaning: " << path;
            try {
                for (const auto &entry : fs::directory_iterator(path)) fs::remove_all(entry.path());
                cout << " [OK]\n";
            } catch (const fs::filesystem_error &) { cout << " [!] Busy\n"; }
        }
    }
    cout << "\n[SUCCESS] Hoan tat dọn dep Cache!\n";
}

void Explorer::nguytrangFolder() {
    string path, name1, nameZip, name3;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Vị trí (vd: D:\\Data): "; getline(cin, path);
    cout << "Ảnh nền (abc.jpg): "; getline(cin, name1);
    cout << "File zip (abc.zip): "; getline(cin, nameZip);
    cout << "Tên đầu ra (abc.png): "; getline(cin, name3);

    string cmd = "cd /d \"" + path + "\" && copy /b \"" + name1 + "\" + \"" + nameZip + "\" \"" + name3 + "\"";
    sc.runCMD(cmd);
    cout << "\n[OK] Đã tạo file ngụy trang tại: " << path << "\\" << name3 << endl;
}