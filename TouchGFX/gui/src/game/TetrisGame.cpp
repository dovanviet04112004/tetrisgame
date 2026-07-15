#include <gui/game/TetrisGame.hpp>

#ifndef SIMULATOR
// Hàm cầu nối định nghĩa trong Core/Src/main.c (chỉ build cho board):
extern "C" uint32_t hw_rand(void);      // RNG phần cứng (Yêu cầu ①)
extern "C" void sound_emit(uint8_t id); // đẩy sự kiện âm vào soundQueue (Yêu cầu ②)
#endif

namespace tetris
{

// ===== Lock delay (Yêu cầu nâng cao — chuẩn Tetris Guideline) =====
static const uint8_t LOCK_DELAY_FRAMES = 30; // ~0,5 s @ 60fps: khoảng trễ trước khi khoá khối chạm đáy
static const uint8_t LOCK_MAX_RESETS   = 15; // tối đa 15 lần hoãn khoá do di chuyển/xoay (chống treo khối vô hạn)

// ===== Wall-kick SRS (Super Rotation System) =====
// Mỗi lần xoay thử lần lượt 5 phép dịch (dx, dy); lọt vị trí nào thì nhận vị trí đó.
// Bảng ghi theo chuẩn SRS với dy DƯƠNG = LÊN — khi áp vào bàn phải ĐẢO DẤU dy vì
// trục y của bàn hướng XUỐNG. Chỉ lưu chiều CW từ trạng thái s (hàng s = s -> s+1);
// chiều CCW từ s về s-1 = ĐẢO DẤU hàng (s-1) (tính đối xứng của SRS: kick A->B = -kick B->A).
static const int8_t KICK_JLSTZ_CW[4][5][2] = {
    { {0,0}, {-1,0}, {-1, 1}, {0,-2}, {-1,-2} }, // 0 -> R
    { {0,0}, { 1,0}, { 1,-1}, {0, 2}, { 1, 2} }, // R -> 2
    { {0,0}, { 1,0}, { 1, 1}, {0,-2}, { 1,-2} }, // 2 -> L
    { {0,0}, {-1,0}, {-1,-1}, {0, 2}, {-1, 2} }, // L -> 0
};
static const int8_t KICK_I_CW[4][5][2] = {
    { {0,0}, {-2,0}, { 1,0}, {-2,-1}, { 1, 2} }, // 0 -> R
    { {0,0}, {-1,0}, { 2,0}, {-1, 2}, { 2,-1} }, // R -> 2
    { {0,0}, { 2,0}, {-1,0}, { 2, 1}, {-1,-2} }, // 2 -> L
    { {0,0}, { 1,0}, {-2,0}, { 1,-2}, {-2, 1} }, // L -> 0
};

TetrisGame::TetrisGame()
    : curType(0), curRot(0), curX(0), curY(0), gameOver(true), lines(0), score(0), level(1), mode(MODE_MARATHON), rngState(0x2545F491u), holdPiece(-1), holdUsed(false),
      gravCounter(0), lockFrames(0), lockResets(0)
{
    bagIdx = NUM_PIECES; // ép đổ túi mới ở lần lấy khối đầu tiên
    nextPiece = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            board[r][c] = 0;
}

// Ô (r,c) trong hộp 4x4 của khối (type) sau khi xoay CW 'rot' lần có được lấp không?
// Ánh xạ ngược mỗi lần xoay CW: (r,c) -> (3-c, r) về toạ độ hình spawn.
bool TetrisGame::cellFilled(int type, int rot, int r, int c) const
{
    int rr = r, cc = c;
    for (int i = 0; i < (rot & 3); i++)
    {
        const int nr = 3 - cc;
        const int nc = rr;
        rr = nr;
        cc = nc;
    }
    return PIECE_SPAWN[type][rr][cc] != 0;
}

bool TetrisGame::collides(int type, int rot, int x, int y) const
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (!cellFilled(type, rot, r, c))
                continue;
            const int br = y + r;
            const int bc = x + c;
            if (bc < 0 || bc >= COLS || br >= ROWS) // ra ngoài tường/đáy
                return true;
            if (br >= 0 && board[br][bc] != 0)      // đè khối đã khoá (cho phép br<0 = trên đỉnh)
                return true;
        }
    }
    return false;
}

