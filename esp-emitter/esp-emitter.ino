// //84:FC:E6:09:D8:08
// #include <esp_now.h>
// #include <WiFi.h>

// // ******* 将这里替换为您接收方ESP32的实际MAC地址 *******
// uint8_t receiverMacAddress[] = {0x84, 0xFC, 0xE6, 0x09, 0xD8, 0x08}; // 请务必换为接收方真实MAC地址

// // 定义要发送的数据结构
// typedef struct struct_message {
//   int number;
// } struct_message;

// // 创建一个数据结构变量
// struct_message myData;

// // 发送回调函数，确认数据是否发送成功
// void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//   Serial.print("\r\nLast Packet Send Status:\t");
//   Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
// }

// // 用于循环发送的计数器
// int counter = 0;

// void setup() {
//   // 初始化串口通信
//   Serial.begin(115200);

//   // 设置WiFi模式为Station
//   WiFi.mode(WIFI_STA);

//   // 初始化ESP-NOW
//   if (esp_now_init() != ESP_OK) {
//     Serial.println("Error initializing ESP-NOW");
//     return;
//   }
//   Serial.println("ESP-NOW Initialized.");

//   // 注册发送回调函数
//   esp_now_register_send_cb(OnDataSent);

//   // 注册接收方为peer
//   esp_now_peer_info_t peerInfo;
//   memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
//   peerInfo.channel = 0; // 使用WiFi当前信道
//   peerInfo.encrypt = false;

//   // 添加peer
//   if (esp_now_add_peer(&peerInfo) != ESP_OK){
//     Serial.println("Failed to add peer");
//     return;
//   }
//   Serial.println("Peer Added.");
// }

// void loop() {
//   // 设置要发送的数字 (0, 1, 2, 3 循环)
//   myData.number = counter;

//   // 发送消息
//   esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *) &myData, sizeof(myData));

//   if (result == ESP_OK) {
//     Serial.print("Sent number: ");
//     Serial.println(myData.number);
//   }
//   else {
//     Serial.print("Error sending the data. Error code: ");
//     Serial.println(result);
//   }

//   // 更新计数器
//   counter++;
//   if (counter > 3) {
//     counter = 0; // 重置计数器
//   }

//   // 等待一段时间再发送下一个数字
//   delay(2000); // 每2秒发送一次
// }

#include <WiFi.h>
#include <WiFiClient.h>

// --- WiFi Station Settings ---
const char *ssid = "ESP32_AP_Server"; // Must match the server's SSID
const char *password = "123456789"; // Must match the server's password

// --- Server Details ---
const char *serverIP = "192.168.4.1"; // Default IP address of the ESP32 Soft AP
const uint ServerPort = 8080;       // Must match the server's port

WiFiClient client; // Create a WiFiClient object

int counter = 0; // Counter for sending 0, 1, 2, 3

void setup() {
  Serial.begin(115200);
  Serial.println();

  // Connect to Server WiFi Access Point
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  Serial.print("Connecting to server ");
  Serial.print(serverIP);
  Serial.print(":");
  Serial.println(ServerPort);

  // Use client.connect() to establish a connection
  if (client.connect(serverIP, ServerPort)) {
    Serial.println("Connected to server!");

    // Prepare the number to send (0, 1, 2, or 3) as a character
    char numberToSend = (counter % 4) + '0'; // Convert int 0-3 to char '0'-'3'

    // Send the data
    client.print(numberToSend); // Send the character
    Serial.print("Sent number: ");
    Serial.println(numberToSend - '0'); // Print the integer value

    // Optional: wait a bit for data to be sent before stopping
    delay(100);

    // Close the connection
    client.stop();
    Serial.println("Connection closed.");

  } else {
    Serial.println("Connection failed!");
    // Optional: Add a small delay before retrying if connection failed
    delay(1000);
  }

  // Increment counter for next iteration
  counter++;
  if (counter > 3) {
      counter = 0;
  }

  // Wait before sending the next number
  delay(2000); // Send every 2 seconds
}
