# BÁO CÁO BÀI 3 – TOUCHGFX & FREERTOS
## STM32F429I-Discovery

**Board:** STM32F429I-Discovery (LCD 240×320)  
**Framework:** TouchGFX + FreeRTOS + STM32 HAL  

---

# BÀI 3.1 – GIẢI THÍCH HOẠT ĐỘNG CHƯƠNG TRÌNH (cuối phần 3.1.3)

**Project tham chiếu:** HelloTouchGFX (`Bai313Lab3`)

## 1. Mục tiêu bài 3.1

Bài 3.1 giới thiệu lập trình giao diện đồ họa với **TouchGFX** trên STM32F429: tạo màn hình đầu tiên, hiểu kiến trúc **MVP** (Model – View – Presenter), kết hợp **FreeRTOS** để tách task đọc phần cứng và task vẽ màn hình. Phần 3.1.3 mở rộng thêm **Message Queue** để truyền sự kiện nút bấm vật lý từ task nền sang luồng TouchGFX.

## 2. Tổng quan hệ thống

Chương trình chạy trên STM32F429I-Discovery với LCD 240×320, SDRAM làm framebuffer, LTDC + DMA2D vẽ ảnh. **FreeRTOS** quản lý hai task chính:

| Task | Nhiệm vụ |
|------|----------|
| `TouchGFX_Task` (GUI) | Khởi tạo và chạy vòng lặp TouchGFX: vẽ widget, xử lý tick animation, nhận sự kiện cảm ứng |
| `defaultTask` | Đọc trạng thái nút bấm vật lý **PA0** (User button), gửi dữ liệu vào hàng đợi `myQueue01` |

Luồng khởi động trong `main.c`:

1. `HAL_Init()` → cấu hình clock hệ thống (180 MHz).
2. Khởi tạo LTDC, DMA2D, FMC/SDRAM (bộ nhớ khung hình LCD).
3. `MX_TouchGFX_Init()` – khởi tạo framework TouchGFX.
4. Tạo **Message Queue** `myQueue01Handle` (16 phần tử, kích thước phần tử theo cấu hình lab).
5. Tạo task `defaultTask` và `TouchGFX_Task`.
6. `osKernelStart()` – bắt đầu scheduler FreeRTOS.

## 3. Kiến trúc MVP (Model – View – Presenter)

TouchGFX dùng mô hình **MVP** để tách giao diện và logic:

| Thành phần | File | Vai trò |
|------------|------|---------|
| **Model** | `Model.cpp` | Lưu trữ dữ liệu nền (trong lab này còn trống, `Model::tick()` chưa xử lý gì) |
| **View** | `Screen1View.cpp` | Hiển thị widget, xử lý animation và sự kiện tick |
| **Presenter** | `Screen1Presenter.cpp` | Cầu nối giữa View và Model |
| **FrontendApplication** | `FrontendApplication.cpp` | Quản lý chuyển màn hình, gọi `model.tick()` mỗi frame |

Khi khởi động, `FrontendApplication` gọi `gotoScreen1ScreenNoTransition()` để hiển thị **Screen1** – màn hình Hello TouchGFX.

## 4. Giao diện Screen1 (TouchGFX Designer)

Màn hình gồm các widget chính:

- **TextArea** hiển thị lời chào (Hello TouchGFX).
- **Circle** (`circle1`) – hình tròn dùng cho animation.
- **Button** trên màn hình – khi bấm gọi callback `buttonClicked()`.

## 5. Task đọc nút bấm – `StartDefaultTask()` (main.c)

Task `defaultTask` chạy vòng lặp vô hạn:

```
for (;;)
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)  // User button PA0
    {
        uint8_t data = 'X';
        osMessageQueuePut(myQueue01Handle, &data, 0, 10);
    }
    osDelay(1);
}
```

**Giải thích:**

- **PA0** là nút User trên board Discovery; khi nhấn, mức logic = HIGH.
- Gửi ký tự `'X'` (0x58) vào queue `myQueue01Handle` – báo hiệu có sự kiện nút bấm.
- `osDelay(1)` trì hoãn 1 ms, tránh đọc dồn dập và giảm nhiễu cơ học nút bấm.

Task này **không trực tiếp** vẽ lên LCD; chỉ ghi message vào queue để task TouchGFX xử lý sau.

