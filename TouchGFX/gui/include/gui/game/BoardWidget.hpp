#ifndef BOARDWIDGET_HPP
#define BOARDWIDGET_HPP

#include <touchgfx/widgets/Widget.hpp>
#include <gui/game/TetrisDefs.hpp>

// Custom widget tự vẽ bàn Tetris 10x20 từ một mảng ô (row-major).
// Mỗi ô là 1 byte: 0 = trống, 1..7 = mã màu khối. Vẽ bằng LCD::fillRect (cách chuẩn TouchGFX).
class BoardWidget : public touchgfx::Widget
{
public:
    BoardWidget();
    virtual ~BoardWidget() {}

    // Bắt buộc override (Widget là abstract)
    virtual void draw(const touchgfx::Rect& invalidatedArea) const;
    virtual touchgfx::Rect getSolidRect() const;

    // Trỏ tới mảng ROWS*COLS byte chứa trạng thái bàn (đã gộp khối hiện tại)
    void setCells(const uint8_t* c)
    {
        cells = c;
    }

    // Đổi cạnh 1 ô (px) -> bàn nhỏ lại cho chế độ 2 người. Tự cập nhật kích thước widget.
    void setCell(int px)
    {
        cellPx = px;
        setWidthHeight(tetris::COLS * cellPx, tetris::ROWS * cellPx);
    }

private:
    const uint8_t* cells;
    int            cellPx;

    // Tô 1 hình chữ nhật (toạ độ trong widget), tự cắt theo vùng cần vẽ lại + đổi sang toạ độ tuyệt đối
    void fillR(const touchgfx::Rect& inv, int x, int y, int w, int h, touchgfx::colortype col) const;
};

#endif // BOARDWIDGET_HPP
