#include <gui/game/MenuBgWidget.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/hal/HAL.hpp>

using namespace touchgfx;

MenuBgWidget::MenuBgWidget()
{
}

Rect MenuBgWidget::getSolidRect() const
{
    return Rect(0, 0, getWidth(), getHeight()); // đặc -> che hết bàn/HUD phía sau khi ở menu
}

void MenuBgWidget::draw(const Rect& invalidatedArea) const
{
    const int W = getWidth();
    const int H = getHeight();
    if (H <= 0)
        return;

    // Gradient dọc: trên (20,26,54) -> dưới (4,6,16)
    const int topR = 20, topG = 26, topB = 54;
    const int botR = 4,  botG = 6,  botB = 16;
    const int band = 8; // mỗi dải cao 8px (đủ mượt, ít lệnh vẽ)

    for (int y = 0; y < H; y += band)
    {
        const int t = (y * 100) / H; // 0..100 theo chiều cao
        const uint8_t rr = (uint8_t)(topR + (botR - topR) * t / 100);
        const uint8_t gg = (uint8_t)(topG + (botG - topG) * t / 100);
        const uint8_t bb = (uint8_t)(topB + (botB - topB) * t / 100);

        Rect b(0, y, W, band);
        Rect d = b & invalidatedArea;
        if (!d.isEmpty())
        {
            translateRectToAbsolute(d);
            HAL::lcd().fillRect(d, Color::getColorFromRGB(rr, gg, bb), 255);
        }
    }
}