## 6. Xử lý sự kiện trong Screen1View

### 6.1. `tickEvent()` – Animation và nhận queue

Hàm `Screen1View::tickEvent()` được gọi mỗi frame từ TouchGFX:

1. **Cập nhật bộ đếm thời gian animation:**
   - `tickCount += 2`
   - `tickCount = tickCount % 240` (chu kỳ lặp)

2. **Kiểm tra hàng đợi:**
   - Nếu `osMessageQueueGetCount(myQueue01Handle) > 0`, đọc message bằng `osMessageQueueGet()`.
   - Nếu dữ liệu nhận được là `'X'`, kích hoạt di chuyển hình tròn.

3. **Tính quỹ đạo dao động (hàm sin):**

```cpp
float x = tickCount / 55.0f;
float y = sin(x) + sin(2*x) + sin(3*x) + 1;
```

4. **Di chuyển `circle1`:**
   - `circle1.moveTo((int16_t)floor(x * 55), 200 - (int16_t)floor(y * 50))`
   - `circle1.invalidate()` – yêu cầu TouchGFX vẽ lại vùng hình tròn.

Hình tròn di chuyển theo **đường cong dao động** (tổng nhiều sin), tạo hiệu ứng lượn sóng trên màn hình. Message `'X'` từ queue kích hoạt bước di chuyển mới mỗi khi người dùng nhấn nút PA0.

### 6.2. `buttonClicked()` – Nút trên màn hình cảm ứng

Khi người dùng chạm nút trên LCD:

```cpp
void Screen1View::buttonClicked()
{
    HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);  // LED đỏ trên board
}
```

Mỗi lần bấm nút TouchGFX, LED **PG13** (LED đỏ) trên Discovery đổi trạng thái sáng/tắt – minh họa tương tác từ giao diện đồ họa xuống GPIO.

## 7. Luồng dữ liệu tổng thể (Bài 3.1.3)

```
[Nút PA0 trên board]
        ↓
  defaultTask: đọc GPIO → gửi 'X' vào myQueue01
        ↓
  FreeRTOS Message Queue (myQueue01Handle)
        ↓
  Screen1View::tickEvent(): nhận 'X' → di chuyển circle1 theo quỹ đạo sin
        ↓
  TouchGFX vẽ lại framebuffer → LTDC hiển thị lên LCD

[Nút cảm ứng trên LCD]
        ↓
  Screen1View::buttonClicked() → toggle LED PG13
```

## 8. Kết luận bài 3.1

Chương trình HelloTouchGFX minh họa:

- Khởi tạo TouchGFX trên STM32F429 với FreeRTOS.
- Kiến trúc MVP: View xử lý hiển thị, Presenter/Model sẵn sàng mở rộng.
- **Tách task**: task nền đọc GPIO, task GUI vẽ màn hình – giao tiếp qua **Message Queue** CMSIS-RTOS v2.
- Animation widget (`circle1`) kết hợp hàm lượng giác và sự kiện phần cứng.

Đây là nền tảng cho các bài sau (StopWatch, game đua xe) khi thay GPIO bằng ADC joystick và mở rộng logic trong Model/View.

---

# BÀI 3.2 – GIẢI THÍCH HOẠT ĐỘNG CHƯƠNG TRÌNH

**Project tham chiếu:** StopWatch (`Bai32Lab3`)

## 1. Mục tiêu bài 3.2

Xây dựng ứng dụng **đồng hồ bấm giờ (StopWatch)** trên TouchGFX: kim đồng hồ quay theo thời gian đã trôi qua, điều khiển bằng nút bấm vật lý qua FreeRTOS queue. Bài tập củng cố kỹ năng dùng **TextureMapper** (widget 3D/phẳng xoay ảnh) và đo thời gian bằng `osKernelGetTickCount()`.

## 2. Tổng quan hệ thống

Tương tự bài 3.1, chương trình dùng hai task:

| Task | Nhiệm vụ |
|------|----------|
| `TouchGFX_Task` | Chạy TouchGFX, vẽ mặt đồng hồ và kim |
| `defaultTask` | Liên tục đọc PA0, gửi trạng thái nút (0/1) vào `myQueue01` |

Queue được tạo bằng:

