#include "include/Explorer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <limits>
#include <algorithm> 
#include <cctype>    

using namespace std;
namespace fs = std::filesystem;

Explorer::Explorer(SystemCore &s) : sc(s) {}

// =========================================================
// CÁC HÀM KIỂM TRA HỆ THỐNG (HEALTH CHECK)
// =========================================================

bool Explorer::checkToolExists(string path, string toolName) {
    if (!fs::exists(path)) {
        sc.setColor(4); 
        cout << "\n[!] CẢNH BÁO: Không tìm thấy '" << toolName << "' tại: " << path << endl;
        cout << "[!] Có thể file bị xóa, mất hoặc bạn chưa tải dữ liệu về.\n";
        cout << "[!] Vui lòng kiểm tra lại thư mục 'File_exe' để sử dụng tính năng này.\n";
        sc.setColor(7); 
        return false;
    }
    return true;
}

bool Explorer::checkFFmpeg() {
    return checkToolExists(FF_PATH, "ffmpeg.exe");
}

bool Explorer::check7Zip() {
    return checkToolExists(SZ_PATH, "7z.exe");
}

// =========================================================
// CÁC HÀM TIỆN ÍCH (HELPER)
// =========================================================

string Explorer::cleanPath(string p) {
    if (!p.empty() && p.front() == '\"') p.erase(0, 1);
    if (!p.empty() && p.back() == '\"') p.erase(p.length() - 1);
    return p;
}

string Explorer::prepareOutputPath(string input, string suffix, string extension) {
    if (!fs::exists(OUT_DIR)) fs::create_directories(OUT_DIR);
    string baseName = fs::path(input).stem().string();
    if (extension == "") extension = fs::path(input).extension().string();
    return OUT_DIR + "\\" + baseName + suffix + extension;
}

char Explorer::strongByteProcess(char byte, string key, int position, bool isEncrypt) {
    if (key.empty()) return byte;
    char k = key[position % key.length()];
    if (isEncrypt) return (byte ^ k) + (position % 7);
    else           return (byte - (position % 7)) ^ k;
}

// =========================================================
// CÁC CHỨC NĂNG CHÍNH CỦA EXPLORER
// =========================================================

void Explorer::clearBrowserCache() {
    char *localAppData = std::getenv("LOCALAPPDATA");
    if (!localAppData) return;

    string baseLocal = string(localAppData);
    vector<string> cachePaths = {
        baseLocal + "\\Google\\Chrome\\User Data\\Default\\Cache",
        baseLocal + "\\Microsoft\\Edge\\User Data\\Default\\Cache",
        baseLocal + "\\CocCoc\\Browser\\User Data\\Default\\Cache",
        baseLocal + "\\Opera Software\\Opera Stable\\Cache"
    };

    cout << "====================================================\n";
    cout << "[SYSTEM] ĐANG DỌN DẸP CACHE MULTI-BROWSER...\n";
    for (const string &path : cachePaths) {
        if (!path.empty() && fs::exists(path)) {
            cout << "[...] Đang dọn: " << path;
            try {
                for (const auto &entry : fs::directory_iterator(path)) fs::remove_all(entry.path());
                cout << " [OK]\n";
            } catch (const fs::filesystem_error &) { cout << " [!] Đang bận\n"; }
        }
    }
    cout << "\n[SUCCESS] Hoàn tất dọn dẹp Cache!\n";
}

void Explorer::nguytrangFolder() {
    string path, name1, nameZip, name3;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Vị trí (vd: D:\\Data): "; getline(cin, path); path = cleanPath(path);
    cout << "Ảnh nền (abc.jpg): "; getline(cin, name1);
    cout << "File zip (abc.zip): "; getline(cin, nameZip);
    cout << "Tên đầu ra (abc.png): "; getline(cin, name3);

    string cmd = "cd /d \"" + path + "\" && copy /b \"" + name1 + "\" + \"" + nameZip + "\" \"" + name3 + "\"";
    sc.runCMD(cmd);
    cout << "\n[OK] Đã tạo file ngụy trang tại: " << path << "\\" << name3 << endl;
}

