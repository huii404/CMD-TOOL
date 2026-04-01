#include "include/Internet.h"
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <thread>
#include <algorithm>
#include <cstdio>
#include <ctime>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

Internet::Internet(SystemCore &s) : sc(s), listenSocket(INVALID_SOCKET), httpPort(8080), shareSize(0), dlCount(0) {}

Internet::~Internet() {
    if (listenSocket != INVALID_SOCKET)
        closesocket(listenSocket);
}

string Internet::getField(const string &line) {
    size_t pos = line.find(":");
    if (pos != string::npos && pos + 2 < line.size()) return sc.trim(line.substr(pos + 2));
    return "";
}

string Internet::getContentType(const string &fpath) {
    string ext = sc.trim(fpath.substr(fpath.find_last_of(".") != string::npos ? fpath.find_last_of(".") : fpath.length()));
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    map<string, string> types = {{".pdf", "application/pdf"}, {".txt", "text/plain"}, {".html", "text/html"}, {".zip", "application/zip"}, {".mp4", "video/mp4"}, {".mp3", "audio/mpeg"}, {".jpg", "image/jpeg"}, {".png", "image/png"}, {".exe", "application/octet-stream"}};
    return types.count(ext) ? types[ext] : "application/octet-stream";
}

string Internet::getLocalIP() {
    FILE *pipe = _popen("powershell -NoProfile -Command \"(Get-NetIPAddress -AddressFamily IPv4 | Where-Object {$_.IPAddress -like '192.168.*' -or $_.IPAddress -like '10.*'} | Select-Object -First 1).IPAddress\"","r");
    if (pipe) {
        char buf[32] = {0};
        if (fgets(buf, sizeof(buf), pipe)) {
            string ip = sc.trim(string(buf));
            _pclose(pipe);
            if (!ip.empty() && ip != "0.0.0.0") return ip;
        }
        _pclose(pipe);
    }
    return "127.0.0.1";
}

void Internet::openFW() {
    char cmd[256];
    sprintf_s(cmd, sizeof(cmd), "netsh advfirewall firewall add rule name=\"QuickShare_%d\" dir=in action=allow protocol=tcp localport=%d >nul 2>&1", httpPort, httpPort);
    system(cmd);
}

string Internet::formatSize(long long b) {
    const char *u[] = {"B", "KB", "MB", "GB"};
    double sz = (double)b;
    int i = 0;
    while (sz >= 1024 && i < 3) {
        sz /= 1024;
        i++;
    }
    char buf[32];
    sprintf_s(buf, sizeof(buf), "%.2f %s", sz, u[i]);
    return string(buf);
}

string Internet::getTime() {
    time_t now = time(0);
    tm *t = localtime(&now);
    char buf[100];
    strftime(buf, sizeof(buf), "[%H:%M:%S]", t);
    return string(buf);
}

HTTPRequest Internet::parseReq(const string &raw) {
    HTTPRequest req;
    istringstream iss(raw);
    string line;
    if (getline(iss, line)) {
        istringstream ls(line);
        ls >> req.method >> req.path >> req.httpVersion;
    }
    while (getline(iss, line)) {
        line = sc.trim(line);
        if (line.empty()) break;
        size_t p = line.find(":");
        if (p != string::npos) {
            req.headers[sc.trim(line.substr(0, p))] = sc.trim(line.substr(p + 1));
        }
    }
    return req;
}

void Internet::sendFile(SOCKET client) {
    ifstream f(sharePath, ios::binary);
    if (!f.good()) {
        string err = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 13\r\nConnection: close\r\n\r\nFile not found";
        send(client, err.c_str(), (int)err.length(), 0);
        return;
    }

    string ct = getContentType(sharePath);
    string hdr = "HTTP/1.1 200 OK\r\nContent-Type: " + ct + "\r\nContent-Length: " + to_string(shareSize) +"\r\nContent-Disposition: attachment; filename=\"" + shareName + "\"\r\nConnection: close\r\n\r\n";
    send(client, hdr.c_str(), (int)hdr.length(), 0);

    const int CHUNK = 1024 * 64;
    char buf[CHUNK];
    long long sent = 0;
    int prog = 0;
    cout << getTime() << " | Gửi: ";

    while (f.read(buf, CHUNK) || f.gcount() > 0) {
        int toSend = (int)f.gcount();
        if (send(client, buf, toSend, 0) <= 0) {
            cout << "\n⚠️  Gián đoạn\n";
            f.close();
            return;
        }
        sent += toSend;
        int np = (int)((sent * 100) / shareSize);
        if (np > prog && np % 10 == 0) {
            cout << np << "% ";
            cout.flush();
        }
        prog = np;
    }
    f.close();
    dlCount++;
    cout << "100% ✓ [" << dlCount << "]\n";
}