```c
myQueue01Handle = osMessageQueueNew(1, sizeof(uint8_t), &myQueue01_attributes);
```

Hàng đợi 1 phần tử, mỗi phần tử 1 byte – lưu trạng thái nút: `0` (nhả) hoặc `1` (đang giữ).

## 3. Giao diện Screen1

Màn hình StopWatch gồm:

- **TextureMapper** (`textureMapper1`) – hiển thị ảnh kim phút (`ANALOGCLOCK_HANDS_SMALL_MINUTE`), đặt tại vị trí (0, 40), scale 1.0.
- Kim được xoay quanh trục Z bằng `textureMapper1.updateAngles(xAngle, yAngle, zAngle)`.

Widget TextureMapper cho phép xoay bitmap mà không cần vẽ lại từng pixel thủ công.

## 4. Task đọc nút bấm – `StartDefaultTask()`

```c
for (;;)
{
    buttonState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    osMessageQueuePut(myQueue01Handle, &buttonState, 0U, 0U);
    osDelay(10);
}
```

**Khác bài 3.1:**

- Gửi **liên tục** trạng thái nút (0 hoặc 1), không chỉ khi nhấn.
- Chu kỳ 10 ms – đủ nhanh để bắt trạng thái giữ nút, đủ chậm để không chiếm CPU.

## 5. Logic StopWatch trong `Screen1View::tickEvent()`

View duy trì các biến trạng thái:

| Biến | Ý nghĩa |
|------|---------|
| `stopwatchMs` | Tổng thời gian đã đo (millisecond) |
| `lastKernelTick` | Tick FreeRTOS ở lần cập nhật trước |
| `buttonPressed` | Nút PA0 đang được giữ (true/false) |
| `tickCount` | Bộ đếm phụ (khởi tạo 0) |

### Bước 1 – Đọc hàng đợi

```cpp
while (osMessageQueueGet(myQueue01Handle, &buttonState, 0U, 0U) == osOK)
{
    buttonPressed = (buttonState != 0);
}
```

Vòng `while` xả toàn bộ message trong queue, lấy trạng thái nút mới nhất.

### Bước 2 – Đo thời gian

```cpp
uint32_t now = osKernelGetTickCount();

if (lastKernelTick == 0)
    lastKernelTick = now;

if (buttonPressed)
    stopwatchMs += (now - lastKernelTick);

lastKernelTick = now;
```

- `osKernelGetTickCount()` trả về số tick từ khi kernel khởi động (1 tick = 1 ms với cấu hình mặc định).
- Chỉ **cộng dồn thời gian** khi nút đang được giữ (`buttonPressed == true`) – mô phỏng bấm giờ khi giữ nút.

### Bước 3 – Quay kim đồng hồ

```cpp
uint32_t msInMinute = stopwatchMs % 60000U;
float angle = ((float)msInMinute / 60000.0f) * 2.0f * 3.1415926f;
textureMapper1.updateZAngle(angle);
```

- Lấy phần dư modulo 60 giây (60000 ms) – kim quay **một vòng đủ** mỗi phút.
- Góc Z tỷ lệ thuận với thời gian: 0 → 2π radian khi đủ 60 giây.
- `updateZAngle(angle)` xoay kim và TouchGFX vẽ lại frame.

## 6. Luồng hoạt động tổng thể

```
[Nút PA0: nhả / giữ]
        ↓
  defaultTask (10 ms): đọc GPIO → gửi 0/1 vào myQueue01
        ↓
  Screen1View::tickEvent() (mỗi frame ~16 ms):
      - Nhận buttonPressed từ queue
      - Nếu đang giữ nút: stopwatchMs += delta tick
      - Tính góc kim từ stopwatchMs
      - textureMapper1.updateZAngle(angle)
        ↓
  LCD hiển thị kim quay theo thời gian bấm giữ
```

## 7. So sánh bài 3.1 và 3.2

| Tiêu chí | Bài 3.1 (HelloTouchGFX) | Bài 3.2 (StopWatch) |
|----------|-------------------------|---------------------|
| Dữ liệu queue | Ký tự `'X'` khi nhấn | Trạng thái 0/1 liên tục |
| Chu kỳ task nền | 1 ms, chỉ gửi khi nhấn | 10 ms, gửi mỗi vòng |
| Widget chính | Circle | TextureMapper (kim đồng hồ) |
| Logic View | Animation sin + di chuyển | Đo thời gian + xoay góc |
| Tương tác phụ | Nút cảm ứng → LED PG13 | Không có (chỉ PA0) |

