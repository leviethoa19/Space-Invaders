# STM32F429I Space Invaders

Game Space Invaders  chạy trên STM32F429I Discovery, giao diện bằng TouchGFX, điều khiển bằng joystick/nút, có buzzer và LED phản hồi.

## Tính năng

- Menu, pause, game over.
- Điều khiển máy bay trái/phải.
- Bắn đạn có giới hạn tần số.
- Nhiều enemy di chuyển bằng FSM.
- Va chạm đạn-enemy, đạn enemy-player, enemy-player.
- Score, HP, lives, level và độ khó tăng dần.
- Boss nhiều pha ở mỗi 3 level.
- Vật phẩm hồi máu và tăng sức mạnh.
- Nhiều loại đạn: normal, power, spread.
- Animation nổ theo uy lực đạn.
- Buzzer/LED phản hồi khi bắn, trúng, mất máu, qua level, game over.

## Cấu trúc thư mục

```text
CoreGame/
  GameConfig.hpp      Tham số game: kích thước, tốc độ, số lượng object
  GameTypes.hpp       Struct/enum dùng chung
  GameCore.hpp/.cpp   Logic chính của game

STM32HAL/
  SpaceInvadersHw.hpp/.cpp   Đọc nút, điều khiển buzzer/LED

TouchGFXIntegration/
  *_example.hpp/.cpp  Mẫu tích hợp vào Model, Presenter, GameScreenView

docs/
  INTEGRATION_GUIDE.md
  REPORT_DRAFT.md
  TEST_PLAN.md

host_test.cpp         Test nhanh GameCore trên PC
```

## Công cụ đề xuất

- STM32CubeIDE 1.18.0 hoặc phiên bản đang dùng trong lớp.
- TouchGFX Designer 4.x.
- Board: STM32F429I Discovery.
- Ngôn ngữ: C/C++.

## Cách dùng với TouchGFX

1. Tạo project mới trong TouchGFX Designer:
   - Board: STM32F429I Discovery.
   - Template: Empty UI.
   - Tạo 3 screen: `MenuScreen`, `GameScreen`, `GameOverScreen`.
2. Trong `GameScreen`, tạo các Box và TextArea theo tên trong `docs/INTEGRATION_GUIDE.md`.
3. Generate Code từ TouchGFX Designer.
4. Copy thư mục `CoreGame` và `STM32HAL` vào project TouchGFX/STM32CubeIDE.
5. Sửa `Model.hpp/.cpp`, `GameScreenPresenter.hpp/.cpp`, `GameScreenView.hpp/.cpp` theo các file mẫu trong `TouchGFXIntegration`.
6. Trong CubeMX/CubeIDE, cấu hình GPIO cho nút, buzzer và LED.
7. Build, flash lên board và test từng module.

## Gợi ý chân GPIO

Bạn có thể chọn chân theo phần cứng thực tế. Đặt label trong CubeMX như sau để code mẫu tự nhận:

```text
BTN_LEFT
BTN_RIGHT
BTN_SHOOT
BTN_START
BTN_PAUSE
BUZZER
LED_FEEDBACK
```

Khuyến nghị nút dùng GPIO Input Pull-up, nhấn xuống GND. Buzzer đơn giản có thể dùng GPIO Output, nếu muốn âm thanh tốt hơn thì chuyển sang PWM bằng Timer.

## Test logic trên PC

Nếu máy có `g++`, có thể test logic game:

```bash
g++ -std=c++11 host_test.cpp CoreGame/GameCore.cpp -o host_test
./host_test
```

## Thành viên nhóm

| Thành viên | Phần phụ trách | Commit gợi ý |
|---|---|---|
| Thành viên 1 | TouchGFX UI, menu, game screen, game over, render object | `feat: add touchgfx screens`, `feat: render game objects` |
| Thành viên 2 | Input, player movement, bullet, cooldown, item | `feat: add input driver`, `feat: add player shooting` |
| Thành viên 3 | Enemy FSM, collision, score/level, boss, buzzer/LED, README | `feat: add enemy fsm`, `feat: add collision and scoring`, `docs: add readme` |

## AI-assisted disclaimer

AI được dùng như trợ giảng kỹ thuật để gợi ý kiến trúc, module, thuật toán, test plan và báo cáo. Nhóm cần tự tạo project TouchGFX, cấu hình GPIO, tích hợp code, build, flash, kiểm thử trên board thật và quay demo.
