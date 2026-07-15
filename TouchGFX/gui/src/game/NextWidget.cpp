#include <gui/game/NextWidget.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/hal/HAL.hpp>

using namespace touchgfx;

NextWidget::NextWidget()
    : pieceType(-1), pcell(12)
{
    setWidthHeight(4 * pcell, 4 * pcell);
}

Rect NextWidget::getSolidRect() const
{
    return Rect(0, 0, getWidth(), getHeight());
}

void NextWidget::fillR(const Rect& inv, int x, int y, int w, int h, colortype col) const
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

void NextWidget::draw(const Rect& invalidatedArea) const
{
    const int e = (pcell >= 14) ? 2 : 1;
    const colortype grid = Color::getColorFromRGB(64, 70, 98);

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            uint8_t v = 0;
            if (pieceType >= 0 && pieceType < tetris::NUM_PIECES &&
                tetris::PIECE_SPAWN[pieceType][r][c])
            {
                v = (uint8_t)(pieceType + 1);
            }
            const uint8_t* b = tetris::CELL_RGB[v];
            const uint8_t cr = b[0], cg = b[1], cb = b[2];

            const int gx = c * pcell, gy = r * pcell;
            const int x0 = gx + 1, y0 = gy + 1, w0 = pcell - 1, h0 = pcell - 1;

            fillR(invalidatedArea, gx, gy, pcell, pcell, grid);
            fillR(invalidatedArea, x0, y0, w0, h0, Color::getColorFromRGB(cr, cg, cb));

            if (v >= 1) // khối đặc -> viền nổi 3D
            {
                const colortype lt = Color::getColorFromRGB(
                    (uint8_t)(cr + (255 - cr) * 45 / 100),
                    (uint8_t)(cg + (255 - cg) * 45 / 100),
                    (uint8_t)(cb + (255 - cb) * 45 / 100));
                const colortype dk = Color::getColorFromRGB(
                    (uint8_t)(cr * 45 / 100), (uint8_t)(cg * 45 / 100), (uint8_t)(cb * 45 / 100));
                fillR(invalidatedArea, x0, y0, w0, e, lt);
                fillR(invalidatedArea, x0, y0, e, h0, lt);
                fillR(invalidatedArea, x0, y0 + h0 - e, w0, e, dk);
                fillR(invalidatedArea, x0 + w0 - e, y0, e, h0, dk);
            }
        }
    }
}