## 8. Kết luận bài 3.2

StopWatch kết hợp TouchGFX widget nâng cao (TextureMapper) với đo thời gian thực trên FreeRTOS. Message queue truyền trạng thái nút bấm an toàn giữa hai task; View tích lũy thời gian và ánh xạ sang góc quay kim – minh họa cách xây dựng ứng dụng nhúng có phản hồi thời gian thực trên giao diện đồ họa.

---

# BÀI TẬP TỰ LÀM
## Giải thích mã nguồn thêm vào – Game đua xe SimpleRacing

**Project:** SimpleRacing  
**Board:** STM32F429I-Discovery  
**Framework:** TouchGFX + FreeRTOS + STM32 HAL  

---

## 1. Mục tiêu bài tập

Xây dựng game đua xe trên nền project TouchGFX SimpleRacing với các yêu cầu:

- Đường đua di chuyển theo chiều từ trên xuống
- Xe chỉ di chuyển theo chiều ngang
- Điều khiển xe bằng joystick (ADC, 1 kênh ngang – VRx nối PC3/IN13)
- Tăng tốc theo thời gian chơi
- Sinh chướng ngại vật ngẫu nhiên
- Tính điểm, lưu và hiển thị High score

---

## 2. Tổng quan các file đã thêm hoặc sửa

| STT | File | Nội dung thêm vào |
|-----|------|-------------------|
| 1 | Core/Src/main.c | Cấu hình Queue1, task đọc ADC joystick |
| 2 | TouchGFX/gui/src/model/Model.cpp | Logic game, điểm, tốc độ, lưu Flash |
| 3 | TouchGFX/gui/include/gui/model/Model.hpp | Khai báo biến và hàm trạng thái game |
| 4 | TouchGFX/gui/src/screen2_screen/Screen2View.cpp | Gameplay: xe, va chạm, UI điểm |
| 5 | TouchGFX/gui/include/gui/screen2_screen/Screen2View.hpp | Buffer hiển thị điểm |
| 6 | TouchGFX/gui/src/screen2_screen/Screen2Presenter.cpp | Cầu nối View – Model |
| 7 | TouchGFX/gui/include/gui/screen2_screen/Screen2Presenter.hpp | Getter/setter dữ liệu game |
| 8 | TouchGFX/assets/texts/texts.xml | Text Score/High và ký tự số cho font |

**Cấu hình CubeMX:** ADC1 kênh IN13 (PC3), FreeRTOS, độ phân giải ADC 8-bit.

---

## 3. Luồng hoạt động tổng thể

Chương trình sử dụng hai task FreeRTOS chạy song song:

1. **defaultTask:** Đọc giá trị ADC từ joystick, xác định hướng (-1/0/+1), gửi vào hàng đợi Queue1.
2. **GUI_Task (TouchGFX):** Vẽ giao diện, cập nhật animation và gameplay mỗi frame.

Luồng dữ liệu:

```
Joystick (VRx → PC3 → ADC1)
        ↓
  defaultTask: đọc ADC → dir (-1 / 0 / +1)
        ↓
  Queue1 (FreeRTOS message queue)
        ↓
  Model::tick(): nhận dir, tính score, tăng gameSpeed
        ↓
  Screen2Presenter: chuyển dữ liệu sang View
        ↓
  Screen2View::handleTickEvent(): vẽ đường, xe, chướng ngại,
                                  kiểm tra va chạm, cập nhật điểm
```

Việc tách task đọc phần cứng và task vẽ màn hình giúp luồng TouchGFX không bị block khi đọc ADC, đồng thời giảm nhiễu nhờ lọc và hiệu chuẩn tín hiệu analog.

---

## 4. Giải thích chi tiết từng phần mã nguồn

### 4.1. File main.c – Ghép nối joystick qua ADC

#### a) Tạo Message Queue

```c
osMessageQueueId_t Queue1Handle;
Queue1Handle = osMessageQueueNew(8, sizeof(int8_t), &Queue1_attributes);
```