void Internet::handleClient(SOCKET client) {
    const int BUFSIZE = 4096;
    char buf[BUFSIZE];
    int rcv = recv(client, buf, BUFSIZE - 1, 0);

    if (rcv > 0) {
        buf[rcv] = '\0';
        HTTPRequest req = parseReq(buf);
        cout << getTime() << " | " << req.method << " " << req.path << "\n";

        if (req.method == "GET" && sharePath[0]) {
            sendFile(client);
        }
    }
    closesocket(client);
}

bool Internet::checkFileSizeAndConfirm(const string &path, long long &outSize) {
    ifstream testFile(path, ios::binary);
    if (!testFile.good()) {
        cout << "[!] File không tồn tại!\n";
        return false;
    }

    testFile.seekg(0, ios::end);
    long long fileSize = testFile.tellg();
    testFile.close();

    if (fileSize == 0) {
        cout << "[!] File trống!\n";
        return false;
    }

    outSize = fileSize;

    if (fileSize > MAX_FILE_SIZE) {
        cout << "\n" << string(70, '=') << "\n";
        cout << "[⚠️  CẢNH BÁO] File vượt quá giới hạn an toàn!\n";
        cout << "Kích thước file: " << formatSize(fileSize) << "\n";
        cout << "Giới hạn tối đa:  " << formatSize(MAX_FILE_SIZE) << "\n";
        cout << string(70, '=') << "\n";

        string confirm;
        cout << "\n[?] Bạn vẫn muốn tiếp tục? (Y/N): ";
        cin >> confirm;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (confirm != "y" && confirm != "Y") {
            cout << "[i] Hủy bỏ quá trình chia sẻ.\n";
            return false;
        }
    }

    return true;
}

bool Internet::getFileSizeInfoAndPrompt(const string &path, long long &outSize) {
    if (!checkFileSizeAndConfirm(path, outSize)) {
        return false;
    }

    cout << "\n[✓] Thông tin file:\n";
    cout << "    - Đường dẫn: " << path << "\n";
    cout << "    - Dung lượng: " << formatSize(outSize) << "\n";

    string proceedChoice;
    cout << "\n[?] Tiếp tục với quá trình chia sẻ? (Y/N): ";
    cin >> proceedChoice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (proceedChoice != "y" && proceedChoice != "Y") {
        cout << "[i] Người dùng đã hủy bỏ.\n";
        return false;
    }

    return true;
}

void Internet::showIP() { sc.runCMD("ipconfig/all"); }
void Internet::renewIP() { sc.runCMD("ipconfig /renew"); }
void Internet::flushdns() { sc.runCMD("ipconfig /flushdns"); }
void Internet::netsh_tcpIP() { sc.runAdmin("netsh int ip reset"); }

void Internet::wifiAudit() {
    FILE *pipe = _popen("netsh wlan show profiles", "r");
    if (!pipe) {
        cout << "[!] Không chạy lệnh WiFi.\n";
        return;
    }
    char buf[512];
    int idx = 1;
    while (fgets(buf, sizeof(buf), pipe)) {
        string line = buf;
        if (line.find("All User Profile") != string::npos) {
            string wifi = getField(line);
            cout << "\n[" << idx++ << "] WiFi: " << wifi << "\n";
            string cmd = "netsh wlan show profile \"" + wifi + "\" key=clear";
            FILE *p2 = _popen(cmd.c_str(), "r");
            if (!p2) continue;
            char b2[512];
            string auth = "", cipher = "", pass = "(Open)";
            while (fgets(b2, sizeof(b2), p2)) {
                string inf = b2;
                if (inf.find("Authentication") != string::npos) auth = getField(inf);
                if (inf.find("Cipher") != string::npos) cipher = getField(inf);
                if (inf.find("Key Content") != string::npos) pass = getField(inf);
            }
            _pclose(p2);
            cout << "Pass: " << pass << " | Auth: " << auth << " | Cipher: " << cipher << "\n" << string(50, '-');
        }
    }
    _pclose(pipe);
}

