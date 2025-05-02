#include <ESP32Servo.h> // <--- 修改这里：使用 ESP32 的 Servo 库

Servo motorfront; // 创建 Servo 对象 (ESP32Servo 库会处理这个类型)
// Servo motorback;

// --- 定义常量 ---
const int motorFrontPin = 13; // 确认这是你连接的 GPIO 引脚

const int MOTOR_STOP_SIGNAL = 1500;
const int MOTOR_FORWARD_SIGNAL = 2000;

const int RUN_DURATION = 500000;
const int STOP_DURATION = 5000;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Motor Test - motorfront (using ESP32Servo)");

  // --- ESP32Servo 特有设置 (通常推荐) ---
  // ESP32Servo 可能需要为 PWM 分配定时器资源
  // 预先分配可以避免冲突，特别是当使用多个伺服或 PWM 输出时
  // 参数是定时器编号 (0-3)
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // 设置 PWM 频率，标准伺服是 50Hz
  motorfront.setPeriodHertz(50);
  // -----------------------------------------

  // 将 motorfront 对象附加到指定的 GPIO 引脚
  // ESP32Servo 的 attach 通常接受 pin, min_us, max_us
  motorfront.attach(motorFrontPin, 1000, 2000); // 最小脉宽 1000µs, 最大脉宽 2000µs

  Serial.print("Motor front attached to GPIO: ");
  Serial.println(motorFrontPin);

  Serial.println("Setting initial state to STOP...");
  motorfront.writeMicroseconds(MOTOR_STOP_SIGNAL);
  delay(1000);

  Serial.println("Setup complete. Starting cycle.");
}

void loop() {
  // --- 正转阶段 ---
  Serial.println("Motor FORWARD");
  motorfront.writeMicroseconds(MOTOR_FORWARD_SIGNAL);
  delay(RUN_DURATION);

  // --- 停止阶段 ---
  Serial.println("Motor STOP");
  motorfront.writeMicroseconds(MOTOR_STOP_SIGNAL);
  delay(STOP_DURATION);
}