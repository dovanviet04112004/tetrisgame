# HƯỚNG DẪN BUILD & NẠP CHO NGƯỜI MỚI CLONE

> Repo này **không chứa code tự sinh** (framework TouchGFX ~281MB, thư mục
> `TouchGFX/generated/`, `.mxproject`…) — chúng bị loại trong `.gitignore` vì
> CubeMX/TouchGFX Designer sinh lại được. Vì vậy sau khi clone **phải chạy
> 2 bước Generate** trước khi build. Làm đúng thứ tự dưới đây.

---

## 1) Cài đặt (một lần duy nhất)

| Phần mềm | Phiên bản | Ghi chú |
|---|---|---|
| **STM32CubeIDE** | 1.14 trở lên | đã gồm CubeMX + toolchain ARM + ST-LINK driver |
| **X-CUBE-TOUCHGFX** | **4.25.0** (đúng version!) | pack cho CubeMX: mở CubeIDE → Help → Manage Embedded Software Packages → tab STMicroelectronics → X-CUBE-TOUCHGFX → 4.25.0 → Install |
| **TouchGFX Designer** | **4.25.0** | cài kèm khi install pack ở trên (hoặc tải riêng từ st.com) |

⚠️ Version TouchGFX phải **đúng 4.25.0** — khác version sẽ sinh framework lệch, build lỗi hàng loạt.

## 2) Clone repo

```bash
git clone https://github.com/dovanviet04112004/tetrisgame.git
cd tetrisgame
```

## 3) Generate từ CubeMX (phục hồi framework + driver)

1. Mở **STM32CubeIDE** → File → Open Projects from File System → trỏ vào thư mục `STM32CubeIDE/` trong repo → Finish (project `STM32F429I_DISCO_REV_D01` xuất hiện).
2. Double-click file **`STM32F429I_DISCO_REV_D01.ioc`** (ở gốc repo) → mở giao diện CubeMX.
3. **Ctrl+S** → CubeMX hỏi "Do you want to generate Code?" → **Yes**.
   - Bước này phục hồi: `Middlewares/ST/touchgfx/` (framework), `.mxproject`, và các file driver.
   - Nếu CubeMX báo thiếu pack X-CUBE-TOUCHGFX → quay lại bước cài đặt.

## 4) Generate từ TouchGFX Designer (phục hồi GUI generated)

1. Mở file **`TouchGFX/TetrisGame.touchgfx`** (double-click, mở bằng TouchGFX Designer 4.25.0).
2. Bấm nút **Generate Code** (hoặc F4).
   - Bước này phục hồi: `TouchGFX/generated/` (font, text, view base class…) + `TouchGFX/config/`.
3. (Tuỳ chọn) Bấm **Run Simulator** (F5) để chơi thử trên PC — điều khiển bằng bàn phím, không cần board.

## 5) Build

- Trong CubeIDE: chọn project → **Ctrl+B**.
- Kết quả phải **0 errors**. (Cảnh báo `LOAD segment RWX` là bình thường, bỏ qua.)
- Nếu lỗi thiếu file `ili9341.c/stmpe811.c` → Refresh project (F5 trên project) rồi build lại.

## 6) Nạp xuống board

1. Cắm board STM32F429I-DISC1 vào máy qua cổng **USB mini-B của ST-LINK** (cổng CN1, gần đèn LD1 — KHÔNG phải cổng micro-USB OTG ở cạnh dưới).
2. Bấm **Run** (▶) trong CubeIDE → chờ Console hiện `Download verified successfully`.
3. Board tự reset và vào Menu game.

## 7) Chạy thử nhanh (không cần lắp mạch ngoài)

Chưa cắm nút/joystick vẫn test được một phần:
- **Nút B1 (USER, màu xanh trên board)** = PAUSE.
- LED xanh LD3 sáng = âm thanh đang bật (chưa có buzzer thì không nghe tiếng).
- Muốn chơi đầy đủ → lắp mạch theo `docs/P0.2_PINMAP_CubeMX.md` (pin-map) và `docs/P0.3_SO_DO_LAP_MACH.md` (sơ đồ lắp).

---

## Lỗi hay gặp

| Triệu chứng | Nguyên nhân / cách sửa |
|---|---|
| Build lỗi hàng trăm errors kiểu thiếu `touchgfx/...hpp` | Chưa làm bước 3 (CubeMX Generate) — framework chưa được phục hồi |
| Lỗi thiếu `TextKeysAndLanguages.hpp`, `Screen1ViewBase.hpp` | Chưa làm bước 4 (TouchGFX Designer Generate) |
| CubeMX đòi migrate version khi mở .ioc | Cài đúng X-CUBE-TOUCHGFX **4.25.0**, không bấm migrate lung tung |
| Nạp không được, không thấy ST-LINK | Cắm nhầm cổng USB (phải cắm cổng mini-B CN1), hoặc thiếu driver ST-LINK |
| Board chạy nhưng nút ngoài không ăn | Chưa lắp mạch ngoài — xem `docs/P0.3_SO_DO_LAP_MACH.md` |
