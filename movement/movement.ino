#include <Wire.h>

/*
 * MPU6050与Arduino Nano连接方式：
 * VCC -> 3.3V或5V
 * GND -> GND
 * SCL -> A5 (I2C时钟线)
 * SDA -> A4 (I2C数据线)
 * INT -> D2 (可选)
 * AD0 -> GND (I2C地址为0x68)
 */

// MPU6050 I2C地址
const int MPU = 0x68;

// 加速度和角速度变量
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;

// 校准偏移量
float offsetAccX = 0, offsetAccY = 0, offsetAccZ = 0;
float offsetGyroX = 0, offsetGyroY = 0, offsetGyroZ = 0;

// 校准状态
bool calibrated = false;
unsigned long calibrationStartTime = 0;
const unsigned long CALIBRATION_DURATION = 10000; // 校准时间10秒

// 状态判断阈值
const float MOTION_THRESHOLD_Z = 0.4; // 上下跳动阈值
const float MOTION_THRESHOLD_GYRO = 20; // 左右晃动阈值

// 状态定义
#define STATE_STABLE 0      // 静止
#define STATE_UPDOWN 1      // 上下跳动
#define STATE_LEFTRIGHT 2   // 左右晃动

// 时间变量
unsigned long lastTime = 0;
const int sampleInterval = 500; // 采样间隔(ms)

// 运动状态历史记录
const int HISTORY_SIZE = 3; // 保存最近3次的检测结果
bool upDownHistory[HISTORY_SIZE] = {false, false, false};
bool leftRightHistory[HISTORY_SIZE] = {false, false, false};
int historyIndex = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  // 初始化MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 寄存器
  Wire.write(0x00);  // 设置为0，唤醒MPU6050
  Wire.endTransmission(true);
  
  // 配置加速度计范围 ±4g
  Wire.beginTransmission(MPU);
  Wire.write(0x1C);  // ACCEL_CONFIG 寄存器
  Wire.write(0x08);  // 设置为0x08，选择±4g范围
  Wire.endTransmission(true);
  
  // 配置陀螺仪范围 ±250deg/s
  Wire.beginTransmission(MPU);
  Wire.write(0x1B);  // GYRO_CONFIG 寄存器
  Wire.write(0x00);  // 设置为0，选择±250deg/s范围
  Wire.endTransmission(true);
  
  delay(300);
  
  // 开始校准过程
  Serial.println("请保持传感器静止10秒钟进行校准...");
  calibrationStartTime = millis();
  
  // 初始化校准变量
  startCalibration();
}

void loop() {
  unsigned long currentTime = millis();
  
  // 校准过程
  if (!calibrated) {
    // 收集校准数据
    collectCalibrationData();
    
    // 检查校准是否完成
    if (currentTime - calibrationStartTime >= CALIBRATION_DURATION) {
      finishCalibration();
      calibrated = true;
      Serial.println("校准完成！");
      Serial.print("加速度偏移量: X=");
      Serial.print(offsetAccX);
      Serial.print(", Y=");
      Serial.print(offsetAccY);
      Serial.print(", Z=");
      Serial.print(offsetAccZ);
      Serial.print(" | 陀螺仪偏移量: X=");
      Serial.print(offsetGyroX);
      Serial.print(", Y=");
      Serial.print(offsetGyroY);
      Serial.print(", Z=");
      Serial.println(offsetGyroZ);
    }
    return; // 校准未完成，不执行后续代码
  }
  
  // 校准完成后的正常操作
  if (currentTime - lastTime >= sampleInterval) {
    lastTime = currentTime;
    
    // 读取并校正数据
    readAndCorrectData();
    
    // 输出传感器数据
    Serial.print("AccX: ");
    Serial.print(AccX);
    Serial.print(" | AccY: ");
    Serial.print(AccY);
    Serial.print(" | AccZ: ");
    Serial.print(AccZ);
    Serial.print(" | GyroX: ");
    Serial.print(GyroX);
    Serial.print(" | GyroY: ");
    Serial.print(GyroY);
    Serial.print(" | GyroZ: ");
    Serial.println(GyroZ);

    // 检测当前帧的运动状态
    detectCurrentMotion();
    
    // 根据历史记录判断整体运动状态
    int state = determineOverallState();
    
    // 输出状态到串口
    Serial.print("状态: ");
    Serial.println(state);
  }
}

// 初始化校准变量
void startCalibration() {
  offsetAccX = 0;
  offsetAccY = 0;
  offsetAccZ = 0;
  offsetGyroX = 0;
  offsetGyroY = 0;
  offsetGyroZ = 0;
}

