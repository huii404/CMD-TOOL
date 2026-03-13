# 🛠 Windows Toolkit v1.3.0 PRO BETA
> **All-in-One System Optimizer & Management Tool**
> Giải pháp tối ưu hóa hệ thống mạnh mẽ được phát triển bởi **Nhân (Huii404)**.

---

## 📋 Mục lục
- [Giới thiệu](#-giới-thiệu)
- [Review Tác vụ Chi tiết](#-review-tác-vụ-chi-tiết)
- [Cơ chế Whitelist](#-cơ-chế-whitelist)
- [Hướng dẫn Kỹ thuật](#-hướng-dẫn-kỹ-thuật)
- [Nhật ký Cập nhật](#-nhật-ký-cập-nhật)

---

## 🌟 Giới thiệu
Toolkit được xây dựng trên ngôn ngữ **C++**, tập trung vào việc can thiệp sâu vào hệ thống thông qua **Win32 API** thay vì chỉ sử dụng các lệnh CMD thông thường. Dự án này giúp người dùng kiểm soát hoàn toàn các dịch vụ ngầm, gỡ bỏ ứng dụng rác (Bloatware) và tối ưu hóa tốc độ xử lý của Windows.

---

## 🔍 Review Tác vụ Chi tiết

### 1. 🚀 Tối ưu hóa Hệ thống (System Optimal)
Hàm `optimizeSystemPRO()` tích hợp các kỹ thuật tinh chỉnh Registry nâng cao:
- **Dọn rác (Clean Disk):** Xóa sạch file tạm, Prefetch và logs để giải phóng ổ cứng.
- **Giao diện (UI Lag):** Tinh chỉnh `VisualFXSetting` để Windows chạy mượt mà hơn trên các máy cấu hình yếu.
- **Chặn App ngầm:** Vô hiệu hóa tính năng tự tải ứng dụng quảng cáo từ Microsoft.

### 2. 🛡️ Quản lý Dịch vụ (Service Management)
Sử dụng logic **"Thụ động & Chủ động"** để quản lý tài nguyên:
- **Trạng thái Manual:** Đưa các dịch vụ như Xbox về chế độ chờ, không chạy ngầm lúc khởi động nhưng vẫn có thể tự bật khi cần chơi game.
- **Trạng thái Disabled:** Khóa vĩnh viễn các dịch vụ không cần thiết để đạt hiệu suất tối đa.
- **API Win32:** Sử dụng `OpenSCManager` và `ChangeServiceConfigA` để điều khiển trực tiếp, đảm bảo tốc độ và độ tin cậy.

### 3. 🧹 Gỡ bỏ App rác (Bloatware Uninstaller)
Cơ chế gỡ cài đặt thông minh thông qua PowerShell API:
- **Quét thông minh:** Tự động nhận diện các gói Appx không thiết yếu.
- **Lọc Whitelist:** Bảo vệ các ứng dụng quan trọng không bị xóa nhầm.

### 4. 🌐 Quản lý Mạng (Internet Utilities)
- **WiFi Audit:** Trích xuất và hiển thị mật khẩu WiFi đã lưu trên thiết bị.
- **Network Reset:** Reset TCP/IP và Flush DNS để sửa nhanh các lỗi kết nối.

---

## 🛡️ Cơ chế Whitelist (Danh sách an toàn)
Công cụ cam kết sự minh bạch bằng cách giữ lại các thành phần cốt lõi của hệ thống:

| Thành phần | Trạng thái | Lý do giữ lại |
| :--- | :--- | :--- |
| **Microsoft Store** | **Cấm xóa** | Để người dùng có thể tải app khi cần |
| **AAD.BrokerPlugin** | **Cấm xóa** | Đảm bảo tính năng đăng nhập tài khoản MS ổn định |
| **Calculator** | **Cấm xóa** | Tiện ích máy tính cơ bản |
| **Paint / Photos** | **Cấm xóa** | Các trình xem ảnh và vẽ cơ bản |

---

## 🛠 Hướng dẫn Kỹ thuật
### Yêu cầu
- **OS:** Windows 10/11 (Yêu cầu quyền Administrator).
- **Compiler:** Visual Studio (Khuyên dùng) hoặc g++ (cần link `-ladvapi32`).

### Cách biên dịch
```bash
g++ tool.cpp -o Toolkit.exe -ladvapi32