- Tạo hàng đợi CMSIS-RTOS v2 gồm 8 phần tử.
- Mỗi phần tử 1 byte kiểu int8_t, biểu diễn hướng: -1 (trái), 0 (giữa), +1 (phải).

#### b) Hàm StartDefaultTask() – Đọc ADC

**Bước 1 – Hiệu chuẩn điểm giữa joystick**

Khi bật nguồn, chương trình đọc ADC 64 lần (mỗi lần cách 10 ms), lấy trung bình làm giá trị adcCenter. Mục đích: mỗi joystick có điện áp nghỉ khác nhau, cần xác định điểm giữa thực tế thay vì cố định 128.

**Bước 2 – Vòng lặp đọc liên tục (chu kỳ 20 ms)**

- Khởi động chuyển đổi ADC: HAL_ADC_Start(), HAL_ADC_PollForConversion(), HAL_ADC_GetValue().
- Kênh ADC: IN13, chân PC3 (nối VRx joystick).
- Lọc nhiễu: lấy trung bình 4 mẫu liên tiếp.
- So sánh với vùng chết (dead zone) quanh adcCenter:
  - ADC < adcCenter - adcDead  → dir = -1 (sang trái)
  - ADC > adcCenter + adcDead  → dir = +1 (sang phải)
  - Trong vùng chết           → dir = 0  (đứng yên)
- Gửi dir vào Queue1 bằng osMessageQueuePut().

**Ghép nối phần cứng:**

| Chân joystick | Nối vào board |
|---------------|---------------|
| GND | GND |
| +5V / VCC | 3.3V |
| VRx | PC3 (ADC1_IN13) |
| VRy | Không nối |

---

### 4.2. File Model.hpp / Model.cpp – Logic game

#### Các biến trạng thái bổ sung

| Biến | Kiểu | Ý nghĩa |
|------|------|---------|
| carDirection | int8_t | Hướng xe: -1, 0, +1 |
| score | uint16_t | Điểm hiện tại |
| highScore | uint16_t | Điểm cao nhất |
| gameSpeed | uint8_t | Tốc độ game (2 đến 8) |
| gameTickCounter | uint32_t | Đếm tick để tăng tốc |
| gameOver | bool | Trạng thái va chạm / kết thúc |
| gameRunning | bool | Đang trong màn chơi |

#### Hàm Model::tick()

Được gọi mỗi frame từ FrontendApplication::handleTickEvent():

1. Đọc tất cả message trong Queue1, cập nhật carDirection (chỉ khi gameRunning và chưa gameOver).
2. Tăng score lên 1 mỗi frame (thưởng thời gian sống sót).
3. Cứ 60 tick (~1 giây), tăng gameSpeed lên 1 (tối đa 8).

#### Các hàm điều khiển vòng đời game

- **startGame():** Reset score, gameSpeed = 2, gameRunning = true, carDirection = 0.
- **stopGame():** gameRunning = false, carDirection = 0.
- **setGameOver():** gameOver = true; nếu score > highScore thì cập nhật và ghi Flash.

#### Lưu High score vào Flash

- Địa chỉ: sector 23, 0x081E0000.
- Cấu trúc: 4 byte magic (0x48534352) + 4 byte điểm.
- loadHighScore(): đọc khi khởi động, kiểm tra magic hợp lệ.
- saveHighScore(): xóa sector, ghi dữ liệu mới khi phá kỷ lục.

---

### 4.3. File Screen2Presenter.cpp – Lớp Presenter

Presenter đóng vai trò cầu nối giữa View và Model, không trực tiếp vẽ giao diện.

| Hàm | Chức năng |
|-----|-----------|
| activate() | Gọi startGame() và updateScoreTexts() khi vào màn chơi |
| deactivate() | Gọi stopGame() khi rời màn chơi |
| getScore(), getHighScore(), getCarDirection(), getGameSpeed() | Lấy dữ liệu từ Model cho View |
| setGameOver() | Báo Model kết thúc game |

---

### 4.4. File Screen2View.cpp – Gameplay và giao diện

#### a) Khởi tạo màn chơi (setupScreen)

