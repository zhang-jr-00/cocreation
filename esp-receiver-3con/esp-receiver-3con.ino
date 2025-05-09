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




// //ESP-NOW
// #include <esp_now.h>
// #include <WiFi.h>

// // 定义接收数据结构，必须与发送方匹配
// typedef struct struct_message {
//   int number;
// } struct_message;

// // 创建一个数据结构变量来存储接收到的数据
// struct_message myData;

// // ======== 修正后的接收回调函数 ========
// // 注意参数列表的变化：第一个参数是 const esp_now_recv_info * info
// void OnDataRecv(const esp_now_recv_info * info, const uint8_t *incomingData, int len) {
//   // 从 info 结构体中获取发送方的 MAC 地址
//   Serial.print("Packet received from: ");
//   char macStr[18];
//   snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
//            info->src_addr[0], info->src_addr[1], info->src_addr[2],
//            info->src_addr[3], info->src_addr[4], info->src_addr[5]);
//   Serial.println(macStr);

//   // 检查数据长度是否与预期结构体大小一致
//   if (len == sizeof(myData)) {
//     memcpy(&myData, incomingData, sizeof(myData)); // 复制接收到的数据
//     Serial.print("Bytes received: ");
//     Serial.println(len);
//     Serial.print("Received number: ");
//     Serial.println(myData.number); // 打印接收到的数字
//     Serial.println();
//   } else {
//     Serial.print("Received data of unexpected length: ");
//     Serial.println(len);
//   }
// }

// void setup() {
//   // 初始化串口通信
//   Serial.begin(115200);
//   Serial.println("ESP-NOW Receiver");

//   // 设置WiFi模式为Station
//   WiFi.mode(WIFI_STA);
//   Serial.print("Receiver MAC Address: ");
//   Serial.println(WiFi.macAddress()); // 再次打印本机MAC地址以供核对

//   // 初始化ESP-NOW
//   if (esp_now_init() != ESP_OK) {
//     Serial.println("Error initializing ESP-NOW");
//     return;
//   }
//   Serial.println("ESP-NOW Initialized.");

//   // ======== 使用正确的函数签名注册接收回调 ========
//   if (esp_now_register_recv_cb(OnDataRecv) != ESP_OK) {
//       Serial.println("Failed to register receive callback");
//       return;
//   }
//    Serial.println("Receive callback registered.");
// }

// void loop() {
//   // ESP-NOW的数据接收是在回调函数中异步处理的，主循环可以留空
//   delay(10); // 短暂延时，给系统处理时间
// }

// #include <WiFi.h>
// // #include <WiFiServer.h> // WiFi.h usually includes this, but uncomment if needed

// // --- WiFi Access Point Settings ---
// // These MUST match the settings the sender is trying to connect to
// const char *ssid = "ESP32_AP_Server";
// const char *password = "123456789";

// // --- Server Settings ---
// const uint ServerPort = 8080; // The port the sender is sending data to
// WiFiServer server(ServerPort); // Create a WiFiServer object on the specified port

// void setup() {
//   // Initialize Serial communication
//   Serial.begin(115200);
//   Serial.println("\n--- WiFi TCP Server Receiver ---");

//   // Set device as a Wi-Fi Access Point
//   Serial.print("Setting up Access Point: ");
//   Serial.println(ssid);
//   // Start the Access Point. Default IP is 192.168.4.1
//   if (!WiFi.softAP(ssid, password)) {
//       Serial.println("Failed to start Access Point!");
//       // Optional: Halt or restart if AP fails
//       while(1) delay(1000);
//   }

//   // Print the IP address of the Access Point
//   Serial.print("AP IP address: ");
//   Serial.println(WiFi.softAPIP()); // Should print 192.168.4.1

//   // Start the TCP server
//   server.begin();
//   Serial.print("TCP Server started on port: ");
//   Serial.println(ServerPort);
// }

// void loop() {
//   // Check if a client has connected
//   WiFiClient client = server.available(); // Listen for incoming clients

//   if (client) { // If a new client connects,
//     Serial.println("New Client Connected!");
//     String clientInfo = "  Client IP: " + client.remoteIP().toString();
//     Serial.println(clientInfo);

//     unsigned long connectionStartTime = millis();
//     const unsigned long connectionTimeout = 2000; // Timeout after 2 seconds if no data

//     // Loop while the client is connected and within timeout
//     while (client.connected()) {
//         // Check if data is available to read from the client
//         if (client.available()) {
//             char receivedChar = client.read(); // Read the incoming byte (character)

//             Serial.print("  Received character: '");
//             Serial.print(receivedChar);
//             Serial.println("'");