void TetrisGame::lockPiece()
{
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            if (cellFilled(curType, curRot, r, c))
            {
                const int br = curY + r;
                const int bc = curX + c;
                if (br >= 0 && br < ROWS && bc >= 0 && bc < COLS)
                    board[br][bc] = (uint8_t)(curType + 1);
            }
    emitSound(SND_LOCK);
}

void TetrisGame::clearLines()
{
    int cleared = 0;
    for (int r = ROWS - 1; r >= 0;)
    {
        bool full = true;
        for (int c = 0; c < COLS; c++)
            if (board[r][c] == 0) { full = false; break; }

        if (full)
        {
            for (int rr = r; rr > 0; rr--)        // dồn các hàng trên xuống
                for (int c = 0; c < COLS; c++)
                    board[rr][c] = board[rr - 1][c];
            for (int c = 0; c < COLS; c++)
                board[0][c] = 0;
            cleared++;
            // không giảm r: kiểm tra lại chính hàng này (giờ là hàng vừa dồn xuống)
        }
        else
        {
            r--;
        }
    }

    if (cleared > 0)
    {
        const uint8_t oldLevel = level;
        // Điểm theo số hàng xoá CÙNG LÚC (×level) — chuẩn Guideline
        static const uint16_t lineScore[5] = { 0, 100, 300, 500, 800 };
        const int n = (cleared > 4) ? 4 : cleared;
        score += (uint32_t)lineScore[n] * level;
        lines += (uint16_t)cleared;
        level = (uint8_t)(1 + lines / 10); // mỗi 10 hàng -> +1 level -> rơi nhanh hơn

        emitSound((cleared >= 4) ? SND_TETRIS : SND_LINE);
        if (level > oldLevel)
            emitSound(SND_LEVELUP);
    }
}

int TetrisGame::getGravityFrames() const
{
    if (mode == MODE_ZEN)
        return 40; // Zen: tốc độ nhẹ cố định (~0.67s/ô), không tăng theo level
    // 60fps: level 1 ~48 frame/ô (0.8s) ... giảm 4 frame mỗi level, sàn 4 frame
    int f = 48 - (int)(level - 1) * 4;
    return (f < 4) ? 4 : f;
}

uint32_t TetrisGame::randomValue()
{
#ifndef SIMULATOR
    return hw_rand();                              // RNG phần cứng STM32 (Yêu cầu ①)
#else
    rngState = rngState * 1103515245u + 12345u;    // simulator: RNG phần mềm (LCG)
    return rngState;
#endif
}

void TetrisGame::emitSound(uint8_t id)
{
#ifndef SIMULATOR
    sound_emit(id); // board: đẩy vào hàng đợi -> AudioTask phát qua buzzer
#else
    (void)id;       // simulator: không có buzzer
#endif
}

// Thuật toán 7-bag: mỗi túi chứa đúng 1 lần đủ 7 khối, xáo trộn -> công bằng,
// không bị "đói" khối, không ra quá nhiều khối trùng liên tiếp.
void TetrisGame::refillBag()
{
    for (int i = 0; i < NUM_PIECES; i++)
        bag[i] = (uint8_t)i;

    // Xáo trộn Fisher-Yates bằng RNG phần cứng
    for (int i = NUM_PIECES - 1; i > 0; i--)
    {
        uint32_t j = randomValue() % (uint32_t)(i + 1);
        uint8_t t = bag[i];
        bag[i] = bag[j];
        bag[j] = (uint8_t)t;
    }
    bagIdx = 0;
}

int TetrisGame::nextType()
{
    if (bagIdx >= NUM_PIECES)
        refillBag();
    return (int)bag[bagIdx++];
}

void TetrisGame::spawn()
{
    curType = nextPiece;     // khối hiện tại = "next" trước đó
    nextPiece = nextType();  // bốc khối next mới từ túi 7-bag
    curRot = 0;
    curX = 3; // canh giữa hộp 4x4 trong 10 cột
    curY = 0;
    holdUsed = false;        // khối mới -> được phép giữ lại 1 lần
    resetPieceTimers();      // khối mới -> reset đồng hồ trọng lực + lock delay
    if (collides(curType, curRot, curX, curY))
    {
        if (mode == MODE_ZEN)
        {
            // Zen: không bao giờ thua -> dọn sạch bàn rồi chơi tiếp
            // (bàn trống nên khối spawn tại (3,0) không còn đè).
            for (int r = 0; r < ROWS; r++)
                for (int c = 0; c < COLS; c++)
                    board[r][c] = 0;
        }
        else
        {
            gameOver = true; // không còn chỗ spawn -> thua
            emitSound(SND_GAMEOVER);
        }
    }
}

