#ifndef MENUBGWIDGET_HPP
#define MENUBGWIDGET_HPP

#include <touchgfx/widgets/Widget.hpp>

// Nền menu: tô gradient dọc (xanh đậm -> gần đen) bằng các dải ngang.
// Vẽ thủ tục nên KHÔNG cần ảnh/asset, không tốn flash, chạy mọi độ phân giải.
class MenuBgWidget : public touchgfx::Widget
{
public:
    MenuBgWidget();
    virtual ~MenuBgWidget() {}

    virtual void draw(const touchgfx::Rect& invalidatedArea) const;
    virtual touchgfx::Rect getSolidRect() const;
};

#endif // MENUBGWIDGET_HPP