// 收集校准数据
void collectCalibrationData() {
  static int sampleCount = 0;
  static float sumAccX = 0, sumAccY = 0, sumAccZ = 0;
  static float sumGyroX = 0, sumGyroY = 0, sumGyroZ = 0;
  
  // 读取原始数据
  readRawData();
  
  // 累加数据
  sumAccX += AccX;
  sumAccY += AccY;
  sumAccZ += AccZ;
  sumGyroX += GyroX;
  sumGyroY += GyroY;
  sumGyroZ += GyroZ;
  
  sampleCount++;
  
  // 每秒显示校准进度
  if (sampleCount % 20 == 0) {
    int progress = (millis() - calibrationStartTime) * 100 / CALIBRATION_DURATION;
    Serial.print("校准进度: ");
    Serial.print(progress);
    Serial.println("%");
  }
  
  // 计算当前平均值作为临时偏移量
  if (sampleCount > 0) {
    offsetAccX = sumAccX / sampleCount;
    offsetAccY = sumAccY / sampleCount;
    offsetAccZ = sumAccZ / sampleCount - 1.0; // Z轴减去重力加速度
    offsetGyroX = sumGyroX / sampleCount;
    offsetGyroY = sumGyroY / sampleCount;
    offsetGyroZ = sumGyroZ / sampleCount;
  }
  
  delay(50); // 每50ms收集一个样本
}

// 完成校准
void finishCalibration() {
  // 校准已在collectCalibrationData中完成
  // 这里可以添加额外的处理逻辑
}

// 读取原始数据
void readRawData() {
  // 读取加速度数据
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // 从ACCEL_XOUT_H寄存器开始
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);  // 读取6个寄存器
  
  // 对于±4g范围，需要除以8192.0
  AccX = (Wire.read() << 8 | Wire.read()) / 8192.0;  // X轴加速度
  AccY = (Wire.read() << 8 | Wire.read()) / 8192.0;  // Y轴加速度
  AccZ = (Wire.read() << 8 | Wire.read()) / 8192.0;  // Z轴加速度
  
  // 读取陀螺仪数据
  Wire.beginTransmission(MPU);
  Wire.write(0x43);  // 从GYRO_XOUT_H寄存器开始
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);  // 读取6个寄存器
  
  // 对于±250deg/s范围，需要除以131.0
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
}

// 读取并校正数据
void readAndCorrectData() {
  // 读取原始数据
  readRawData();
  
  // 应用校准偏移量
  AccX -= offsetAccX;
  AccY -= offsetAccY;
  AccZ = AccZ - offsetAccZ; // Z轴已经在校准时减去了重力加速度
  GyroX -= offsetGyroX;
  GyroY -= offsetGyroY;
  GyroZ -= offsetGyroZ;
}

// 检测当前帧的运动状态并更新历史记录
void detectCurrentMotion() {
  // 计算加速度绝对值
  float absAccX = abs(AccX);
  float absAccY = abs(AccY);
  float absAccZ = abs(AccZ-1);
  
  // 计算角速度绝对值
  float absGyroX = abs(GyroX);
  float absGyroY = abs(GyroY);
  float absGyroZ = abs(GyroZ);
  
  // 检测上下跳动
  bool isUpDown = (absAccZ > MOTION_THRESHOLD_Z);
  
  // 检测左右晃动
  bool isLeftRight = (absGyroX > MOTION_THRESHOLD_GYRO || absGyroZ > MOTION_THRESHOLD_GYRO);
  
  // 更新历史记录
  upDownHistory[historyIndex] = isUpDown;
  leftRightHistory[historyIndex] = isLeftRight;
  
  // 更新历史索引
  historyIndex = (historyIndex + 1) % HISTORY_SIZE;
  
  // 输出当前帧检测结果（调试用）
  Serial.print("当前帧: 上下=");
  Serial.print(isUpDown ? "是" : "否");
  Serial.print(", 左右=");
  Serial.println(isLeftRight ? "是" : "否");
}

// 根据历史记录判断整体运动状态
int determineOverallState() {
  bool hasUpDown = false;
  bool hasLeftRight = false;
  
  // 检查历史记录中是否有上下跳动
  for (int i = 0; i < HISTORY_SIZE; i++) {
    if (upDownHistory[i]) {
      hasUpDown = true;
      break;
    }
  }
  
  // 检查历史记录中是否有左右晃动
  for (int i = 0; i < HISTORY_SIZE; i++) {
    if (leftRightHistory[i]) {
      hasLeftRight = true;
      break;
    }
  }
  
  // 优先判断上下跳动
  if (hasUpDown) {
    return STATE_UPDOWN;
  }
  
  // 其次判断左右晃动
  if (hasLeftRight) {
    return STATE_LEFTRIGHT;
  }
  
  // 如果都没有，则为静止状态
  return STATE_STABLE;
}