void TetrisGame::start(Mode m)
{
    mode = (uint8_t)m;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            board[r][c] = 0;
    gameOver = false;
    lines = 0;
    score = 0;
    level = 1;
    holdPiece = -1;         // chưa giữ khối nào
    holdUsed = false;
    bagIdx = NUM_PIECES;    // túi rỗng -> sẽ đổ túi mới
    nextPiece = nextType(); // bốc khối "next" đầu tiên
    spawn();
}

// Khối hiện tại đã "tiếp đất" chưa (không thể rơi thêm 1 ô)?
bool TetrisGame::grounded() const
{
    return collides(curType, curRot, curX, curY + 1);
}

// Reset đồng hồ trọng lực + lock delay — gọi khi có khối MỚI (spawn/hold đổi khối)
void TetrisGame::resetPieceTimers()
{
    gravCounter = 0;
    lockFrames = 0;
    lockResets = 0;
}

// Move-reset: thao tác (di chuyển/xoay) thành công khi khối đang chạm đáy
// -> hoãn khoá thêm 0,5 s, tối đa LOCK_MAX_RESETS lần (chống giữ khối lơ lửng mãi)
void TetrisGame::onSuccessfulMove()
{
    if (lockFrames > 0 && lockResets < LOCK_MAX_RESETS)
    {
        lockFrames = 0;
        lockResets++;
    }
}

// Xoay theo SRS: dir = +1 (CW) / -1 (CCW). Thử 5 phép dịch wall-kick;
// lọt vị trí nào nhận vị trí đó, trượt cả 5 thì giữ nguyên.
void TetrisGame::tryRotate(int dir)
{
    if (curType == 1) // khối O: xoay không đổi hình -> bỏ qua
        return;

    const int nr = (curRot + (dir > 0 ? 1 : 3)) & 3;
    // CW từ trạng thái s: dùng hàng s. CCW từ s về s-1: đảo dấu hàng (s-1) = hàng nr.
    const int row = (dir > 0) ? curRot : nr;
    const int8_t (*kick)[2] = (curType == 0) ? KICK_I_CW[row] : KICK_JLSTZ_CW[row];

    for (int i = 0; i < 5; i++)
    {
        const int dx = (dir > 0) ? kick[i][0] : -kick[i][0];
        const int dy = (dir > 0) ? kick[i][1] : -kick[i][1];
        // SRS quy ước dy dương = LÊN; trục y bàn hướng XUỐNG -> trừ dy
        const int nx = curX + dx;
        const int ny = curY - dy;
        if (!collides(curType, nr, nx, ny))
        {
            curRot = nr;
            curX = nx;
            curY = ny;
            onSuccessfulMove(); // xoay thành công lúc chạm đáy -> hoãn khoá
            emitSound(SND_ROTATE);
            return;
        }
    }
}

// Gọi MỖI FRAME (60fps). Gộp trọng lực + lock delay:
// - Chưa chạm đáy: đếm gravCounter, đủ getGravityFrames() thì rơi 1 ô.
// - Đã chạm đáy: đếm lockFrames, đủ LOCK_DELAY_FRAMES (~0,5 s) mới khoá
//   (người chơi còn kịp trượt/xoay; mỗi thao tác thành công reset đồng hồ này).
// Trả về true nếu có thay đổi cần vẽ lại.
bool TetrisGame::onFrameTick()
{
    if (gameOver)
        return false;

    if (grounded())
    {
        gravCounter = 0; // đứng trên nền -> trọng lực tạm nghỉ
        if (++lockFrames >= LOCK_DELAY_FRAMES)
        {
            lockPiece();
            clearLines();
            spawn();
            return true;
        }
        return false;
    }

    lockFrames = 0; // rơi tự do (vd vừa trượt ra mép) -> huỷ đếm khoá
    if (++gravCounter >= (uint8_t)getGravityFrames())
    {
        gravCounter = 0;
        curY++;
        return true;
    }
    return false;
}