- Đặt xe image1 tại Y cố định CAR_Y = 230 (gần đáy màn 320px).
- Khôi phục vị trí X từ Model (ImageX).
- Đặt chướng ngại lamb phía trên màn hình.
- Cấu hình textArea1 (Score) và textArea2 (High): màu, vị trí, gán wildcard buffer.

#### b) Animation đường đua cuộn từ trên xuống

- Dùng 5 ảnh track0 đến track4 xếp chồng full màn hình 240×320.
- Mỗi roadInterval tick, chỉ hiện một ảnh, ẩn ảnh trước → hiệu ứng đường chạy xuống.
- roadInterval = 10 - gameSpeed → tốc độ cuộn tăng khi gameSpeed tăng.

#### c) Xe chỉ di chuyển ngang

```cpp
localImageX += dir * CAR_SPEED;   // CAR_SPEED = 4 pixel/tick
// Giới hạn trong [CAR_MIN_X, CAR_MAX_X]
image1.setX(localImageX);
```

Tọa độ Y của xe không đổi; chỉ X thay đổi theo carDirection từ joystick.

#### d) Chướng ngại vật ngẫu nhiên

```cpp
newY = lamb.getY() + gameSpeed;
if (newY >= 320) {
    newY = -OBSTACLE_HEIGHT;
    lane = getRandomLane();        // random 0..3
    lamb.setX(lane * 60 + 15);     // 4 làn, mỗi làn rộng ~60px
}
```

Hàm getRandomLane() dùng thuật toán Linear Congruential Generator, không cần thư viện rand() của C.

#### e) Phát hiện va chạm (checkCollision)

So sánh vùng hình chữ nhật (AABB) của xe và chướng ngại, có thu nhỏ biên bằng HITBOX_MARGIN. Nếu hai vùng giao nhau → gọi setGameOver(), dừng animation.

#### f) Hiển thị điểm (updateScoreTexts)

- Dùng Unicode::itoa() chuyển số thành chuỗi Unicode.
- Gán vào scoreBuffer / highScoreBuffer qua setWildcard().
- resizeToCurrentTextWithAlignment() để khung chữ vừa nội dung.
- invalidateContent() để TouchGFX vẽ lại text.

Lưu ý: Font phải chứa ký tự 0–9 (cấu hình trong texts.xml, Generate Code TouchGFX).

#### g) Thoát game (ExitFromScreen2)

Gọi khi bấm nút Home → stopGame() → dừng cập nhật logic khi về menu.

---

## 5. Bảng ánh xạ yêu cầu bài tập – mã nguồn

| Yêu cầu | Vị trí thực hiện |
|---------|------------------|
| Đường đua cuộn từ trên xuống | Screen2View::handleTickEvent() – luân phiên track0..track4 |
| Xe chỉ di chuyển ngang | localImageX += dir * CAR_SPEED, Y cố định |
| Joystick ADC (PC3/IN13) | main.c StartDefaultTask + Model::tick() đọc Queue1 |
| Tăng tốc | Model::tick(): gameSpeed++ mỗi 60 tick |
| Chướng ngại ngẫu nhiên | getRandomLane() + reset lamb khi ra khỏi màn |
| Tính điểm | Model::tick(): score++ mỗi frame |
| Lưu high score | saveHighScore() / loadHighScore() – Flash sector 23 |
| Hiển thị score & high | textArea1, textArea2 + updateScoreTexts() |

---

## 6. Kết luận bài tập tự làm

Mã nguồn tự thêm hoàn thiện game đua xe SimpleRacing theo mô hình MVP của TouchGFX:

- **Tầng phần cứng (main.c):** Đọc joystick qua ADC, gửi hướng qua FreeRTOS queue.
- **Tầng Model:** Quản lý trạng thái game, điểm số, tăng tốc, lưu high score vào Flash.
- **Tầng View (Screen2View):** Thực hiện animation đường, di chuyển xe, rơi chướng ngại, va chạm và hiển thị điểm.

Thiết kế tách biệt giữa đọc phần cứng và vẽ giao diện giúp chương trình ổn định, dễ mở rộng và phù hợp với kiến trúc embedded RTOS + TouchGFX – kế thừa trực tiếp từ bài 3.1 (queue + GPIO) và bài 3.2 (đo thời gian, animation widget).

---

*Tài liệu báo cáo Bài 3 – TouchGFX Lab, project SimpleRacing.*
