# Hướng Dẫn Tích Hợp TouchGFX

## 1. Tạo project

Trong TouchGFX Designer:

```text
Create New Project
Board: STM32F429I Discovery
Application Template: Empty UI
Create
```

Tạo 3 screen:

```text
MenuScreen
GameScreen
GameOverScreen
```

## 2. MenuScreen

Thêm:

```text
TextArea: SPACE INVADERS
Button: START
Interaction: click START -> change screen to GameScreen
```

Trên board thật, nút Start/Shoot cũng có thể bắt đầu game vì `GameCore` xử lý `input.start`.

## 3. GameScreen widget contract

Trong `GameScreen`, tạo các widget sau. Nên dùng Box để dễ render và ít tốn tài nguyên.

### Player

```text
Box name: playerBox
Size: 22 x 14
Color: xanh/cyan
```

### Player bullets

Tạo 10 Box:

```text
bulletBox0
bulletBox1
...
bulletBox9
```

Size ban đầu 3 x 8, màu cyan hoặc vàng.

### Enemy bullets

Tạo 8 Box:

```text
enemyBulletBox0
...
enemyBulletBox7
```

Size ban đầu 4 x 8, màu đỏ.

### Enemies

Tạo 24 Box:

```text
enemyBox0
...
enemyBox23
```

Enemy thường dùng 18 x 14. Boss dùng 58 x 28, code sẽ resize tự động.

### Explosions

Tạo 12 Box:

```text
explosionBox0
...
explosionBox11
```

Màu cam/vàng. Code sẽ resize để tạo hiệu ứng nổ to dần.

### Items

Tạo 5 Box:

```text
itemBox0
...
itemBox4
```

Size 10 x 10. Màu xanh cho heal, vàng cho power.

### Text

Tạo các TextArea with wildcard:

```text
scoreText      Label: Score
levelText      Label: Level
hpText         Label: HP
bossHpText     Label: Boss HP
```

Trong Designer, bật wildcard buffer cho từng TextArea. Tên buffer TouchGFX sinh ra có thể khác ví dụ trong file mẫu, nên bạn cần sửa lại cho khớp.

## 4. Copy code core

Copy các thư mục này vào project:

```text
CoreGame/
STM32HAL/
```

Thêm include path trong CubeIDE nếu cần:

```text
../CoreGame
../STM32HAL
```

## 5. Tích hợp Model

Trong `TouchGFX/gui/include/gui/model/Model.hpp`, thêm:

```cpp
#include "CoreGame/GameCore.hpp"

class Model
{
public:
    Model();
    void tick();
    const si::RenderState& getGameState() const;

private:
    si::GameCore game;
};
```

Trong `TouchGFX/gui/src/model/Model.cpp`, thêm ý tưởng:

```cpp
#include <gui/model/Model.hpp>
#include "STM32HAL/SpaceInvadersHw.hpp"

Model::Model() : modelListener(0)
{
    si::HW_Init();
}

void Model::tick()
{
    si::InputState input = si::HW_ReadInput();
    game.update(input);
    si::HW_HandleSoundAndLed(game.consumeSoundEvent());
}

const si::RenderState& Model::getGameState() const
{
    return game.renderState();
}
```

Lưu ý: file `Model.hpp/.cpp` thật của TouchGFX có thể đã có `modelListener`; giữ nguyên phần sinh bởi TouchGFX, chỉ thêm biến `game` và 3 đoạn logic trên.

## 6. Tích hợp Presenter

Trong `GameScreenPresenter`, thêm hàm:

```cpp
const si::RenderState& getGameState() const;
```

Trong `.cpp`:

```cpp
const si::RenderState& GameScreenPresenter::getGameState() const
{
    return model->getGameState();
}
```

## 7. Tích hợp GameScreenView

Copy logic từ:

```text
TouchGFXIntegration/GameScreenView_example.cpp
TouchGFXIntegration/GameScreenView_example.hpp
```

Nếu build lỗi do tên buffer TextArea khác, mở file generated của TouchGFX để xem tên chính xác rồi sửa lại.

## 8. Cấu hình GPIO

Trong CubeMX:

```text
BTN_LEFT       GPIO_Input Pull-up
BTN_RIGHT      GPIO_Input Pull-up
BTN_SHOOT      GPIO_Input Pull-up
BTN_START      GPIO_Input Pull-up
BTN_PAUSE      GPIO_Input Pull-up
BUZZER         GPIO_Output
LED_FEEDBACK   GPIO_Output
```

Nếu thiếu nút, có thể dùng:

```text
BTN_SHOOT làm START
BTN_PAUSE bỏ qua
```

## 9. Trình tự test

1. Build project TouchGFX rỗng.
2. Copy `CoreGame`, build lại.
3. Thêm `STM32HAL`, build lại.
4. Tích hợp `Model`, build lại.
5. Tích hợp `Presenter`, build lại.
6. Tích hợp `GameScreenView`, build lại.
7. Flash board.
8. Test input, đạn, enemy, collision, score, boss.