void TetrisGame::command(Command c)
{
    if (gameOver)
        return;

    switch (c)
    {
    case CMD_LEFT:
        if (!collides(curType, curRot, curX - 1, curY))
        {
            curX--;
            onSuccessfulMove(); // trượt thành công lúc chạm đáy -> hoãn khoá (move-reset)
            emitSound(SND_MOVE);
        }
        break;
    case CMD_RIGHT:
        if (!collides(curType, curRot, curX + 1, curY))
        {
            curX++;
            onSuccessfulMove();
            emitSound(SND_MOVE);
        }
        break;
    case CMD_ROT_CW:
        tryRotate(+1); // xoay CW theo SRS: thử 5 phép dịch wall-kick
        break;
    case CMD_ROT_CCW:
        tryRotate(-1); // xoay CCW theo SRS
        break;
    case CMD_SOFT_DROP:
        if (!collides(curType, curRot, curX, curY + 1))
        {
            curY++;
            gravCounter = 0; // vừa rơi tay -> hoãn nhịp trọng lực kế
            score += 1;      // thưởng nhẹ soft drop
        }
        // Đã chạm đáy: KHÔNG khoá ngay — để lock delay xử lý (người chơi còn kịp chỉnh)
        break;
    case CMD_HARD_DROP:
    {
        int dist = 0;
        while (!collides(curType, curRot, curX, curY + 1)) { curY++; dist++; }
        score += (uint32_t)(2 * dist); // thưởng hard drop theo quãng đường rơi
        lockPiece();                   // hard drop là ngoại lệ duy nhất: khoá NGAY, bỏ qua lock delay
        clearLines();
        spawn();
        break;
    }
    }
}

void TetrisGame::hold()
{
    if (gameOver || holdUsed)
        return; // mỗi khối chỉ giữ được 1 lần

    if (holdPiece < 0)
    {
        holdPiece = curType; // lần đầu: cất khối hiện tại, lấy khối mới
        spawn();             // (spawn đặt holdUsed=false) -> set true bên dưới
    }
    else
    {
        const int t = holdPiece; // đổi khối hiện tại <-> khối đang giữ
        holdPiece = curType;
        curType = t;
        curRot = 0;
        curX = 3;
        curY = 0;
        resetPieceTimers();      // khối đổi vào coi như khối mới -> reset trọng lực + lock delay
        if (collides(curType, curRot, curX, curY))
        {
            if (mode == MODE_ZEN)
            {
                for (int r = 0; r < ROWS; r++)
                    for (int c = 0; c < COLS; c++)
                        board[r][c] = 0; // Zen không thua: dọn bàn rồi chơi tiếp
            }
            else
            {
                gameOver = true; // hiếm: khối đổi vào kẹt ngay
                emitSound(SND_GAMEOVER);
            }
        }
    }
    holdUsed = true;
    emitSound(SND_ROTATE);
}

void TetrisGame::renderTo(uint8_t* grid) const
{
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            grid[r * COLS + c] = board[r][c];

    if (!gameOver)
    {
        // Bóng (ghost): hạ khối hiện tại thẳng xuống đáy để báo nơi sẽ rơi.
        // Mã ô bóng = (mã khối) + 8  -> 9..15, BoardWidget vẽ mờ lại.
        int gy = curY;
        while (!collides(curType, curRot, curX, gy + 1))
            gy++;
        if (gy != curY)
        {
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++)
                    if (cellFilled(curType, curRot, r, c))
                    {
                        const int br = gy + r;
                        const int bc = curX + c;
                        if (br >= 0 && br < ROWS && bc >= 0 && bc < COLS &&
                            grid[br * COLS + bc] == 0) // chỉ vẽ bóng lên ô trống
                            grid[br * COLS + bc] = (uint8_t)(curType + 1 + 8);
                    }
        }

        // Khối hiện tại (vẽ SAU -> đè lên bóng nếu trùng ô)
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                if (cellFilled(curType, curRot, r, c))
                {
                    const int br = curY + r;
                    const int bc = curX + c;
                    if (br >= 0 && br < ROWS && bc >= 0 && bc < COLS)
                        grid[br * COLS + bc] = (uint8_t)(curType + 1);
                }
    }
}

} // namespace tetris