void Explorer::processSingleMedia(string input, int actionType) {
    string ext = fs::path(input).extension().string();
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    string quotedInput = "\"" + input + "\"";

    bool isVideo = (ext == ".mov" || ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".flv");
    bool isImage = (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".heic" || ext == ".webp");

    if (isVideo) {
        if (actionType == 1) { 
            string output = prepareOutputPath(input, "_compressed", ".mp4");
            cout << "\n[System] Đang nén video: " << fs::path(input).filename() << "...\n";
            string cmdGPU = FF_PATH + " -hwaccel qsv -i " + quotedInput + " -c:v h264_qsv -global_quality 25 -preset faster -pix_fmt yuv420p -y \"" + output + "\"";
            if (system(cmdGPU.c_str()) != 0) {
                cout << "[!] GPU QSV không khả dụng. Chuyển sang CPU...\n";
                string cmdCPU = FF_PATH + " -i " + quotedInput + " -c:v libx264 -crf 28 -threads 4 -preset faster -y \"" + output + "\"";
                system(cmdCPU.c_str());
            }
        } else if (actionType == 2) { 
            string output = prepareOutputPath(input, "_converted", ".mp4");
            cout << "\n[System] Đang chuyển đổi sang MP4: " << fs::path(input).filename() << "...\n";
            string cmd = FF_PATH + " -i " + quotedInput + " -c copy -y \"" + output + "\"";
            system(cmd.c_str());
        }
    } else if (isImage) {
        if (actionType == 1) { 
            string output = prepareOutputPath(input, "_compressed", ".jpg");
            cout << "\n[System] Đang giảm dung lượng ảnh: " << fs::path(input).filename() << "...\n";
            string cmd = FF_PATH + " -i " + quotedInput + " -q:v 5 -y \"" + output + "\""; 
            system(cmd.c_str());
        } else if (actionType == 2) { 
            string output = prepareOutputPath(input, "_converted", ".jpg");
            cout << "\n[System] Đang chuyển đổi sang JPG: " << fs::path(input).filename() << "...\n";
            string cmd = FF_PATH + " -i " + quotedInput + " -q:v 2 -y \"" + output + "\"";
            system(cmd.c_str());
        }
    }
}

void Explorer::executeMediaAction(string path, int actionType, int limit) {
    if (!fs::exists(path)) {
        cout << "[!] Lỗi: Đường dẫn không tồn tại!\n";
        return;
    }

    if (fs::is_regular_file(path)) {
        processSingleMedia(path, actionType);
        cout << "\n[OK] Xử lý xong file!\n";
    } 
    else if (fs::is_directory(path)) {
        int count = 0;
        for (const auto& entry : fs::directory_iterator(path)) {
            if (limit > 0 && count >= limit) {
                cout << "\n[Notice] Đã đạt giới hạn " << limit << " file. Dừng tiến trình.\n";
                break;
            }
            string ext = entry.path().extension().string();
            transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".mov" || ext == ".mp4" || ext == ".mkv" || ext == ".avi" ||
                ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".heic") {
                processSingleMedia(entry.path().string(), actionType);
                count++;
            }
        }
        if (count == 0) cout << "[!] Không tìm thấy file media phù hợp trong thư mục.\n";
        else cout << "\n[OK] Hoàn tất xử lý " << count << " file.\n";
    }
}

void Explorer::handleMediaFeature(int actionType) {
    if (!checkFFmpeg()) return;

    string path;
    int limit = 0;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Kéo File hoặc Thư mục vào đây: "; getline(cin, path);
    path = cleanPath(path);

    if (fs::is_directory(path)) {
        cout << "Bạn muốn xử lý bao nhiêu file trong thư mục? (Nhập 0 để xử lý TẤT CẢ): ";
        cin >> limit;
    }

    executeMediaAction(path, actionType, limit);
}

void Explorer::splitVideoMenu() {
    if (!checkFFmpeg()) return;

    string input;
    int parts, totalSeconds;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Kéo file video vào: "; getline(cin, input);
    input = cleanPath(input);
    
    cout << "Tổng thời gian video (tính bằng giây): "; cin >> totalSeconds;
    cout << "Muốn chia thành bao nhiêu video bằng nhau: "; cin >> parts;

    if (parts <= 0 || totalSeconds <= 0) return;
    
    int segmentTime = totalSeconds / parts;
    string outputPattern = prepareOutputPath(input, "_part_%03d", ".mp4");
    
    cout << "\n[System] Đang cắt video...\n";
    string cmd = FF_PATH + " -i \"" + input + "\" -c copy -f segment -segment_time " + to_string(segmentTime) + " -reset_timestamps 1 -y \"" + outputPattern + "\"";
    sc.runCMD(cmd);
    cout << "[OK] Hoàn tất cắt video!\n";
}

