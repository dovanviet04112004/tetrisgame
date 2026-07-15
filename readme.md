# 🎮 Tetris trên STM32F429I-DISC1

Bài tập lớn Hệ nhúng — game **Tetris** chạy trên kit **STM32F429I-DISC1**
(LCD 240×320), xây dựng bằng **STM32CubeIDE + TouchGFX 4.25.0 + FreeRTOS**.

**Thành viên:**
| Họ tên | Phân công |
|---|---|
| Đỗ Văn Việt | Phần cứng (pin-map, lắp mạch, nút/joystick/buzzer, RNG, DMA/ADC) + driver I/O |
| Nguyễn Tuấn Cảnh | Engine luật game + giao diện TouchGFX (menu, HUD, PvP) |

## ✨ Tính năng

- **4 chế độ chơi đơn**: Marathon (leo level) / Sprint (đua 40 dòng) / Ultra (3 phút) / Zen (thư giãn)
- **PvP 2 người** trên cùng màn hình: 1 người dùng 7 nút bấm, 1 người dùng joystick + 2 nút
- Ghost piece, HOLD, NEXT, tăng tốc theo level, điểm cao lưu **Flash** (không mất khi tắt nguồn)
- **Số ngẫu nhiên phần cứng** (HAL_RNG): sinh khối 7-bag bằng RNG peripheral của STM32 ✔ *(yêu cầu ①)*
- **Âm thanh** qua buzzer thụ động (PWM TIM3): nhạc nền Korobeiniki + hiệu ứng ✔ *(yêu cầu ②)*
- Tắt/bật tiếng bằng nút HOLD tại Menu (LED xanh = có tiếng, LED đỏ = tắt tiếng)

## 🔌 Phần cứng

- Kit STM32F429I-DISC1 (LCD ILI9341 240×320, chạm STMPE811)
- Joystick analog KY-023 (2 trục ADC + nút SW)
- 9 nút bấm rời + buzzer thụ động KY-006
- Chi tiết đấu nối: [docs/P0.2_PINMAP_CubeMX.md](docs/P0.2_PINMAP_CubeMX.md) (pin-map)
  và [docs/P0.3_SO_DO_LAP_MACH.md](docs/P0.3_SO_DO_LAP_MACH.md) (sơ đồ lắp breadboard)

## 🚀 Build & nạp

Repo **không chứa code tự sinh** (framework TouchGFX, `generated/`…) — sau khi
clone phải Generate lại bằng CubeMX + TouchGFX Designer **4.25.0** rồi mới build.

👉 Làm theo từng bước trong [docs/HUONG_DAN_BUILD.md](docs/HUONG_DAN_BUILD.md).

Tóm tắt: mở `.ioc` → Ctrl+S Generate → mở `TouchGFX/TetrisGame.touchgfx` →
Generate Code → Ctrl+B build → cắm USB ST-LINK (CN1) → Run ▶.

Có thể chạy thử **simulator trên PC** (không cần board): mở file `.touchgfx`
trong TouchGFX Designer → Run Simulator (F5), điều khiển bằng bàn phím.

## 🕹️ Điều khiển (trên board)

| Nút | Chân | Chức năng |
|---|---|---|
| LEFT / RIGHT | PG2 / PG3 | di chuyển khối |
| ROTATE CW / CCW | PC11 / PD4 | xoay khối |
| SOFT / HARD DROP | PD2 / PD7 | rơi nhanh / thả thẳng |
| HOLD | PD5 | giữ khối; tại Menu = bật/tắt tiếng; màn End = chơi lại |
| PAUSE | B1 (trên board) | tạm dừng |
| Joystick | PC3/PA7 + SW PG9 | Menu: chọn chế độ; PvP: điều khiển người chơi 2 |
| HOLD P2 / CCW P2 | PB7 / PC12 | PvP: giữ khối / xoay ngược cho người chơi 2 |
