# Test Plan

## Test 1: LCD và TouchGFX

Mục tiêu: xác nhận project chạy trên STM32F429I.

Thao tác:

1. Build project TouchGFX rỗng.
2. Flash lên board.
3. Kiểm tra LCD hiện `MenuScreen`.

Kết quả đạt: LCD không trắng, không treo, chuyển được sang `GameScreen`.

## Test 2: Input

Mục tiêu: kiểm tra nút/joystick.

Thao tác:

1. Bấm trái/phải.
2. Quan sát player di chuyển.
3. Bấm shoot.

Kết quả đạt: player không vượt biên, đạn xuất hiện khi bấm shoot.

## Test 3: Cooldown bắn

Mục tiêu: đạn không sinh ra quá nhanh.

Thao tác:

1. Giữ nút shoot.
2. Quan sát số đạn trên màn hình.

Kết quả đạt: đạn xuất hiện theo nhịp, không tràn màn hình.

## Test 4: Enemy FSM

Mục tiêu: enemy di chuyển đúng mẫu.

Thao tác:

1. Vào game.
2. Quan sát enemy đi ngang.
3. Khi chạm biên, enemy đi xuống và đổi hướng.

Kết quả đạt: enemy không đi ra khỏi màn hình.

## Test 5: Collision

Mục tiêu: kiểm tra va chạm.

Thao tác:

1. Bắn trúng enemy.
2. Để enemy bullet chạm player.
3. Để enemy chạm player.

Kết quả đạt:

- Enemy mất HP/chết.
- Score tăng.
- Explosion xuất hiện.
- Player mất HP/lives.

## Test 6: Level và độ khó

Mục tiêu: sau khi diệt hết enemy, level tăng.

Thao tác:

1. Diệt toàn bộ enemy.
2. Quan sát level, tốc độ enemy và enemy bullet.

Kết quả đạt: level tăng, enemy khó hơn.

## Test 7: Boss

Mục tiêu: boss xuất hiện và có nhiều pha.

Thao tác:

1. Chơi tới level 3.
2. Bắn boss đến dưới 50% HP và dưới 25% HP.

Kết quả đạt:

- Boss HP giảm.
- Boss đổi kiểu bắn theo pha.
- Khi boss chết có explosion lớn và tăng điểm nhiều.

## Test 8: Buzzer/LED

Mục tiêu: phản hồi phần cứng.

Thao tác:

1. Bắn.
2. Bắn trúng enemy.
3. Player mất HP.
4. Game over.

Kết quả đạt: buzzer/LED có phản hồi tương ứng.
