#include <gui/screen1_screen/Screen1View.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/Drawable.hpp>

#ifndef SIMULATOR
// Trên board: nhận lệnh nút/joystick từ InputTask (defaultTask trong main.c) qua hàng đợi
#include "cmsis_os.h"
extern "C" {
extern osMessageQueueId_t inputQueueHandle;
void set_music_paused(uint8_t paused);     // cầu nối tới AudioTask (main.c)
void hiscore_load(uint32_t* out4);         // đọc 4 điểm cao từ Flash (main.c)
void hiscore_save(const uint32_t* in4);    // ghi 4 điểm cao vào Flash (main.c)
void set_pvp_mode(uint8_t on);             // báo InputTask tách nút (P1) / joystick (P2)
void set_sound_on(uint8_t on);             // bật/tắt tiếng + LED báo (xanh/đỏ)
}
#endif

// Bật/tắt 1 widget đúng quy chuẩn TouchGFX: invalidate vùng cũ (khi còn visible)
// -> đổi visible -> invalidate vùng mới (khi đã visible). invalidate() là no-op
// nếu widget đang ẩn, nên gọi 2 lần bao quanh setVisible() xử lý đúng cả 2 chiều.
static void setVis(touchgfx::Drawable& d, bool v)
{
    d.invalidate();
    d.setVisible(v);
    d.invalidate();
}

