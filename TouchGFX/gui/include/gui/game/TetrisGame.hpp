#ifndef TETRISGAME_HPP
#define TETRISGAME_HPP

#include <stdint.h>
#include <gui/game/TetrisDefs.hpp>

namespace tetris
{
// Lệnh điều khiển — dùng chung cho bàn phím (simulator), nút bấm & joystick (phần cứng)
enum Command
{
    CMD_LEFT,
    CMD_RIGHT,
    CMD_ROT_CW,
    CMD_ROT_CCW,
    CMD_SOFT_DROP,
    CMD_HARD_DROP
};

// Mã giao diện trên hàng đợi nhập (InputTask -> View), tách khỏi enum Command
// (nước đi engine 0..5) vì đây là lệnh điều khiển màn hình.
static const uint8_t CMD_PAUSE_CODE   = 6; // nút PAUSE (PA0): tạm dừng / tiếp tục
static const uint8_t CMD_RESTART_CODE = 7; // nút HOLD P1 (PD5): giữ khối khi chơi / chơi lại ở màn End
static const uint8_t CMD_HOLD_P2_CODE = 8; // nút HOLD P2 (PB7): chỉ dùng ở PvP -> joystick player giữ khối

// Chế độ chơi (BTL yêu cầu 4 chế độ)
enum Mode
{
    MODE_MARATHON, // chơi vô tận, lên level theo số hàng, thua khi đầy đỉnh
    MODE_SPRINT,   // đua xoá 40 hàng nhanh nhất (đếm giờ tăng)
    MODE_ULTRA,    // 2:00 đếm ngược, ăn điểm tối đa
    MODE_ZEN       // thư giãn: không thua, tốc độ nhẹ cố định
};
static const uint8_t MODE_COUNT = 4;

// Sự kiện âm thanh — mã KHỚP bảng SFX trong AudioTask (Core/Src/main.c)
enum Sound
{
    SND_MOVE,
    SND_ROTATE,
    SND_LOCK,
    SND_LINE,
    SND_TETRIS,
    SND_LEVELUP,
    SND_GAMEOVER
};

// Lõi luật game Tetris (thuần logic, không phụ thuộc TouchGFX -> dễ kiểm thử & tái dùng).
class TetrisGame
{
public:
    TetrisGame();

    void start(Mode m = MODE_MARATHON); // reset bàn theo chế độ + spawn khối đầu tiên
    bool onFrameTick();                 // gọi MỖI FRAME (60fps): trọng lực + lock delay; true = cần vẽ lại
    void command(Command c);            // xử lý 1 lệnh điều khiển
    void hold();                        // giữ khối hiện tại / đổi với khối đã giữ (1 lần/khối)
    void renderTo(uint8_t* grid) const; // gộp bàn + khối hiện tại -> grid[ROWS*COLS]

    bool     isGameOver() const { return gameOver; }
    int      getHoldType() const { return holdPiece; }      // khối đang giữ (-1 = trống)
    uint16_t getLines() const   { return lines; }
    uint32_t getScore() const   { return score; }
    uint8_t  getLevel() const   { return level; }
    uint8_t  getMode() const    { return mode; }            // chế độ đang chơi
    int      getNextType() const { return nextPiece; }      // khối kế tiếp (cho ô NEXT)
    int      getGravityFrames() const; // số frame/ô rơi (giảm dần theo level)

private:
    uint8_t  board[ROWS][COLS]; // 0 = trống, 1..7 = mã màu khối đã khoá
    int      curType;           // 0..6 (I,O,T,S,Z,J,L)
    int      curRot;            // 0..3 (số lần xoay CW)
    int      curX, curY;        // toạ độ ô (0,0) của hộp 4x4 trong bàn
    int      nextPiece;         // khối kế tiếp (lookahead, hiển thị ô NEXT)
    bool     gameOver;
    uint16_t lines;
    uint32_t score;
    uint8_t  level;             // bắt đầu 1, +1 mỗi 10 hàng
    uint8_t  mode;              // chế độ chơi (tetris::Mode)
    uint32_t rngState;          // RNG phần mềm — chỉ dùng cho simulator
    uint8_t  bag[NUM_PIECES];   // túi 7 khối (thuật toán 7-bag)
    uint8_t  bagIdx;            // vị trí lấy hiện tại trong túi
    int      holdPiece;         // khối đang giữ (-1 = chưa giữ gì)
    bool     holdUsed;          // đã giữ trong lượt khối này chưa (mỗi khối chỉ giữ 1 lần)
    uint8_t  gravCounter;       // đếm frame cho trọng lực (đủ getGravityFrames() -> rơi 1 ô)
    uint8_t  lockFrames;        // lock delay: đếm frame từ khi khối chạm đáy (đủ 30 -> khoá)
    uint8_t  lockResets;        // số lần đã hoãn khoá do di chuyển/xoay (move-reset, tối đa 15)

    bool cellFilled(int type, int rot, int r, int c) const; // ô (r,c) hộp 4x4 có khối?
    bool grounded() const;                                  // khối hiện tại đã chạm đáy/khối khác?
    void tryRotate(int dir);                                // xoay +1 CW / -1 CCW, thử 5 kick SRS
    void onSuccessfulMove();                                // move-reset: hoãn lock delay khi thao tác thành công
    void resetPieceTimers();                                // reset đồng hồ trọng lực + lock cho khối mới
    bool collides(int type, int rot, int x, int y) const;   // đè biên hoặc khối khác?
    void lockPiece();                                       // ghi khối hiện tại vào board
    void clearLines();                                      // xoá các hàng đầy, dồn xuống
    void spawn();                                           // sinh khối mới (-> gameOver nếu kẹt)
    int  nextType();                                        // lấy khối kế tiếp từ túi 7-bag
    void refillBag();                                       // đổ + xáo túi 7 khối (Fisher-Yates)
    uint32_t randomValue();                                 // RNG: phần cứng (board) / phần mềm (sim)
    void emitSound(uint8_t id);                             // đẩy sự kiện âm vào soundQueue (board) / no-op (sim)
};

} // namespace tetris

#endif // TETRISGAME_HPP
