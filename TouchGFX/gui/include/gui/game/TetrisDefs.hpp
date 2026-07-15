#ifndef TETRISDEFS_HPP
#define TETRISDEFS_HPP

#include <stdint.h>

// Hằng số & dữ liệu lõi của game Tetris (độc lập TouchGFX để dễ tái dùng/kiểm thử).
namespace tetris
{
// Kích thước bàn chơi
const int COLS = 10;   // số cột
const int ROWS = 20;   // số hàng
const int CELL = 15;   // cạnh 1 ô (px)  -> bàn 150 x 300 (đầy chiều dọc 320)

// Vị trí góc trên-trái của bàn trên màn 240x320 (chừa ~84px bên phải cho HUD)
const int BOARD_X = 6;
const int BOARD_Y = 10;

const int NUM_PIECES = 7; // I, O, T, S, Z, J, L  (mã màu 1..7)

// Hình SPAWN của 7 khối trong ô 4x4 (1 = có ô). Thứ tự: 0=I,1=O,2=T,3=S,4=Z,5=J,6=L
// (P2 sẽ thêm phép xoay; P1 chỉ cần hình spawn để vẽ.)
extern const uint8_t PIECE_SPAWN[NUM_PIECES][4][4];

// Màu RGB cho giá trị ô 0..7 (0 = trống/nền, 1..7 = 7 khối)
extern const uint8_t CELL_RGB[1 + NUM_PIECES][3];

} // namespace tetris

#endif // TETRISDEFS_HPP