Screen1View::Screen1View()
    : pvp(false), pvpWinner(0),
      state(VS_MENU), paused(false), currentMode(tetris::MODE_MARATHON),
      menuCursor(0), endReason(END_LOSE), playFrames(0), lastSec(0), endGuardFrames(0),
      newRecord(false)
{
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    state = VS_MENU;
    paused = false;
    currentMode = tetris::MODE_MARATHON;
    menuCursor = 0;
    endReason = END_LOSE;
    playFrames = 0;
    lastSec = 0;
    endGuardFrames = 0;
    newRecord = false;
    pvp = false;
    pvpWinner = 0;
    soundOn = true;
#ifndef SIMULATOR
    set_sound_on(1); // đồng bộ LED báo tiếng lúc khởi động (xanh = bật)
#endif

    // Điểm cao: nạp từ Flash trên board; simulator giữ trong RAM (reset mỗi phiên)
    for (int i = 0; i < tetris::MODE_COUNT; i++)
        hiScore[i] = 0;
#ifndef SIMULATOR
    hiscore_load(hiScore);
#endif

    game.start(tetris::MODE_MARATHON); // dọn bàn (ẩn sau menu lúc khởi động)

    // ===== Bố cục 1 người: bàn lấp KÍN chiều cao 320 (ô 16px) + HUD dọc bên phải =====
    const int SCELL = 16;                   // ô 16px -> bàn 160 x 320 (kín cao, khối to)
    const int boardW = tetris::COLS * SCELL; // 160
    const int boardH = tetris::ROWS * SCELL; // 320
    const int hudX = boardW + 6;            // 166
    const int hudW = 240 - hudX;            // 74

    boardWidget.setCells(boardCells);
    boardWidget.setCell(SCELL);
    boardWidget.setXY(0, 0);                // sát mép trái-trên, hết viền đen dọc
    add(boardWidget);

    // Nền panel HUD (xanh đậm, không phải đen) phủ kín cột phải 160..240
    hudPanel.setPosition(boardW, 0, 240 - boardW, 320);
    hudPanel.setColor(touchgfx::Color::getColorFromRGB(16, 20, 40));
    add(hudPanel);

    // Panel HUD bên phải — NEXT + HOLD đóng khung ở trên, số liệu nhiều màu ở dưới
    const int frameX = hudX + (hudW - 48) / 2; // canh giữa khung 48px trong cột HUD

    nextLabel.setPosition(hudX, 4, hudW, 12);
    nextLabel.setColor(touchgfx::Color::getColorFromRGB(150, 205, 255));
    nextLabel.setTypedText(touchgfx::TypedText(T_TETRISNEXT));
    add(nextLabel);

    nextFrame.setPosition(frameX, 18, 48, 48); // khung sáng quanh ô NEXT
    nextFrame.setColor(touchgfx::Color::getColorFromRGB(70, 120, 190));
    add(nextFrame);
    nextWidget.setCell(11);                    // ô NEXT 4*11 = 44px (chừa viền 2px)
    nextWidget.setXY(frameX + 2, 20);
    add(nextWidget);

    holdLabel.setPosition(hudX, 70, hudW, 12);
    holdLabel.setColor(touchgfx::Color::getColorFromRGB(150, 205, 255));
    holdLabel.setTypedText(touchgfx::TypedText(T_TETRISHOLD));
    add(holdLabel);

    holdFrame.setPosition(frameX, 84, 48, 48);
    holdFrame.setColor(touchgfx::Color::getColorFromRGB(70, 120, 190));
    add(holdFrame);
    holdWidget.setCell(11);
    holdWidget.setXY(frameX + 2, 86);
    add(holdWidget);

    scoreText.setPosition(hudX, 144, hudW, 14);
    scoreText.setColor(touchgfx::Color::getColorFromRGB(120, 210, 255)); // cyan
    scoreText.setTypedText(touchgfx::TypedText(T_TETRISSCORE));
    scoreText.setWildcard(scoreBuf);
    add(scoreText);

    levelText.setPosition(hudX, 174, hudW, 14);
    levelText.setColor(touchgfx::Color::getColorFromRGB(130, 230, 150)); // xanh lá
    levelText.setTypedText(touchgfx::TypedText(T_TETRISLEVEL));
    levelText.setWildcard(levelBuf);
    add(levelText);

    linesText.setPosition(hudX, 204, hudW, 14);
    linesText.setColor(touchgfx::Color::getColorFromRGB(245, 220, 120)); // vàng
    linesText.setTypedText(touchgfx::TypedText(T_TETRISLINES));
    linesText.setWildcard(linesBuf);
    add(linesText);

    timeText.setPosition(hudX, 234, hudW, 14);
    timeText.setColor(touchgfx::Color::getColorFromRGB(200, 180, 255)); // tím nhạt
    timeText.setTypedText(touchgfx::TypedText(T_TETRISTIME));
    timeText.setWildcard(timeBuf);
    add(timeText);

    hudBest.setPosition(hudX, 264, hudW, 14);
    hudBest.setColor(touchgfx::Color::getColorFromRGB(255, 220, 90));
    hudBest.setTypedText(touchgfx::TypedText(T_TETRISBEST));
    hudBest.setWildcard(hudBestBuf);
    add(hudBest);

    // ===== Lớp phủ Pause / Game Over: phủ vùng bàn (HUD vẫn đọc được) =====
    overlayBox.setPosition(0, 0, boardW, boardH);
    overlayBox.setColor(touchgfx::Color::getColorFromRGB(8, 10, 20));
    overlayBox.setAlpha(200); // mờ ~78% để vẫn thấy lờ mờ bàn phía sau
    overlayBox.setVisible(false);
    add(overlayBox);

    overlayTitle.setPosition(0, boardH / 2 - 44, boardW, 30);
    overlayTitle.setColor(touchgfx::Color::getColorFromRGB(255, 220, 90));
    overlayTitle.setVisible(false);
    add(overlayTitle);

    bestText.setPosition(0, boardH / 2 - 8, boardW, 16);
    bestText.setColor(touchgfx::Color::getColorFromRGB(200, 204, 220));
    bestText.setTypedText(touchgfx::TypedText(T_TETRISBEST));
    bestText.setWildcard(bestBuf);
    bestText.setVisible(false);
    add(bestText);

    overlayHint.setPosition(0, boardH / 2 + 18, boardW, 16);
    overlayHint.setColor(touchgfx::Color::getColorFromRGB(210, 214, 230));
    overlayHint.setTypedText(touchgfx::TypedText(T_TETRISHINT));
    overlayHint.setVisible(false);
    add(overlayHint);

    // ===== Bàn + HUD chế độ 2 người (PvP): ô 12px -> 2 bàn 120x240 phủ KÍN ngang & cao =====
    // HUD mỗi người ở trên (nhãn + số dòng + NEXT, canh giữa cột 120px), bàn lấp gần hết dọc.
    // frameP1 = nền/khung quanh vùng 2 bàn (lộ viền 2px trên-dưới); frameP2 = vạch ngăn giữa.
    frameP1.setPosition(0, 68, 240, 244);
    frameP1.setColor(touchgfx::Color::getColorFromRGB(40, 70, 120));
    frameP1.setVisible(false);
    add(frameP1);

    boardP1.setCells(cellsP1);
    boardP1.setCell(PVP_CELL);
    boardP1.setXY(0, 70);    // 120x240 -> x 0..120, y 70..310
    boardP1.setVisible(false);
    add(boardP1);

    boardP2.setCells(cellsP2);
    boardP2.setCell(PVP_CELL);
    boardP2.setXY(120, 70);  // 120x240 -> x 120..240
    boardP2.setVisible(false);
    add(boardP2);

    frameP2.setPosition(119, 68, 2, 244);    // vạch ngăn giữa (vẽ đè lên mép 2 bàn)
    frameP2.setColor(touchgfx::Color::getColorFromRGB(90, 140, 210));
    frameP2.setVisible(false);
    add(frameP2);

    p1Label.setPosition(0, 4, 120, 14);
    p1Label.setColor(touchgfx::Color::getColorFromRGB(120, 210, 255));
    p1Label.setTypedText(touchgfx::TypedText(T_TETRISP1));
    p1Label.setVisible(false);
    add(p1Label);

    p2Label.setPosition(120, 4, 120, 14);
    p2Label.setColor(touchgfx::Color::getColorFromRGB(255, 180, 120));
    p2Label.setTypedText(touchgfx::TypedText(T_TETRISP2));
    p2Label.setVisible(false);
    add(p2Label);

    p1Lines.setPosition(0, 18, 120, 12);
    p1Lines.setColor(touchgfx::Color::getColorFromRGB(120, 210, 255)); // cyan ~ P1
    p1Lines.setTypedText(touchgfx::TypedText(T_TETRISLINESC)); // canh giữa
    p1Lines.setWildcard(p1LinesBuf);
    p1Lines.setVisible(false);
    add(p1Lines);

    p2Lines.setPosition(120, 18, 120, 12);
    p2Lines.setColor(touchgfx::Color::getColorFromRGB(255, 180, 120)); // cam ~ P2
    p2Lines.setTypedText(touchgfx::TypedText(T_TETRISLINESC)); // canh giữa
    p2Lines.setWildcard(p2LinesBuf);
    p2Lines.setVisible(false);
    add(p2Lines);

    // Mỗi người: 2 ô nhỏ cạnh nhau (TRÁI = Next, PHẢI = Hold) + nhãn 10px phía trên.
    // Ô 4x6 = 24px; canh giữa cặp ô trong cột 120px (x 32 và 64; P2 cộng 120).
    nextLblP1.setPosition(32, 31, 32, 11);
    nextLblP1.setColor(touchgfx::Color::getColorFromRGB(150, 205, 255));
    nextLblP1.setTypedText(touchgfx::TypedText(T_TETRISNEXT));
    nextLblP1.setVisible(false);
    add(nextLblP1);

    holdLblP1.setPosition(64, 31, 32, 11);
    holdLblP1.setColor(touchgfx::Color::getColorFromRGB(150, 205, 255));
    holdLblP1.setTypedText(touchgfx::TypedText(T_TETRISHOLD));
    holdLblP1.setVisible(false);
    add(holdLblP1);

    nextP1.setCell(6);
    nextP1.setXY(32, 42);
    nextP1.setVisible(false);
    add(nextP1);

    holdP1.setCell(6);
    holdP1.setXY(64, 42);
    holdP1.setVisible(false);
    add(holdP1);

    nextLblP2.setPosition(152, 31, 32, 11);
    nextLblP2.setColor(touchgfx::Color::getColorFromRGB(255, 200, 150));
    nextLblP2.setTypedText(touchgfx::TypedText(T_TETRISNEXT));
    nextLblP2.setVisible(false);
    add(nextLblP2);

    holdLblP2.setPosition(184, 31, 32, 11);
    holdLblP2.setColor(touchgfx::Color::getColorFromRGB(255, 200, 150));
    holdLblP2.setTypedText(touchgfx::TypedText(T_TETRISHOLD));
    holdLblP2.setVisible(false);
    add(holdLblP2);

    nextP2.setCell(6);
    nextP2.setXY(152, 42);
    nextP2.setVisible(false);
    add(nextP2);

    holdP2.setCell(6);
    holdP2.setXY(184, 42);
    holdP2.setVisible(false);
    add(holdP2);

    // Banner giữa vùng bàn cho PvP (Paused / P1 Wins / P2 Wins)
    pvpBox.setPosition(0, 150, 240, 60);
    pvpBox.setColor(touchgfx::Color::getColorFromRGB(8, 10, 20));
    pvpBox.setAlpha(210);
    pvpBox.setVisible(false);
    add(pvpBox);

    pvpTitle.setPosition(0, 158, 240, 30);
    pvpTitle.setColor(touchgfx::Color::getColorFromRGB(255, 220, 90));
    pvpTitle.setVisible(false);
    add(pvpTitle);

    pvpHint.setPosition(0, 194, 240, 16);
    pvpHint.setColor(touchgfx::Color::getColorFromRGB(210, 214, 230));
    pvpHint.setTypedText(touchgfx::TypedText(T_TETRISHINT));
    pvpHint.setVisible(false);
    add(pvpHint);

    // ===== Menu: nền gradient + thanh sáng chọn + tiêu đề lớn (phủ TOÀN màn hình) =====
    menuBg.setPosition(0, 0, 240, 320);
    menuBg.setVisible(false);
    add(menuBg);

    menuSel.setPosition(18, 90, 204, 28); // Y cập nhật theo mục đang chọn trong updateUI
    menuSel.setColor(touchgfx::Color::getColorFromRGB(60, 110, 180));
    menuSel.setAlpha(150);                // mờ -> ánh gradient xuyên qua
    menuSel.setVisible(false);
    add(menuSel);

    menuTitle.setPosition(0, 30, 240, 46); // font Large (40px)
    menuTitle.setColor(touchgfx::Color::getColorFromRGB(120, 210, 255));
    menuTitle.setTypedText(touchgfx::TypedText(T_TETRISTITLE));
    menuTitle.setVisible(false);
    add(menuTitle);

    const touchgfx::TypedText modeTT[MENU_ITEMS] = {
        touchgfx::TypedText(T_TETRISMARATHON),
        touchgfx::TypedText(T_TETRISSPRINT),
        touchgfx::TypedText(T_TETRISULTRA),
        touchgfx::TypedText(T_TETRISZEN),
        touchgfx::TypedText(T_TETRISVERSUS) // mục 4 = 2 người
    };
    for (int i = 0; i < MENU_ITEMS; i++)
    {
        modeText[i].setPosition(0, 92 + i * 30, 240, 26);
        modeText[i].setColor(touchgfx::Color::getColorFromRGB(120, 124, 140));
        modeText[i].setTypedText(modeTT[i]);
        modeText[i].setVisible(false);
        add(modeText[i]);
    }

    menuBest.setPosition(0, 244, 240, 16);
    menuBest.setColor(touchgfx::Color::getColorFromRGB(255, 220, 90));
    menuBest.setTypedText(touchgfx::TypedText(T_TETRISBEST));
    menuBest.setWildcard(menuBestBuf);
    menuBest.setVisible(false);
    add(menuBest);

    menuHint.setPosition(0, 276, 240, 16);
    menuHint.setColor(touchgfx::Color::getColorFromRGB(150, 154, 170));
    menuHint.setTypedText(touchgfx::TypedText(T_TETRISMENUHINT));
    menuHint.setVisible(false);
    add(menuHint);

    refresh();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::updateTimeText()
{
    uint32_t sec;
    if (currentMode == tetris::MODE_ULTRA)
    {
        const uint32_t el = playFrames / 60;       // Ultra: 2:00 đếm ngược
        sec = (el >= 120) ? 0u : (120u - el);
    }
    else
    {
        sec = playFrames / 60;                     // còn lại: đếm lên
    }
    touchgfx::Unicode::snprintf(timeBuf, 12, "%u:%02u",
                                (unsigned int)(sec / 60), (unsigned int)(sec % 60));
    timeText.invalidate();
}

void Screen1View::refresh()
{
    if (pvp)
    {
        game.renderTo(cellsP1);   boardP1.invalidate();   // P1
        gameP2.renderTo(cellsP2); boardP2.invalidate();   // P2
        nextP1.setPiece(game.getNextType());   nextP1.invalidate();
        nextP2.setPiece(gameP2.getNextType()); nextP2.invalidate();
        holdP1.setPiece(game.getHoldType());    holdP1.invalidate(); // -1 = ô HOLD trống
        holdP2.setPiece(gameP2.getHoldType());  holdP2.invalidate();
        touchgfx::Unicode::snprintf(p1LinesBuf, 8, "%u", (unsigned int)game.getLines());
        p1Lines.invalidate();
        touchgfx::Unicode::snprintf(p2LinesBuf, 8, "%u", (unsigned int)gameP2.getLines());
        p2Lines.invalidate();
        updateUI();
        return;
    }

    game.renderTo(boardCells);
    boardWidget.invalidate();

    nextWidget.setPiece(game.getNextType());
    nextWidget.invalidate();
    holdWidget.setPiece(game.getHoldType()); // -1 = ô HOLD trống
    holdWidget.invalidate();

    touchgfx::Unicode::snprintf(scoreBuf, 12, "%u", (unsigned int)game.getScore());
    scoreText.invalidate();
    touchgfx::Unicode::snprintf(levelBuf, 8, "%u", (unsigned int)game.getLevel());
    levelText.invalidate();
    touchgfx::Unicode::snprintf(linesBuf, 8, "%u", (unsigned int)game.getLines());
    linesText.invalidate();
    touchgfx::Unicode::snprintf(hudBestBuf, 12, "%u", (unsigned int)hiScore[currentMode]);
    hudBest.invalidate();
    updateTimeText();

    updateUI();
}

void Screen1View::updateUI()
{
    const bool menu  = (state == VS_MENU);
    const bool end   = (state == VS_END);
    const bool pause = (state == VS_PLAY && paused);
    const bool scrim = end || pause;

    const bool single      = !pvp;                       // widget 1 người (ẩn khi PvP)
    const bool singleScrim = scrim && !pvp;              // overlay 1 người (Pause/End)
    const bool pvpScrim    = scrim && pvp;               // banner PvP (Pause/Wins)
    const bool pvpPlaying  = pvp && (state != VS_MENU);  // 2 bàn hiện khi PvP & ngoài menu

    // Nội dung overlay 1 người (đặt TRƯỚC khi hiện)
    if (singleScrim)
    {
        if (end)
        {
            const TEXTS t = (endReason == END_FINISH) ? T_TETRISFINISH
                          : (endReason == END_TIMEUP) ? T_TETRISTIMEUP
                                                      : T_TETRISGAMEOVER;
            overlayTitle.setTypedText(touchgfx::TypedText(t));
            touchgfx::Unicode::snprintf(bestBuf, 12, "%u", (unsigned int)hiScore[currentMode]);
            bestText.setColor(newRecord
                ? touchgfx::Color::getColorFromRGB(255, 220, 90)
                : touchgfx::Color::getColorFromRGB(200, 204, 220));
            overlayHint.setTypedText(touchgfx::TypedText(T_TETRISHINT));
        }
        else // pause
        {
            overlayTitle.setTypedText(touchgfx::TypedText(T_TETRISPAUSE));
            overlayHint.setTypedText(touchgfx::TypedText(T_TETRISPAUSEHINT));
        }
    }

    // Nội dung banner PvP
    if (pvpScrim)
    {
        if (end)
        {
            pvpTitle.setTypedText(touchgfx::TypedText(pvpWinner == 1 ? T_TETRISP1WIN : T_TETRISP2WIN));
            pvpHint.setTypedText(touchgfx::TypedText(T_TETRISHINT));      // Hold: Retry  Rotate: Menu
        }
        else // pause
        {
            pvpTitle.setTypedText(touchgfx::TypedText(T_TETRISPAUSE));
            pvpHint.setTypedText(touchgfx::TypedText(T_TETRISPAUSEHINT));
        }
    }

    // Màu mục menu + điểm cao mục đang chọn (PvP không có điểm cao)
    if (menu)
    {
        for (int i = 0; i < MENU_ITEMS; i++)
            modeText[i].setColor(i == menuCursor
                ? touchgfx::Color::getColorFromRGB(255, 220, 90)
                : touchgfx::Color::getColorFromRGB(120, 124, 140));
        if (menuCursor < tetris::MODE_COUNT)
            touchgfx::Unicode::snprintf(menuBestBuf, 12, "%u", (unsigned int)hiScore[menuCursor]);
    }

    // Widget 1 người (bàn + HUD): ẩn hẳn khi PvP
    setVis(hudPanel,    single);
    setVis(boardWidget, single);
    setVis(nextFrame,   single);
    setVis(nextWidget,  single);
    setVis(nextLabel,   single);
    setVis(holdFrame,   single);
    setVis(holdWidget,  single);
    setVis(holdLabel,   single);
    setVis(scoreText,   single);
    setVis(levelText,   single);
    setVis(linesText,   single);
    setVis(timeText,    single);
    setVis(hudBest,     single);

    // Overlay 1 người
    setVis(overlayBox,   singleScrim);
    setVis(overlayTitle, singleScrim);
    setVis(bestText,     end && !pvp);
    setVis(overlayHint,  singleScrim);

    // Bàn + HUD PvP (khung + nhãn + số dòng + NEXT)
    setVis(frameP1, pvpPlaying);
    setVis(frameP2, pvpPlaying);
    setVis(boardP1, pvpPlaying);
    setVis(boardP2, pvpPlaying);
    setVis(p1Label, pvpPlaying);
    setVis(p2Label, pvpPlaying);
    setVis(p1Lines, pvpPlaying);
    setVis(p2Lines, pvpPlaying);
    setVis(nextP1,  pvpPlaying);
    setVis(nextP2,  pvpPlaying);
    setVis(holdP1,  pvpPlaying);
    setVis(holdP2,  pvpPlaying);
    setVis(nextLblP1, pvpPlaying);
    setVis(holdLblP1, pvpPlaying);
    setVis(nextLblP2, pvpPlaying);
    setVis(holdLblP2, pvpPlaying);

    // Banner PvP
    setVis(pvpBox,   pvpScrim);
    setVis(pvpTitle, pvpScrim);
    setVis(pvpHint,  pvpScrim);

    // Menu
    setVis(menuBg, menu);

    // Thanh sáng chọn: invalidate vị trí cũ -> dời tới mục đang chọn -> invalidate vị trí mới
    menuSel.invalidate();
    if (menu)
        menuSel.setY(92 + menuCursor * 30 - 2);
    menuSel.setVisible(menu);
    menuSel.invalidate();

    setVis(menuTitle, menu);
    for (int i = 0; i < MENU_ITEMS; i++)
        setVis(modeText[i], menu);
    setVis(menuBest,  menu && (menuCursor < tetris::MODE_COUNT));
    setVis(menuHint,  menu);
}

void Screen1View::startGame(uint8_t m)
{
    currentMode = m;
    paused = false;
    state = VS_PLAY;
    playFrames = 0;
    lastSec = 0;
    game.start((tetris::Mode)m);
#ifndef SIMULATOR
    set_music_paused(0);
#endif
    refresh();
}

void Screen1View::startPvp()
{
    pvp = true;
    pvpWinner = 0;
    paused = false;
    state = VS_PLAY;
    playFrames = 0;
    lastSec = 0;
    game.start(tetris::MODE_MARATHON);   // P1
    gameP2.start(tetris::MODE_MARATHON); // P2
#ifndef SIMULATOR
    set_music_paused(0);
    set_pvp_mode(1); // báo InputTask: nút = P1, joystick = P2
#endif
    refresh();
}

void Screen1View::endPvp(int winner)
{
    pvpWinner = winner; // 1 = P1 thắng, 2 = P2 thắng
    state = VS_END;
    endGuardFrames = 20;
#ifndef SIMULATOR
    set_music_paused(0);
#endif
    refresh(); // 2 bàn đóng băng + banner người thắng
}

void Screen1View::showMenu()
{
    state = VS_MENU;
    paused = false;
    pvp = false;
#ifndef SIMULATOR
    set_music_paused(0); // menu vẫn phát nhạc nền
    set_pvp_mode(0);     // về điều khiển 1 người (gộp nút + joystick)
#endif
    refresh();
}

void Screen1View::endGame(int reason)
{
    endReason = reason;
    state = VS_END;
    endGuardFrames = 20; // ~0.3s: chặn phím lỡ tay (vd Space đang auto-repeat) bỏ qua màn END
    recordHighScore();   // cập nhật + lưu Flash nếu phá kỷ lục (đặt TRƯỚC refresh)
#ifndef SIMULATOR
    set_music_paused(0);
#endif
    refresh();
}

void Screen1View::recordHighScore()
{
    const uint32_t s = game.getScore();
    newRecord = (s > hiScore[currentMode]);
    if (newRecord)
    {
        hiScore[currentMode] = s;
#ifndef SIMULATOR
        hiscore_save(hiScore); // xoá+ghi sector 23 (bank 2 -> không treo màn hình)
#endif
    }
}

void Screen1View::quitToMenu()
{
    if (!pvp) recordHighScore(); // PvP không có điểm cao; chỉ chốt cho chế độ đơn
    showMenu();
}

void Screen1View::togglePause()
{
    paused = !paused; // chỉ gọi khi đang PLAY
#ifndef SIMULATOR
    set_music_paused(paused ? 1 : 0); // tạm dừng/khôi phục nhạc nền buzzer
#endif
    updateUI();
}

void Screen1View::toggleSound()
{
    soundOn = !soundOn; // gọi từ Menu (nút HOLD / phím X)
#ifndef SIMULATOR
    set_sound_on(soundOn ? 1 : 0); // bật/tắt nhạc+SFX, đổi LED xanh/đỏ
#endif
}

void Screen1View::menuMove(int delta)
{
    menuCursor = (menuCursor + delta + MENU_ITEMS) % MENU_ITEMS;
    updateUI();
}

void Screen1View::handleTickEvent()
{
    if (endGuardFrames > 0)
        endGuardFrames--; // đếm lùi cửa sổ khoá nhập sau khi vào END

    bool changed = false;

#ifndef SIMULATOR
    // Rút hàng đợi phần cứng mỗi frame; xử lý theo trạng thái. Sau một hành động
    // ĐỔI MÀN (start/menu) thì dừng rút phần còn lại -> xử lý ở frame sau khi
    // trạng thái đã ổn định (tránh lệnh cũ trong hàng đợi "lọt" vào màn mới).
    uint8_t code;
    while (inputQueueHandle != NULL &&
           osMessageQueueGet(inputQueueHandle, &code, NULL, 0) == osOK)
    {
        bool stateChanged = false;

        if (code == tetris::CMD_PAUSE_CODE)
        {
            if (state == VS_PLAY)                             { togglePause(); }
            else if (state == VS_END && endGuardFrames == 0)  { showMenu(); stateChanged = true; }
        }
        else if (state == VS_MENU)
        {
            if (code == tetris::CMD_HARD_DROP)      menuMove(-1);            // joystick lên
            else if (code == tetris::CMD_SOFT_DROP) menuMove(+1);            // joystick xuống
            else if (code == tetris::CMD_ROT_CW)                             // SW = chọn
            {
                if (menuCursor == tetris::MODE_COUNT) startPvp();            // mục cuối = 2 người
                else startGame((uint8_t)menuCursor);
                stateChanged = true;
            }
            else if (code == tetris::CMD_RESTART_CODE) toggleSound();        // nút HOLD ở Menu = bật/tắt tiếng
        }
        else if (state == VS_PLAY)
        {
            if (paused)
            {
                // Đang Pause: nút Xoay (SW) = thoát về Menu; nút PAUSE (ở trên) = tiếp tục
                if (code == tetris::CMD_ROT_CW) { quitToMenu(); stateChanged = true; }
            }
            else if (!pvp && code == tetris::CMD_RESTART_CODE)
            {
                game.hold(); // nút HOLD lúc đang chơi = giữ khối (chỉ chế độ 1 người)
                changed = true;
            }
            else if (pvp)
            {
                // PvP: mã 0..5 = P1 (nút), mã 16..21 = P2 (joystick)
                //      mã 7 = HOLD P1 (nút PD5), mã 8 = HOLD P2 (nút PB7)
                if (code <= tetris::CMD_HARD_DROP)
                {
                    game.command((tetris::Command)code);
                    changed = true;
                }
                else if (code == tetris::CMD_RESTART_CODE) // nút HOLD P1
                {
                    game.hold();
                    changed = true;
                }
                else if (code == tetris::CMD_HOLD_P2_CODE)  // nút HOLD P2
                {
                    gameP2.hold();
                    changed = true;
                }
                else if (code >= 16 && code <= 16 + tetris::CMD_HARD_DROP)
                {
                    gameP2.command((tetris::Command)(code - 16));
                    changed = true;
                }
            }
            else if (code <= tetris::CMD_HARD_DROP)
            {
                game.command((tetris::Command)code);
                changed = true;
            }
        }
        else // VS_END: Xoay (SW) = về Menu, HOLD = chơi lại. Bỏ nút thả để tránh lỡ tay.
        {
            if (endGuardFrames == 0 &&
                (code == tetris::CMD_ROT_CW || code == 16 + tetris::CMD_ROT_CW)) // Xoay về Menu: P1 (nút PC11) HOẶC P2 (joystick SW = mã 18)
            {
                showMenu();
                stateChanged = true;
            }
            else if (endGuardFrames == 0 &&
                     (code == tetris::CMD_RESTART_CODE || code == tetris::CMD_HOLD_P2_CODE))
            {
                if (pvp) startPvp(); else startGame(currentMode); // chơi lại (HOLD P1 hoặc P2)
                stateChanged = true;
            }
        }

        if (stateChanged)
            break;
    }
#endif

    if (state == VS_PLAY && !paused)
    {
        playFrames++;

        if (pvp)
        {
            bool ch = changed;
            if (game.onFrameTick())   ch = true; // trọng lực + lock delay tự đếm bên trong
            if (gameP2.onFrameTick()) ch = true;

            const bool over1 = game.isGameOver();   // P1 đầy bàn
            const bool over2 = gameP2.isGameOver();  // P2 đầy bàn
            if (over1 || over2)
            {
                int winner;
                if (over1 && over2)
                    winner = (game.getLines() >= gameP2.getLines()) ? 1 : 2; // hoà cùng frame -> ai xoá nhiều hàng hơn thắng (bằng nhau -> P1)
                else
                    winner = over1 ? 2 : 1; // ai đầy bàn thì người kia thắng
                endPvp(winner);
                return;
            }
            if (ch) refresh();
            return;
        }

        // Trọng lực theo level (Zen: tốc độ nhẹ cố định) + lock delay ~0,5 s
        // (khối chạm đáy chưa khoá ngay, di chuyển/xoay còn hoãn được — xem TetrisGame::onFrameTick)
        if (game.onFrameTick())
            changed = true;

        // Kiểm tra THẮNG / HẾT GIỜ TRƯỚC khi kiểm tra THUA: một cú khoá vừa đủ
        // 40 hàng (Sprint) hoặc đúng mốc 2:00 (Ultra) phải tính HOÀN THÀNH, không
        // bị báo nhầm "Game Over" dù khối kế tiếp spawn bị kẹt.
        if (currentMode == tetris::MODE_SPRINT && game.getLines() >= 40)   { endGame(END_FINISH); return; }
        if (currentMode == tetris::MODE_ULTRA  && playFrames >= 120u * 60u) { endGame(END_TIMEUP); return; }
        if (game.isGameOver())                                             { endGame(END_LOSE);   return; }

        const int sec = (int)(playFrames / 60);
        if (changed || sec != lastSec)
        {
            lastSec = sec;
            refresh();
        }
    }
}

void Screen1View::handleKeyEvent(uint8_t key)
{
    using namespace tetris;

    // ----- MENU: chọn chế độ -----
    if (state == VS_MENU)
    {
        if (key == 'w' || key == 'W')      menuMove(-1);
        else if (key == 's' || key == 'S') menuMove(+1);
        else if (key == ' ' || key == '\r' || key == '\n')
        {
            if (menuCursor == tetris::MODE_COUNT) startPvp();      // mục cuối = 2 người
            else startGame((uint8_t)menuCursor);
        }
        else if (key == 'x' || key == 'X') toggleSound();         // bật/tắt tiếng (test sim)
        return;
    }

    // ----- END: chơi lại (R) hoặc về menu (M/Enter) -----
    // KHÔNG dùng Space ở đây: Space là phím thả nhanh lúc chơi, sẽ auto-repeat
    // và nhảy vọt qua màn END. Thêm guard ~0.3s chặn phím lỡ tay ngay sau khi kết thúc.
    if (state == VS_END)
    {
        if (endGuardFrames > 0) return;
        if (key == 'r' || key == 'R')                                    { if (pvp) startPvp(); else startGame(currentMode); } // chơi lại
        else if (key == 'm' || key == 'M' || key == '\r' || key == '\n') showMenu();             // về menu
        return;
    }

    // ----- PLAY -----
    if (key == 'p' || key == 'P') { togglePause(); return; }
    if (key == 'r' || key == 'R') { if (pvp) startPvp(); else startGame(currentMode); return; } // restart nhanh
    if (paused)
    {
        if (key == 'm' || key == 'M') quitToMenu(); // đang Pause: M = thoát về Menu
        return;
    }

    if (pvp)
    {
        // Simulator test 2 người: P1 = WASD + Q(CCW) + Space(thả) + C(hold)
        //                         P2 = IJKL + U(thả) + O(hold) + ;(xoay ngược)
        switch (key)
        {
        case 'a': case 'A': game.command(CMD_LEFT);        break;
        case 'd': case 'D': game.command(CMD_RIGHT);       break;
        case 'w': case 'W': game.command(CMD_ROT_CW);      break;
        case 'q': case 'Q': game.command(CMD_ROT_CCW);     break;
        case 's': case 'S': game.command(CMD_SOFT_DROP);   break;
        case ' ':           game.command(CMD_HARD_DROP);   break;
        case 'c': case 'C': game.hold();                   break; // HOLD P1
        case 'j': case 'J': gameP2.command(CMD_LEFT);      break;
        case 'l': case 'L': gameP2.command(CMD_RIGHT);     break;
        case 'i': case 'I': gameP2.command(CMD_ROT_CW);    break;
        case ';':           gameP2.command(CMD_ROT_CCW);   break;
        case 'k': case 'K': gameP2.command(CMD_SOFT_DROP); break;
        case 'u': case 'U': gameP2.command(CMD_HARD_DROP); break;
        case 'o': case 'O': gameP2.hold();                 break; // HOLD P2
        default: return;
        }
        refresh();
        return;
    }

    switch (key)
    {
    case 'a': case 'A': game.command(CMD_LEFT);      break;
    case 'd': case 'D': game.command(CMD_RIGHT);     break;
    case 'w': case 'W': game.command(CMD_ROT_CW);    break;
    case 'q': case 'Q': game.command(CMD_ROT_CCW);   break;
    case 's': case 'S': game.command(CMD_SOFT_DROP); break;
    case 'e': case 'E': case ' ': game.command(CMD_HARD_DROP); break;
    case 'c': case 'C': game.hold(); break; // giữ khối
    default: return; // phím khác: bỏ qua, không vẽ lại
    }
    refresh();
}
