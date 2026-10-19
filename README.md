# 🔐 Khóa Cửa Vân Tay ESP32 (AS608 + OLED + Servo)

Hệ thống khóa cửa thông minh dùng **ESP32**, cảm biến vân tay **AS608**, màn hình **OLED SSD1306**, **servo quay liên tục (360°)** để mở/khóa cơ cấu cửa, kèm còi cảnh báo khi quét sai nhiều lần.

## ✨ Tính năng

- Quét vân tay để mở khóa tự động: servo mở → giữ cửa mở → tự khóa lại.
- Hiển thị trạng thái trực tiếp trên màn hình OLED.
- Cảnh báo bằng còi **2 giây** khi quét sai **3 lần liên tiếp**.
- Nút nhấn vật lý để **bật/tắt** chế độ quét vân tay.
- Menu điều khiển qua Serial Monitor:
  - Nạp vân tay mới theo ID (1–127)
  - Xóa vân tay theo ID
  - Xóa toàn bộ vân tay (có xác nhận Y/N)
  - Xem số vân tay đã lưu
- In log chi tiết (ID, độ tin cậy, số lần sai…) ra Serial.

## 🧰 Phần cứng

| Linh kiện | Số lượng | Ghi chú |
|---|---|---|
| ESP32 DevKit | 1 | |
| Cảm biến vân tay AS608 | 1 | UART 57600 baud |
| OLED SSD1306 128x64 (I2C) | 1 | Địa chỉ `0x3C` |
| Servo quay liên tục 360° | 1 | Điều khiển bằng xung µs |
| Còi buzzer (active) | 1 | |
| Nút nhấn | 1 | Dùng `INPUT_PULLUP` |

## 🔌 Sơ đồ nối chân

| Thiết bị | Chân thiết bị | Chân ESP32 |
|---|---|---|
| AS608 | TX | GPIO 16 (RX2) |
| AS608 | RX | GPIO 17 (TX2) |
| AS608 | VCC / GND | 3.3V (hoặc 5V theo module) / GND |
| OLED | SDA | GPIO 21 |
| OLED | SCL | GPIO 22 |
| OLED | VCC / GND | 3.3V / GND |
| Servo | Signal | GPIO 18 |
| Servo | VCC / GND | 5V (nguồn riêng nếu cần) / GND |
| Nút nhấn | 1 chân | GPIO 27 (chân còn lại nối GND) |
| Còi | + | GPIO 26 |
| Còi | − | GND |

> ⚠️ **Lưu ý:** TX của AS608 nối với RX của ESP32 và ngược lại. Nên cấp nguồn riêng cho servo và nối chung GND với ESP32 để tránh sụt áp, reset board.

## 📚 Thư viện cần cài

Cài qua **Arduino IDE → Library Manager**:

- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `Adafruit Fingerprint Sensor Library`
- `ESP32Servo`

Board: **ESP32 Dev Module** (cài ESP32 board package của Espressif).

## ⚙️ Cấu hình servo

Servo trong dự án được điều khiển như servo 360° (quay liên tục), bằng độ rộng xung:

```cpp
#define SERVO_STOP    1500   // Dừng
#define OPEN_SPEED    1300   // Quay chiều mở
#define CLOSE_SPEED   1700   // Quay chiều khóa

#define OPEN_TIME     3000   // Thời gian quay để mở (ms)
#define CLOSE_TIME    3000   // Thời gian quay để khóa (ms)
#define DOOR_OPEN_TIME 5000  // Thời gian giữ cửa mở (ms)
```

- Nếu servo quay **ngược chiều**, hãy hoán đổi giá trị `OPEN_SPEED` và `CLOSE_SPEED`.
- Nếu servo **trôi/quay chậm khi dừng**, tinh chỉnh `SERVO_STOP` quanh 1500 µs (ví dụ 1480–1520).
- Điều chỉnh `OPEN_TIME` / `CLOSE_TIME` cho phù hợp với cơ cấu khóa thực tế.

## 🚀 Cách sử dụng

1. Nối dây theo sơ đồ ở trên.
2. Mở file `.ino` bằng Arduino IDE, chọn đúng board và cổng COM.
3. Nạp code lên ESP32.
4. Mở **Serial Monitor** với tốc độ **115200 baud**.
5. Dùng menu Serial để nạp vân tay, sau đó đặt ngón tay lên cảm biến để mở khóa.

### Menu Serial

Gõ số tương ứng rồi nhấn Enter:

| Lệnh | Chức năng |
|---|---|
| `1` | Nạp vân tay mới (nhập ID 1–127, đặt ngón tay 2 lần) |
| `2` | Xóa vân tay theo ID |
| `3` | Xóa tất cả vân tay (nhập `Y` để xác nhận, `N` để hủy) |
| `4` | Xem số vân tay đã lưu |

### Nút nhấn

Nhấn nút trên **GPIO 27** để bật/tắt quét vân tay (có chống rung 300 ms). Trạng thái hiển thị trên OLED và Serial.

## 🔄 Luồng hoạt động

```
Đặt ngón tay
   │
   ├── Đúng ──► Reset đếm sai ─► Servo mở ─► Giữ cửa mở 5s ─► Servo khóa ─► Sẵn sàng
   │
   └── Sai ───► Tăng đếm sai
                  ├── Lần 1, 2: báo "VÂN TAY SAI"
                  └── Lần 3: còi kêu 2s, reset bộ đếm
```

## 🗂️ Cấu trúc mã nguồn

| Hàm | Mô tả |
|---|---|
| `showOLED()` | Hiển thị 2 dòng chữ lên OLED |
| `buzzer()` | Bật còi trong khoảng thời gian cho trước |
| `unlockDoor()` | Quy trình mở – giữ – khóa cửa bằng servo |
| `checkButton()` | Đọc nút nhấn, bật/tắt quét vân tay |
| `checkFingerprint()` | Quét và đối chiếu vân tay |
| `enrollFingerprint()` | Nạp vân tay mới (2 lần quét) |
| `deleteFingerprint()` | Xóa vân tay theo ID |
| `deleteAllFingerprints()` | Xóa toàn bộ cơ sở dữ liệu vân tay |
| `serialMenu()` | Xử lý menu qua Serial |

## 🛠️ Xử lý sự cố

| Hiện tượng | Cách khắc phục |
|---|---|
| `AS608 KHONG KET NOI` | Kiểm tra lại TX/RX (đảo chiều), nguồn và GND |
| OLED không sáng | Kiểm tra SDA/SCL và địa chỉ I2C (`0x3C` hoặc `0x3D`) |
| Servo quay liên tục khi khởi động | Chỉnh `SERVO_STOP` cho đúng điểm dừng |
| ESP32 tự reset khi servo chạy | Cấp nguồn riêng cho servo, nối chung GND |
| OLED hiển thị lỗi font tiếng Việt | Font mặc định không hỗ trợ dấu, dùng tiếng Việt không dấu |

## 📝 Ghi chú

- Font mặc định của Adafruit GFX **không hỗ trợ ký tự có dấu**. Chuỗi như `"xin mời thầy vào ..."` trên OLED sẽ hiển thị sai; nên đổi thành `"xin moi thay vao ..."` hoặc dùng thư viện font Unicode (ví dụ U8g2).
- Dự án mang tính học tập/mô hình. Nếu dùng cho cửa thực tế, nên bổ sung nguồn dự phòng, khóa cơ dự phòng và cơ chế bảo mật bổ sung.

## 📄 Giấy phép

Phát hành theo giấy phép [MIT](LICENSE). Bạn có thể tự do sử dụng, chỉnh sửa và chia sẻ.
