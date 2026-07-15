#include <gui/game/BoardWidget.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/hal/HAL.hpp>

using namespace touchgfx;

BoardWidget::BoardWidget()
    : cells(0), cellPx(tetris::CELL)
{
    // Kích thước widget = đúng kích thước bàn (mặc định ô 15px, PvP đổi nhỏ hơn)
    setWidthHeight(tetris::COLS * cellPx, tetris::ROWS * cellPx);
}

Rect BoardWidget::getSolidRect() const
{
    // Toàn bộ widget là đặc (mọi ô đều được tô) -> giúp framework tối ưu vẽ
    return Rect(0, 0, getWidth(), getHeight());
}

void BoardWidget::fillR(const Rect& inv, int x, int y, int w, int h, colortype col) const
{
    if (w <= 0 || h <= 0)
        return;
    Rect d(x, y, w, h);
    d = d & inv;
    if (!d.isEmpty())
    {
        translateRectToAbsolute(d);
        HAL::lcd().fillRect(d, col, 255);
    }
}

void BoardWidget::draw(const Rect& invalidatedArea) const
{
    const int e = (cellPx >= 14) ? 2 : 1;                 // độ dày viền nổi (px)
    const colortype grid = Color::getColorFromRGB(64, 70, 98);

    for (int r = 0; r < tetris::ROWS; r++)
    {
        for (int c = 0; c < tetris::COLS; c++)
        {
            const uint8_t v = cells ? cells[r * tetris::COLS + c] : 0;
            const bool solid = (v >= 1 && v <= 7);        // khối đặc -> vẽ nổi 3D; trống/bóng -> phẳng

            // Màu ruột ô: 1..7 = màu thật; 9..15 = BÓNG (mờ ~40%); 0 = nền
            uint8_t cr, cg, cb;
            if (v >= 9)
            {
                const uint8_t* b = tetris::CELL_RGB[v - 8];
                cr = (uint8_t)(b[0] * 2 / 5); cg = (uint8_t)(b[1] * 2 / 5); cb = (uint8_t)(b[2] * 2 / 5);
            }
            else
            {
                const uint8_t* b = tetris::CELL_RGB[(v <= 7) ? v : 0];
                cr = b[0]; cg = b[1]; cb = b[2];
            }

            const int gx = c * cellPx, gy = r * cellPx;
            const int x0 = gx + 1, y0 = gy + 1, w0 = cellPx - 1, h0 = cellPx - 1;

            fillR(invalidatedArea, gx, gy, cellPx, cellPx, grid);              // khung lưới 1px
            fillR(invalidatedArea, x0, y0, w0, h0, Color::getColorFromRGB(cr, cg, cb)); // ruột

            if (solid)
            {
                // Viền nổi: sáng trên-trái, tối dưới-phải -> khối trông như gạch 3D
                const colortype lt = Color::getColorFromRGB(
                    (uint8_t)(cr + (255 - cr) * 45 / 100),
                    (uint8_t)(cg + (255 - cg) * 45 / 100),
                    (uint8_t)(cb + (255 - cb) * 45 / 100));
                const colortype dk = Color::getColorFromRGB(
                    (uint8_t)(cr * 45 / 100), (uint8_t)(cg * 45 / 100), (uint8_t)(cb * 45 / 100));
                fillR(invalidatedArea, x0, y0, w0, e, lt);           // mép trên (sáng)
                fillR(invalidatedArea, x0, y0, e, h0, lt);           // mép trái (sáng)
                fillR(invalidatedArea, x0, y0 + h0 - e, w0, e, dk);  // mép dưới (tối)
                fillR(invalidatedArea, x0 + w0 - e, y0, e, h0, dk);  // mép phải (tối)
            }
        }
    }
}
