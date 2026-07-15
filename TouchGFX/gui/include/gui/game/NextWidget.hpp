#ifndef NEXTWIDGET_HPP
#define NEXTWIDGET_HPP

#include <touchgfx/widgets/Widget.hpp>
#include <gui/game/TetrisDefs.hpp>

// Hiển thị khối kế tiếp (NEXT) trong ô 4x4, đặt ở panel HUD bên phải.
class NextWidget : public touchgfx::Widget
{
public:
    NextWidget();
    virtual ~NextWidget() {}

    virtual void draw(const touchgfx::Rect& invalidatedArea) const;
    virtual touchgfx::Rect getSolidRect() const;

    void setPiece(int t)
    {
        pieceType = t;
    }

    // Đổi cạnh ô preview (px) -> ô NEXT nhỏ lại cho HUD 2 người
    void setCell(int px)
    {
        pcell = px;
        setWidthHeight(4 * pcell, 4 * pcell);
    }

private:
    int pieceType;  // -1 = trống, 0..6 = loại khối
    int pcell;      // px mỗi ô preview (mặc định 12)

    void fillR(const touchgfx::Rect& inv, int x, int y, int w, int h, touchgfx::colortype col) const;
};

#endif // NEXTWIDGET_HPP
