#ifndef INTERNET_H
#define INTERNET_H

#include <winsock2.h>
#include <windows.h>
#include <string>
#include <map>
#include <vector>
#include "SystemCore.h"

// Struct dùng nội bộ cho Internet
struct HTTPRequest {
    std::string method, path, httpVersion;
    std::map<std::string, std::string> headers;
};

class Internet {
private:
    SystemCore &sc;
    SOCKET listenSocket;
    int httpPort;
    std::string sharePath;
    std::string shareName;
    long long shareSize;
    int dlCount;
    
    // HẰNG SỐ GIỚI HẠN KÍCH THƯỚC 
    const long long MAX_FILE_SIZE = 1500000000LL; // 1.5 GB

    // Khai báo biến lưu lịch sử chat (MỚI)
    std::vector<std::string> chatHistory;

    std::string getField(const std::string &line);
    std::string getContentType(const std::string &fpath);
    std::string getLocalIP();
    void openFW();
    std::string formatSize(long long b);
    std::string getTime();
    HTTPRequest parseReq(const std::string &raw);
    void sendFile(SOCKET client);
    void handleClient(SOCKET client);
    bool checkFileSizeAndConfirm(const std::string &path, long long &outSize);
    bool getFileSizeInfoAndPrompt(const std::string &path, long long &outSize);

    // Khai báo hàm xử lý luồng Chat (MỚI)
    void handleChatClient(SOCKET client);

public:
    Internet(SystemCore &s);
    ~Internet();

    void showIP();
    void renewIP();
    void flushdns();
    void netsh_tcpIP();
    void wifiAudit();
    void quickSharePRO();
    
    // Khai báo hàm khởi chạy Chat (MỚI)
    void startLocalChat();
};

#endif // INTERNET_H