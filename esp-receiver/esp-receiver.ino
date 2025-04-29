// #include <WiFi.h>

// void setup(){
//   Serial.begin(115200);
//   WiFi.mode(WIFI_STA); // 设置为Station模式以获取MAC地址
//   Serial.println("ESP32 Board MAC Address: ");
//   Serial.println(WiFi.macAddress());
// }

// void loop(){
//   // 不需要循环执行
// }


#include <esp_now.h>
#include <WiFi.h>

// 定义接收数据结构，必须与发送方匹配
typedef struct struct_message {
  int number;
} struct_message;

// 创建一个数据结构变量来存储接收到的数据
struct_message myData;

// ======== 修正后的接收回调函数 ========
// 注意参数列表的变化：第一个参数是 const esp_now_recv_info * info
void OnDataRecv(const esp_now_recv_info * info, const uint8_t *incomingData, int len) {
  // 从 info 结构体中获取发送方的 MAC 地址
  Serial.print("Packet received from: ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);
  Serial.println(macStr);

  // 检查数据长度是否与预期结构体大小一致
  if (len == sizeof(myData)) {
    memcpy(&myData, incomingData, sizeof(myData)); // 复制接收到的数据
    Serial.print("Bytes received: ");
    Serial.println(len);
    Serial.print("Received number: ");
    Serial.println(myData.number); // 打印接收到的数字
    Serial.println();
  } else {
    Serial.print("Received data of unexpected length: ");
    Serial.println(len);
  }
}

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  Serial.println("ESP-NOW Receiver");

  // 设置WiFi模式为Station
  WiFi.mode(WIFI_STA);
  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress()); // 再次打印本机MAC地址以供核对

  // 初始化ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW Initialized.");

  // ======== 使用正确的函数签名注册接收回调 ========
  if (esp_now_register_recv_cb(OnDataRecv) != ESP_OK) {
      Serial.println("Failed to register receive callback");
      return;
  }
   Serial.println("Receive callback registered.");
}

void loop() {
  // ESP-NOW的数据接收是在回调函数中异步处理的，主循环可以留空
  delay(10); // 短暂延时，给系统处理时间
}
