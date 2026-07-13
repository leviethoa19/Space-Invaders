# Báo Cáo Project Hệ Nhúng: Space Invaders Trên STM32F429I

## 1. Giới thiệu đề tài

Project xây dựng game Space Invaders chạy trên board STM32F429I Discovery. Người chơi điều khiển máy bay ở cuối màn hình, né đạn và bắn tiêu diệt kẻ địch. Game sử dụng LCD để hiển thị, joystick/nút để điều khiển, buzzer và LED để phản hồi sự kiện.

### Yêu cầu chức năng

- Hiển thị menu, màn hình chơi, pause và game over.
- Điều khiển máy bay sang trái/phải.
- Bắn đạn có giới hạn tần số.
- Có nhiều enemy trên màn hình.
- Enemy di chuyển theo FSM gồm MOVE_LEFT, MOVE_RIGHT và MOVE_DOWN.
- Xử lý va chạm đạn-enemy, enemy-player và enemy bullet-player.
- Có HP, số mạng, điểm số và level.
- Độ khó tăng dần theo level.
- Có boss nhiều pha ở một số level.
- Có vật phẩm hồi máu hoặc tăng sức mạnh.
- Có nhiều loại đạn: normal, power, spread.
- Có animation nổ, kích thước nổ tương ứng uy lực đạn.
- Có buzzer/âm thanh và LED phản hồi khi bắn, trúng địch, mất HP và game over.

### Yêu cầu phi chức năng

- Game chạy ổn định trên STM32F429I.
- Phản hồi điều khiển đủ nhanh, không gây cảm giác trễ.
- Hiển thị rõ ràng trên LCD.
- Code được chia module để dễ đọc, dễ kiểm thử và dễ mở rộng.
- Có README hướng dẫn build, cấu hình phần cứng và chạy demo.

## 2. Thiết kế

### Phân chia phần cứng và phần mềm

| Chức năng | Thực hiện |
|---|---|
| Hiển thị game | LCD + TouchGFX |
| Đọc điều khiển | GPIO/ADC joystick hoặc nút |
| Logic game | Phần mềm C/C++ |
| Va chạm, điểm, level | Phần mềm C/C++ |
| Âm thanh | Buzzer qua GPIO/PWM |
| Phản hồi trạng thái | LED GPIO |

### Thiết kế phần cứng

Các thành phần chính:

- STM32F429I Discovery.
- LCD tích hợp trên board.
- Joystick hoặc các nút điều khiển trái/phải/bắn/start/pause.
- Buzzer.
- LED phản hồi.

Gợi ý cấu hình:

| Tín hiệu | Kiểu GPIO | Mô tả |
|---|---|---|
| BTN_LEFT | Input Pull-up | Di chuyển trái |
| BTN_RIGHT | Input Pull-up | Di chuyển phải |
| BTN_SHOOT | Input Pull-up | Bắn |
| BTN_START | Input Pull-up | Bắt đầu/chơi lại |
| BTN_PAUSE | Input Pull-up | Tạm dừng |
| BUZZER | Output hoặc PWM | Phát âm thanh |
| LED_FEEDBACK | Output | LED phản hồi |

### Thiết kế phần mềm

Phần mềm được chia thành các module:

| Module | Vai trò |
|---|---|
| GameCore | Logic chính của game |
| Input | Đọc joystick/nút |
| Player | Vị trí, HP, lives, power-up |
| Bullet | Đạn người chơi và đạn enemy |
| Enemy | Enemy thường và boss |
| Collision | Kiểm tra va chạm AABB |
| ScoreLevel | Điểm, level, độ khó |
| SoundLed | Điều khiển buzzer và LED |
| Render | Cập nhật widget TouchGFX |

Luồng hoạt động:

```text
Model::tick()
  -> HW_ReadInput()
  -> GameCore::update(input)
  -> HW_HandleSoundAndLed(sound)

GameScreenView::handleTickEvent()
  -> presenter->getGameState()
  -> render object lên LCD
```

FSM enemy:

```text
MOVE_RIGHT -> chạm biên phải -> MOVE_DOWN -> MOVE_LEFT
MOVE_LEFT  -> chạm biên trái  -> MOVE_DOWN -> MOVE_RIGHT
```

Boss nhiều pha:

```text
Phase 1: bắn 1 viên thẳng
Phase 2: HP < 50%, bắn 3 hướng
Phase 3: HP < 25%, bắn 5 hướng
```

## 3. Cài đặt và xây dựng hệ thống

Project được xây dựng bằng STM32CubeIDE và TouchGFX Designer. TouchGFX đảm nhiệm phần giao diện LCD, còn logic game được tách thành module `GameCore` để dễ kiểm thử.

### Module chính

- `GameCore.cpp`: cập nhật trạng thái game theo từng tick.
- `SpaceInvadersHw.cpp`: đọc input và điều khiển buzzer/LED.
- `GameScreenView.cpp`: render player, bullet, enemy, item, explosion và text lên LCD.

### Đóng góp thành viên

| Thành viên | Đóng góp |
|---|---|
| Thành viên 1 | Thiết kế giao diện TouchGFX, menu, game screen, game over, render object |
| Thành viên 2 | Input, player movement, shooting cooldown, bullet, item |
| Thành viên 3 | Enemy FSM, collision, score/level, boss, buzzer/LED, README và demo |

Số commit cụ thể sẽ được lấy từ GitHub bằng lệnh:

```bash
git shortlog -sne
```

### Kết quả

Demo cần thể hiện:

- Màn hình menu.
- Player di chuyển và bắn.
- Enemy di chuyển bằng FSM.
- Bắn trúng enemy, điểm tăng và có hiệu ứng nổ.
- Player mất HP khi va chạm.
- Level tăng và boss xuất hiện.
- Buzzer/LED phản hồi.
- Game over và restart.

Link demo: điền sau khi quay video.

### Đánh giá

Project đạt các yêu cầu chính: điều khiển máy bay, bắn đạn có cooldown, nhiều enemy, va chạm, score, level, menu, pause, game over, buzzer/LED, boss, vật phẩm và animation nổ.

Ưu điểm:

- Logic game được tách module, dễ kiểm thử.
- Có nhiều tính năng mở rộng so với yêu cầu cơ bản.
- Dễ thay đổi độ khó thông qua `GameConfig.hpp`.

Nhược điểm:

- Hiệu ứng âm thanh ban đầu còn đơn giản nếu chỉ dùng GPIO buzzer.
- Cần tối ưu số lượng widget nếu LCD bị chậm.
- Tên widget trong TouchGFX phải đặt đúng để code render hoạt động.

## 4. AI-assisted disclaimer

Nhóm CÓ sử dụng công cụ AI để hỗ trợ thực hiện project và viết báo cáo.

AI hỗ trợ:

- Phân tích yêu cầu chức năng và phi chức năng.
- Gợi ý kiến trúc phần cứng/phần mềm.
- Gợi ý cách chia module.
- Giải thích thuật toán enemy FSM, collision AABB, cooldown bắn đạn, boss nhiều pha và vật phẩm.
- Gợi ý cách kiểm thử từng module.
- Hỗ trợ soạn README và báo cáo.

Nhóm tự thực hiện và kiểm chứng:

- Tạo project TouchGFX/STM32CubeIDE.
- Cấu hình GPIO, joystick/nút, buzzer và LED.
- Tích hợp, chỉnh sửa và debug source code.
- Build, flash lên STM32F429I.
- Kiểm thử trên phần cứng thật.
- Quay video demo và đánh giá kết quả.

Prompt chính đã sử dụng: chèn prompt đã gửi giáo viên.