void Explorer::captureImagesMenu() {
    if (!checkFFmpeg()) return;

    string input;
    int mode;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Kéo file video vào: "; getline(cin, input);
    input = cleanPath(input);

    cout << "\n[1] Chụp cân bằng (chia đều N ảnh trên toàn video)\n[2] Chụp Check Var (Chụp N ảnh liên tiếp từ 1 thời điểm)\n[0] Quay lại\n[Chọn]: ";
    cin >> mode;

    if (mode == 1) {
        int n, totalSeconds;
        cout << "Tổng thời gian video (tính bằng giây): "; cin >> totalSeconds;
        cout << "Số lượng ảnh muốn chụp: "; cin >> n;
        
        if (totalSeconds > 0 && n > 0) {
            double fps = (double)n / totalSeconds;
            string output = prepareOutputPath(input, "_canbang_%03d", ".jpg");
            string cmd = FF_PATH + " -i \"" + input + "\" -vf \"fps=" + to_string(fps) + "\" -frames:v " + to_string(n) + " -y \"" + output + "\"";
            sc.runCMD(cmd);
        }
    } 
    else if (mode == 2) {
        string startTime;
        int n;
        double interval;
        cout << "Thời gian bắt đầu (vd: 10 hoặc 00:01:20): "; cin >> startTime;
        cout << "Số lượng ảnh: "; cin >> n;
        cout << "Khoảng cách giữa các ảnh (giây, vd: 0.5 hoặc 1): "; cin >> interval;
        
        if (interval > 0 && n > 0) {
            string output = prepareOutputPath(input, "_checkvar_%03d", ".jpg");
            string cmd = FF_PATH + " -ss " + startTime + " -i \"" + input + "\" -vf \"fps=1/" + to_string(interval) + "\" -frames:v " + to_string(n) + " -y \"" + output + "\"";
            sc.runCMD(cmd);
        }
    }
}

void Explorer::enhanceImageMenu() {
    if (!checkFFmpeg()) return; 

    string input;
    int mode;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Kéo file ảnh vào: "; getline(cin, input);
    input = cleanPath(input);

    if (!fs::exists(input)) {
        cout << "[!] File không tồn tại!\n";
        return;
    }

    cout << "\nChọn chế độ xử lý:\n";
    cout << "[1] Nhẹ (Tự nhiên)\n[2] Trung bình\n[3] Mạnh\n[4] Siêu nét\n[5] Phóng to X2 + Làm nét\n";
    cout << "[Chọn]: "; cin >> mode;

    string output = prepareOutputPath(input, "_enhanced", ".jpg");
    string filterStr = "";
    
    switch (mode) {
        case 1: filterStr = "unsharp=5:5:0.7:5:5:0.0"; break;
        case 2: filterStr = "unsharp=5:5:1.0:5:5:0.0"; break;
        case 3: filterStr = "unsharp=7:7:1.3:7:7:0.0"; break;
        case 4: filterStr = "unsharp=7:7:2.5:7:7:0.0"; break;
        case 5: filterStr = "scale=iw*2:-1:flags=lanczos,unsharp=5:5:1.2:5:5:0.0"; break;
        default: filterStr = "unsharp=5:5:1.0:5:5:0.0"; break;
    }

    string cmd = FF_PATH + " -i \"" + input + "\" -vf \"" + filterStr + "\" -q:v 2 -y \"" + output + "\"";

    cout << "\n[System] Đang thực thi thuật toán... Vui lòng đợi.\n";
    sc.runCMD(cmd);

    if (fs::exists(output)) {
        cout << "[OK] Hoàn tất! Ảnh mới nét hơn đáng kể tại: " << output << endl;
        if (mode == 5) cout << "[i] Lưu ý: Ảnh đã được phóng to gấp đôi kích thước gốc.\n";
    } else {
        cout << "[!] Có lỗi xảy ra trong quá trình FFmpeg xử lý.\n";
    }
}