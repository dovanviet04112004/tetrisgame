#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <gui/game/BoardWidget.hpp>
#include <gui/game/NextWidget.hpp>
#include <gui/game/MenuBgWidget.hpp>
#include <gui/game/TetrisGame.hpp>
#include <gui/game/TetrisDefs.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <texts/TextKeysAndLanguages.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    // Vòng lặp game (gọi mỗi frame) + nhập bàn phím (simulator)
    virtual void handleTickEvent();
    virtual void handleKeyEvent(uint8_t key);

protected:
    // Trạng thái màn hình + lý do kết thúc
    enum ViewState { VS_MENU, VS_PLAY, VS_END };
    enum EndReason { END_LOSE, END_FINISH, END_TIMEUP };
    static const int MENU_ITEMS = tetris::MODE_COUNT + 1; // 4 chế độ đơn + 1 mục "2P Versus"
    static const int PVP_CELL   = 12;                     // cạnh ô bàn PvP -> 2 bàn 120x240 phủ kín ngang + cao hơn

    BoardWidget        boardWidget;                          // widget vẽ bàn (1 người)
    NextWidget         nextWidget;                            // ô khối kế tiếp (HUD)
    uint8_t            boardCells[tetris::ROWS * tetris::COLS]; // trạng thái 10x20 ô
    tetris::TetrisGame game;                                 // engine luật game (P1)

    // ===== Chế độ 2 người (PvP) =====
    bool               pvp;          // đang ở chế độ 2 người?
    int                pvpWinner;    // 1 = P1 thắng, 2 = P2 thắng (khi END)
    tetris::TetrisGame gameP2;       // engine người chơi 2
    BoardWidget        boardP1;      // bàn PvP của P1 (ô nhỏ)
    BoardWidget        boardP2;      // bàn PvP của P2
    uint8_t            cellsP1[tetris::ROWS * tetris::COLS];
    uint8_t            cellsP2[tetris::ROWS * tetris::COLS];
    touchgfx::Box      frameP1;      // khung viền sau bàn P1
    touchgfx::Box      frameP2;      // khung viền sau bàn P2

    ViewState          state;        // MENU / PLAY / END
    bool               paused;       // chỉ có nghĩa khi đang PLAY
    uint8_t            currentMode;  // chế độ đang chơi (tetris::Mode)
    int                menuCursor;   // mục đang chọn ở Menu (0..MODE_COUNT-1)
    int                endReason;    // lý do kết thúc (EndReason)
    uint32_t           playFrames;   // số frame đã chơi (đồng hồ, ~60/giây)
    int                lastSec;      // giây hiển thị gần nhất (chỉ vẽ lại khi đổi)
    int                endGuardFrames; // khoá nhập ngay sau khi vào END (chống lỡ tay bỏ qua)
    uint32_t           hiScore[tetris::MODE_COUNT]; // điểm cao mỗi chế độ (nạp từ Flash)
    bool               newRecord;    // ván vừa rồi có phá kỷ lục không (đổi màu chữ Best)
    bool               soundOn;      // bật/tắt tiếng (nút HOLD ở Menu; LED xanh/đỏ báo trên board)

    // HUD chữ (font TouchGFX) ở panel phải
    touchgfx::Box                     hudPanel;  // nền panel HUD (đỡ đen, có khung)
    touchgfx::Box                     nextFrame; // khung quanh ô NEXT
    touchgfx::Box                     holdFrame; // khung quanh ô HOLD
    NextWidget                        holdWidget; // ô HOLD (khối đang giữ)
    touchgfx::TextArea                holdLabel;
    touchgfx::TextArea                nextLabel;
    touchgfx::TextAreaWithOneWildcard scoreText;
    touchgfx::TextAreaWithOneWildcard levelText;
    touchgfx::TextAreaWithOneWildcard linesText;
    touchgfx::TextAreaWithOneWildcard timeText;
    touchgfx::TextAreaWithOneWildcard hudBest;   // "Best <value>" hiện ngay khi chơi (1 người)
    touchgfx::Unicode::UnicodeChar    scoreBuf[12];
    touchgfx::Unicode::UnicodeChar    levelBuf[8];
    touchgfx::Unicode::UnicodeChar    linesBuf[8];
    touchgfx::Unicode::UnicodeChar    timeBuf[12];
    touchgfx::Unicode::UnicodeChar    hudBestBuf[12];

    // Lớp phủ Pause / Game Over (phủ vùng bàn) — thêm trước menu
    touchgfx::Box      overlayBox;
    touchgfx::TextArea overlayTitle;  // "Paused" / "Game Over" / "Finish!" / "Time Up"
    touchgfx::TextAreaWithOneWildcard bestText; // "Best <value>" — chỉ hiện khi END
    touchgfx::TextArea overlayHint;   // gợi ý phím (Pause: P/M, End: R/M)
    touchgfx::Unicode::UnicodeChar    bestBuf[12];

    // HUD trên mỗi bàn PvP: nhãn P1/P2 + số dòng + ô NEXT (lấp khoảng dọc dư) + banner
    touchgfx::TextArea p1Label;
    touchgfx::TextArea p2Label;
    touchgfx::TextAreaWithOneWildcard p1Lines; // "Lines <value>" của P1
    touchgfx::TextAreaWithOneWildcard p2Lines; // "Lines <value>" của P2
    touchgfx::Unicode::UnicodeChar    p1LinesBuf[8];
    touchgfx::Unicode::UnicodeChar    p2LinesBuf[8];
    NextWidget         nextP1;     // khối kế tiếp của P1 (ô nhỏ)
    NextWidget         nextP2;     // khối kế tiếp của P2
    NextWidget         holdP1;     // khối P1 đang giữ (HOLD) — cạnh NEXT
    NextWidget         holdP2;     // khối P2 đang giữ (HOLD)
    touchgfx::TextArea nextLblP1;  // nhãn nhỏ "Next"/"Hold" trên 2 ô của mỗi người
    touchgfx::TextArea holdLblP1;
    touchgfx::TextArea nextLblP2;
    touchgfx::TextArea holdLblP2;
    touchgfx::Box      pvpBox;     // dải nền mờ giữa màn cho banner PvP
    touchgfx::TextArea pvpTitle;   // "Paused" / "P1 Wins" / "P2 Wins"
    touchgfx::TextArea pvpHint;    // gợi ý phím PvP

    // Menu chọn chế độ (phủ TOÀN màn hình) — thêm SAU CÙNG nên ở trên cùng
    MenuBgWidget       menuBg;     // nền gradient menu (thay ô màu phẳng)
    touchgfx::Box      menuSel;    // thanh sáng sau mục đang chọn
    touchgfx::TextArea menuTitle;
    touchgfx::TextArea modeText[MENU_ITEMS];
    touchgfx::TextAreaWithOneWildcard menuBest; // "Best <value>" của chế độ đang chọn
    touchgfx::TextArea menuHint;
    touchgfx::Unicode::UnicodeChar    menuBestBuf[12];

    void refresh();              // render bàn + HUD + overlay
    void updateUI();             // bật/tắt mọi overlay/menu theo state
    void updateTimeText();       // cập nhật đồng hồ (đếm lên / đếm ngược Ultra)
    void togglePause();          // bật/tắt tạm dừng (chỉ khi PLAY)
    void toggleSound();          // bật/tắt tiếng (gọi từ Menu)
    void startGame(uint8_t m);   // bắt đầu chơi chế độ m (1 người)
    void startPvp();             // bắt đầu chế độ 2 người
    void showMenu();             // về Menu chọn chế độ
    void endGame(int reason);    // chuyển sang màn kết thúc (1 người)
    void endPvp(int winner);     // kết thúc PvP, hiện người thắng
    void menuMove(int delta);    // di chuyển con trỏ Menu
    void recordHighScore();      // cập nhật + lưu Flash nếu phá kỷ lục chế độ hiện tại
    void quitToMenu();           // ghi điểm cao rồi về Menu (dùng khi Pause -> Menu)
};

#endif // SCREEN1VIEW_HPP