//             // Process the received character (motion state)
//             // Example: Convert char '0','1','2' to int 0,1,2
//             if (receivedChar >= '0' && receivedChar <= '2') {
//                 int motionState = receivedChar - '0';
//                 Serial.print("  Interpreted Motion State: ");
//                 Serial.println(motionState);
//                 // --- Add your logic here based on the received motionState ---
//                 // e.g., control LEDs, motors, print to display, etc.

//             } else {
//                 Serial.println("  Received unexpected character.");
//             }

//             // Since the sender sends one char per connection, we can break after reading
//             break; // Exit the inner while loop after processing data
//         }

//         // Check for connection timeout
//         if (millis() - connectionStartTime > connectionTimeout) {
//             Serial.println("  Client connection timeout.");
//             break;
//         }

//         delay(10); // Small delay to prevent busy-waiting
//     }

//     // Close the connection with the client
//     client.stop();
//     Serial.println("Client Disconnected.");
//     Serial.println("--------------------------");
//   }

//   // Give the ESP a moment to handle background tasks
//   delay(10);
// }




#include <WiFi.h>
// #include <WiFiServer.h> // Usually included by WiFi.h, uncomment if needed

// --- WiFi Access Point Settings ---
// These MUST match the settings the sender/emitter is trying to connect to
const char *ssid = "ESP32_AP_Server";
const char *password = "123456789";

// --- Server Settings ---
const uint ServerPort = 8080; // The port the sender/emitter is sending data to
WiFiServer server(ServerPort); // Create a WiFiServer object

// --- Global Client Object ---
// This will hold the currently connected client across loop iterations
WiFiClient client;

//===============================================================
// SETUP
//===============================================================
void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  Serial.println("\n--- WiFi TCP Server Receiver (Stable Connection) ---");

  // Set device as a Wi-Fi Access Point
  Serial.print("Setting up Access Point: ");
  Serial.println(ssid);
  // Start the Access Point. Default IP should be 192.168.4.1
  if (!WiFi.softAP(ssid, password)) {
      Serial.println("Failed to start Access Point!");
      while(1) delay(1000); // Halt if AP fails
  }

  // Print the IP address of the Access Point
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP()); // Confirm it matches sender's target IP

  // Start the TCP server
  server.begin();
  Serial.print("TCP Server started on port: ");
  Serial.println(ServerPort);
  Serial.println("Waiting for a client connection...");
  Serial.println("--------------------------");
}

//===============================================================
// MAIN LOOP
//===============================================================
void loop() {

  // --- Client Connection Handling ---
  // Check if a new client is trying to connect, ONLY if we don't currently have a valid, connected client.
  if (!client || !client.connected()) {
    // Check if we had a client before that just disconnected
    if (client) { // If the client object exists but is not connected
        Serial.println("Client Disconnected.");
        client.stop(); // Ensure resources are freed
        Serial.println("--------------------------");
        Serial.println("Waiting for a new client connection...");
    }

    // Listen for a new incoming client connection
    client = server.available(); // Assigns a new client if one is connecting

    // Check if a new client has actually connected in this iteration
    if (client && client.connected()) {
        Serial.println("New Client Connected!");
        String clientInfo = "  Client IP: " + client.remoteIP().toString();
        Serial.println(clientInfo);
    }
  }

  // --- Data Handling for Connected Client ---
  // If we have a client object AND that client is currently connected
  if (client && client.connected()) {
      // Check if data is available to read from this client
      if (client.available()) {
          Serial.print("Data received: ");
          // Read all available bytes (in case multiple chars arrive closely)
          while(client.available()){
             char receivedChar = client.read(); // Read one byte/character

             Serial.print("'"); Serial.p+rint(receivedChar); Serial.print("' "); // Print raw char

             // Process the received character (expecting '0', '1', or '2')
             if (receivedChar >= '0' && receivedChar <= '2') {
                 int motionState = receivedChar - '0'; // Convert char to int
                 Serial.print("[State="); Serial.print(motionState); Serial.print("] ");

                 // +++ Add your specific actions based on motionState here +++
                 // Example:
                 // if (motionState == STATE_STABLE) { /* do stable action */ }
                 // else if (motionState == STATE_UPDOWN) { /* do up/down action */ }
                 // else if (motionState == STATE_LEFTRIGHT) { /* do left/right action */ }
                 // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


             } else if (receivedChar != '\n' && receivedChar != '\r'){ // Ignore newline/carriage return if any sent
                 Serial.print("[Unexpected Char] ");
             }
          } // End while(client.available())
          Serial.println(); // Newline after processing received data chunk
      }
      // --- We DO NOT call client.stop() here - keep connection open ---
  }

  // Small delay to prevent the loop from running too fast and consuming 100% CPU
  delay(10);

} // End Main Loop
