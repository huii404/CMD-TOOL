#ifndef EXPLORER_H
#define EXPLORER_H

#include "SystemCore.h"
#include <string>

class Explorer {
private:
    SystemCore &sc;
    
    // Đưa các hằng số vào làm thuộc tính private của class (Tính đóng gói)
    const std::string OUT_DIR = "Output_Media";
    const std::string FF_PATH = "File_exe\\ffmpeg.exe";
    const std::string SZ_PATH = "File_exe\\7z.exe";

    // Các hàm helper dùng nội bộ (private)
    bool checkToolExists(std::string path, std::string toolName);
    char strongByteProcess(char byte, std::string key, int position, bool isEncrypt);
    std::string prepareOutputPath(std::string input, std::string suffix, std::string extension);
    
    // Core xử lý dùng chung cho Giảm dung lượng và Đổi đuôi
    void processSingleMedia(std::string input, int actionType);
    void executeMediaAction(std::string path, int actionType, int limit);

public:
    Explorer(SystemCore &s);

    // Đưa ra public để main.cpp có thể gọi kiểm tra trước khi chạy chức năng
    bool checkFFmpeg();
    bool check7Zip();

    void clearBrowserCache();
    void nguytrangFolder();
    
    // Sửa lỗi thiếu std:: ở file header
    std::string cleanPath(std::string p); 
    
    // Tính năng 3a & 3b: Xử lý Media (1 = Giảm dung lượng, 2 = Đổi đuôi)
    void handleMediaFeature(int actionType); 
    
    // Tính năng 3c: Cắt video
    void splitVideoMenu();

    // Tính năng 3d: Chụp ảnh từ video
    void captureImagesMenu();
    void enhanceImageMenu();
};

#endif