void Internet::quickSharePRO() {
    sc.cls();
    string path;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "File (drag-drop or path): ";
    getline(cin, path);

    if (path.length() >= 2 && path[0] == '"' && path[path.length() - 1] == '"') {
        path = path.substr(1, path.length() - 2);
    }
    path = sc.trim(path);

    if (!getFileSizeInfoAndPrompt(path, shareSize)) {
        return;
    }

    sharePath = path;
    shareName = path.substr(path.find_last_of("/\\") + 1);

    cout << "\nPort (mặc định 8080): ";
    string portStr;
    getline(cin, portStr);
    if (!portStr.empty()) {
        try {
            int p = stoi(portStr);
            if (p >= 1024 && p <= 65535) httpPort = p;
        } catch (...) {}
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "WSAStartup failed\n";
        return;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        cout << "Socket failed\n";
        WSACleanup();
        return;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(httpPort);

    if (bind(listenSocket, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Bind failed: Port " << httpPort << " đang dùng\n";
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    listen(listenSocket, SOMAXCONN);

    string ip = getLocalIP();
    openFW();

    cout << "\n"<< string(70, '=') << "\n";
    cout << "Server chạy!\n";
    cout << "[IP]: " << ip << " | Port: " << httpPort << " | File: " << shareName << " (" << formatSize(shareSize) << ")\n";
    cout << "[URL]: http://" << ip << ":" << httpPort << "/" << shareName << "\n";

    cout << "\nCtrl+C để dừng\n";
    cout << string(70, '=') << "\n\n";

    while (true) {
        SOCKET client = accept(listenSocket, nullptr, nullptr);
        if (client != INVALID_SOCKET) {
            thread([this, client]() { handleClient(client); }).detach();
        }
    }
}

// Thêm hàm khởi chạy server chat tương tự quickSharePRO

void Internet::startLocalChat() {
    sc.cls();
    httpPort = 9000; 
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(httpPort);

    bind(listenSocket, (sockaddr *)&addr, sizeof(addr));
    listen(listenSocket, SOMAXCONN);

    string ip = getLocalIP();
    cout << "====================================================\n";
    cout << "[CHAT SERVER] DANG CHAY TAI: http://" << ip << ":" << httpPort << "\n";
    cout << "====================================================\n";
    cout << "Nguoi dung cung mang Wi-Fi co the truy cap link tren de chat.\n";
    cout << "Nhan [Ctrl + C] de dong Server khi ket thuc.\n";
    cout << "====================================================\n\n";

    while (true) {
        SOCKET client = accept(listenSocket, nullptr, nullptr);
        if (client != INVALID_SOCKET) {
            // Sử dụng thread để xử lý nhiều người chat cùng lúc
            thread([this, client]() { handleChatClient(client); }).detach();
        }
    }
}

void Internet::handleChatClient(SOCKET client) {
    char buf[4096];
    int rcv = recv(client, buf, sizeof(buf) - 1, 0);
    if (rcv <= 0) return;
    buf[rcv] = '\0';

    HTTPRequest req = parseReq(buf);

    if (req.path == "/" || req.path == "/index.html") {
        string html = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n"
                      "<html><head><title>C++ Local Chat</title>"
                      "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                      "<style>body{font-family:sans-serif; background:#f0f2f5; display:flex; flex-direction:column; align-items:center; margin-top:20px;}"
                      "#box{width:90%; max-width:400px; height:80vh; background:white; border-radius:10px; box-shadow:0 4px 10px rgba(0,0,0,0.1); display:flex; flex-direction:column;}"
                      "#msgs{flex:1; overflow-y:auto; padding:15px; display:flex; flex-direction:column; gap:8px;}"
                      ".msg{background:#e4e6eb; padding:8px 12px; border-radius:15px; width:fit-content; max-width:80%; word-wrap:break-word;}"
                      "input{border:none; padding:15px; border-top:1px solid #ddd; outline:none; font-size:16px; border-bottom-left-radius:10px; border-bottom-right-radius:10px;}</style></head>"
                      "<body><h2>Chat Local</h2><div id='box'><div id='msgs'></div>"
                      "<input type='text' id='inp' placeholder='Nhap tin nhan va nhan Enter...' onkeypress='send(event)'></div>"
                      "<script>"
                      "let msgBox = document.getElementById('msgs');"
                      "function load(){ fetch('/get').then(r=>r.text()).then(t=>{ msgBox.innerHTML = t; msgBox.scrollTop = msgBox.scrollHeight; }); }"
                      "function send(e){ if(e.key==='Enter' && e.target.value.trim() !== ''){ fetch('/send?m=' + encodeURIComponent(e.target.value)); e.target.value=''; load(); } }"
                      "setInterval(load, 1500);" // Cập nhật mỗi 1.5 giây
                      "</script></body></html>";
        send(client, html.c_str(), html.length(), 0);
    }
    else if (req.path.find("/send") == 0) {
        size_t pos = req.path.find("?m=");
        if (pos != string::npos) {
            string msg = req.path.substr(pos + 3);
            
            // Hàm decode URL siêu cơ bản cho C++
            string decodedMsg = "";
            for (size_t i = 0; i < msg.length(); ++i) {
                if (msg[i] == '%') {
                    if (i + 2 < msg.length()) {
                        int value;
                        sscanf(msg.substr(i + 1, 2).c_str(), "%x", &value);
                        decodedMsg += static_cast<char>(value);
                        i += 2;
                    }
                } else if (msg[i] == '+') {
                    decodedMsg += ' ';
                } else {
                    decodedMsg += msg[i];
                }
            }

            chatHistory.push_back(getTime() + " : " + decodedMsg);
            cout << "[NEW MSG] " << decodedMsg << endl;
        }
        string ok = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK";
        send(client, ok.c_str(), ok.length(), 0);
    }
    else if (req.path == "/get") {
        string res = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n";
        for (const auto& m : chatHistory) res += "<div class='msg'>" + m + "</div>";
        send(client, res.c_str(), res.length(), 0);
    }
    closesocket(client);
}