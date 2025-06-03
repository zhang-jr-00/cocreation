// #include <WiFi.h>

// // --- WiFi Access Point Settings ---
// const char *ssid = "ESP32_AP_Server";
// const char *password = "123456789";

// // --- Server Settings ---
// const uint ServerPort = 8080;
// WiFiServer server(ServerPort);

// // --- Client Management ---
// WiFiClient clientForce;
// WiFiClient clientEmitterL;
// WiFiClient clientEmitterR;

// // Flags to track connection status
// bool forceConnected = false;
// bool emitterLConnected = false;
// bool emitterRConnected = false;

// // Expected identifiers
// char forceID = 'F';
// char emitterL_ID = 'L';
// char emitterR_ID = 'R';

// // --- State ---
// int selectedEmitter = 0; // 0 = None, 1 = Left, 2 = Right

// // --- Status Printing Timing ---
// unsigned long lastStatusPrintTime = 0;
// const unsigned long statusPrintInterval = 5000; // Print status every 5 seconds

// //===============================================================
// // SETUP
// //===============================================================
// void setup() {
//   Serial.begin(115200);
//   Serial.println("\n--- Multi-Client TCP Server Receiver (w/ Status & Feedback) ---");

//   // Set up Access Point
//   Serial.print("Setting up Access Point: "); Serial.println(ssid);
//   if (!WiFi.softAP(ssid, password)) {
//       Serial.println("AP Setup Failed!"); while(1) delay(1000);
//   }
//   Serial.print("AP IP address: "); Serial.println(WiFi.softAPIP());

//   // Start TCP server
//   server.begin();
//   Serial.print("TCP Server started on port: "); Serial.println(ServerPort);
//   Serial.println("Waiting for clients (Force='F', EmitterL='L', EmitterR='R')...");
// }

// //===============================================================
// // MAIN LOOP
// //===============================================================
// void loop() {
//   unsigned long currentMillis = millis();

//   handleNewConnections();       // Check for and identify new clients
//   handleClientData();           // Process data from connected clients
//   cleanupDisconnectedClients(); // Check for and handle disconnections

//   // Periodic Status Print
//   if (currentMillis - lastStatusPrintTime >= statusPrintInterval) {
//     lastStatusPrintTime = currentMillis;
//     printConnectionStatus();
//   }

//   delay(10); // Small delay for stability
// }

// //===============================================================
// // Function to Send Selection State Back to Emitters
// //===============================================================
// void sendSelectionStateToEmitters() {
//   char stateForL = (selectedEmitter == 1) ? '1' : '0'; // '1' if L selected, else '0'
//   char stateForR = (selectedEmitter == 2) ? '1' : '0'; // '1' if R selected, else '0'

//   // Send to Emitter L if connected
//   if (emitterLConnected && clientEmitterL.connected()) {
//     Serial.print(">>> Sending selection state '");
//     Serial.print(stateForL);
//     Serial.println("' back to Emitter-L <<<");
//     clientEmitterL.print(stateForL);
//     // Optional: Check connection immediately after sending
//     if (!clientEmitterL.connected()) {
//         Serial.println("!!! Emitter-L disconnected immediately after sending feedback.");
//         emitterLConnected = false; // Update status if disconnected
//     }
//   }

//   // Send to Emitter R if connected
//   if (emitterRConnected && clientEmitterR.connected()) {
//     Serial.print(">>> Sending selection state '");
//     Serial.print(stateForR);
//     Serial.println("' back to Emitter-R <<<");
//     clientEmitterR.print(stateForR);
//     // Optional: Check connection immediately after sending
//     if (!clientEmitterR.connected()) {
//         Serial.println("!!! Emitter-R disconnected immediately after sending feedback.");
//         emitterRConnected = false; // Update status if disconnected
//     }
//   }
// }

// //===============================================================
// // Function to Handle New Client Connections
// //===============================================================
// void handleNewConnections() {
//   WiFiClient newClient = server.available();

//   if (newClient) {
//     Serial.println("\n--------------------------");
//     Serial.print("New client trying to connect! IP: ");
//     Serial.println(newClient.remoteIP());

//     unsigned long startTime = millis();
//     while (!newClient.available() && millis() - startTime < 1000) { delay(10); }

//     if (newClient.available()) {
//       char id = newClient.read();
//       Serial.print("Client sent identifier: '"); Serial.print(id); Serial.println("'");

//       bool assigned = false; // Flag to track if assigned

//       if (id == forceID && !forceConnected) {
//         clientForce = newClient; forceConnected = true; assigned = true;
//         Serial.println(">>> Assigned as Force Sensor ('F') Client.");
//       } else if (id == emitterL_ID && !emitterLConnected) {
//         clientEmitterL = newClient; emitterLConnected = true; assigned = true;
//         Serial.println(">>> Assigned as Emitter-Left ('L') Client.");
//         // Send initial state back to newly connected Emitter L
//         sendSelectionStateToEmitters();
//       } else if (id == emitterR_ID && !emitterRConnected) {
//         clientEmitterR = newClient; emitterRConnected = true; assigned = true;
//         Serial.println(">>> Assigned as Emitter-Right ('R') Client.");
//         // Send initial state back to newly connected Emitter R
//         sendSelectionStateToEmitters();
//       }

//       if (!assigned) { // If not assigned (unknown ID or duplicate)
//         Serial.print("!!! Unknown identifier or client type '"); Serial.print(id);
//         Serial.println("' already connected. Refusing connection.");
//         newClient.stop();
//       }
//     } else {
//       Serial.println("!!! Client did not send identifier within timeout. Disconnecting.");
//       newClient.stop();
//     }
//      Serial.println("--------------------------");
//   }
// }

// //===============================================================
// // Function to Process Data from Connected Clients
// //===============================================================
// void handleClientData() {
//   // --- Handle Force Sensor Client ---
//   if (forceConnected && clientForce.connected() && clientForce.available()) {
//     bool stateChanged = false; // Flag to check if selection changed
//     while (clientForce.available()) {
//       char receivedChar = clientForce.read();
//       Serial.print("Data from Force Sensor ('F'): '");
//       Serial.print(receivedChar); Serial.println("'");

//       if (receivedChar == '1') {
//         if (selectedEmitter != 1) {
//           Serial.println("==> Selecting LEFT Emitter <==");
//           selectedEmitter = 1;
//           stateChanged = true; // Mark state as changed
//         }
//       } else if (receivedChar == '2') {
//         if (selectedEmitter != 2) {
//           Serial.println("==> Selecting RIGHT Emitter <==");
//           selectedEmitter = 2;
//           stateChanged = true; // Mark state as changed
//         }
//       } else if (receivedChar == '0') { // Optional: Handle explicit '0' to deselect
//          if (selectedEmitter != 0) {
//             Serial.println("==> Deselecting Emitter (None) <==");
//             selectedEmitter = 0;
//             stateChanged = true;
//          }
//       } else if (receivedChar != '\n' && receivedChar != '\r') {
//          Serial.println("  (Ignoring unexpected force data)");
//       }
//     }
//     // Send feedback to emitters IF the state changed
//     if (stateChanged) {
//         sendSelectionStateToEmitters();
//     }
//   }

//   // --- Handle Emitter-Left Client ---
//   if (emitterLConnected && clientEmitterL.connected() && clientEmitterL.available()) {
//     while (clientEmitterL.available()) {
//       char receivedChar = clientEmitterL.read();
//       Serial.print("Data from Emitter-Left ('L'): '");
//       Serial.print(receivedChar); Serial.print("'");

//       if (selectedEmitter == 1) { // Only process if L is selected
//         if (receivedChar >= '0' && receivedChar <= '2') {
//           int motionState = receivedChar - '0';
//           Serial.print(" [Processing Left State="); Serial.print(motionState); Serial.println("]");
//           // *** ADD YOUR ACTION BASED ON LEFT EMITTER'S STATE HERE ***
//         } else if (receivedChar != '\n' && receivedChar != '\r') {
//            Serial.println(" [Ignoring unexpected emitter data]");
//         } else { Serial.println(); } // Just newline
//       } else { Serial.println(" [Ignoring - Not Selected]"); }
//     }
//   }

//   // --- Handle Emitter-Right Client ---
//   if (emitterRConnected && clientEmitterR.connected() && clientEmitterR.available()) {
//      while (clientEmitterR.available()) {
//       char receivedChar = clientEmitterR.read();
//       Serial.print("Data from Emitter-Right ('R'): '");
//       Serial.print(receivedChar); Serial.print("'");

//       if (selectedEmitter == 2) { // Only process if R is selected
//          if (receivedChar >= '0' && receivedChar <= '2') {
//           int motionState = receivedChar - '0';
//           Serial.print(" [Processing Right State="); Serial.print(motionState); Serial.println("]");
//           // *** ADD YOUR ACTION BASED ON RIGHT EMITTER'S STATE HERE ***
//          } else if (receivedChar != '\n' && receivedChar != '\r') {
//             Serial.println(" [Ignoring unexpected emitter data]");
//          } else { Serial.println(); } // Just newline
//       } else { Serial.println(" [Ignoring - Not Selected]"); }
//     }
//   }
// }

// //===============================================================
// // Function to Check and Cleanup Disconnected Clients
// //===============================================================
// void cleanupDisconnectedClients() {
//     if (forceConnected && !clientForce.connected()) {
//         Serial.println("\n--------------------------");
//         Serial.println("!!! Force Sensor ('F') Client Disconnected.");
//         Serial.println("--------------------------");
//         forceConnected = false;
//     }
//     if (emitterLConnected && !clientEmitterL.connected()) {
//         Serial.println("\n--------------------------");
//         Serial.println("!!! Emitter-Left ('L') Client Disconnected.");
//         Serial.println("--------------------------");
//         emitterLConnected = false;
//     }
//     if (emitterRConnected && !clientEmitterR.connected()) {
//         Serial.println("\n--------------------------");
//         Serial.println("!!! Emitter-Right ('R') Client Disconnected.");
//         Serial.println("--------------------------");
//         emitterRConnected = false;
//     }
// }

// //===============================================================
// // Function to Print Current Connection Status
// //===============================================================
// void printConnectionStatus() {
//     Serial.print("[Status] Connected Clients: ");
//     bool anyConnected = false;
//     if (forceConnected) { Serial.print("Force('F') "); anyConnected = true;}
//     if (emitterLConnected) { Serial.print("EmitterL('L') "); anyConnected = true;}
//     if (emitterRConnected) { Serial.print("EmitterR('R') "); anyConnected = true;}
//     if (!anyConnected) { Serial.print("None"); }
//     Serial.print(" | Selected Emitter: ");
//     if (selectedEmitter == 1) { Serial.println("Left ('L')"); }
//     else if (selectedEmitter == 2) { Serial.println("Right ('R')"); }
//     else { Serial.println("None"); }
// }




#include <WiFi.h>
#include <ESP32Servo.h> // <--- 包含 ESP32 Servo 库

// --- WiFi Access Point Settings ---
const char *ssid = "ESP32_AP_Server";
const char *password = "123456789";

// --- Server Settings ---
const uint ServerPort = 8080;
WiFiServer server(ServerPort);

// --- Client Management ---
WiFiClient clientForce;
WiFiClient clientEmitterL;
WiFiClient clientEmitterR;

// Flags to track connection status
bool forceConnected = false;
bool emitterLConnected = false;
bool emitterRConnected = false;

// Expected identifiers
char forceID = 'F';
char emitterL_ID = 'L';
char emitterR_ID = 'R';

// --- State ---
int selectedEmitter = 0; // 0 = None, 1 = Left, 2 = Right

// --- Status Printing Timing ---
unsigned long lastStatusPrintTime = 0;
const unsigned long statusPrintInterval = 5000; // Print status every 5 seconds

// --- Motor Control Settings ---
Servo motorfront; // <--- 前电机对象
Servo motorback;  // <--- 后电机对象

// 连接电机控制信号线的 ESP32 GPIO 引脚 (根据实际连接修改)
const int motorFrontPin = 13; // 例如 GPIO13
const int motorBackPin = 12;  // 例如 GPIO12 (确保与 motorFrontPin 不同)

// 电机控制信号值 (单位：微秒 microseconds)
const int MOTOR_STOP_SIGNAL = 1500;    // 停止信号
const int MOTOR_FORWARD_SIGNAL = 2000; // 正转信号 (假设两个电机都用这个信号正转)
// 如果需要反转，可以定义: const int MOTOR_REVERSE_SIGNAL = 1000;

//===============================================================
// Helper Function to Stop Both Motors
//===============================================================
void stopBothMotors() {
  Serial.println("[Motor Action] Stopping both motors.");
  if (motorfront.attached()) { // 检查是否已附加
      motorfront.writeMicroseconds(MOTOR_STOP_SIGNAL);
  }
  if (motorback.attached()) { // 检查是否已附加
      motorback.writeMicroseconds(MOTOR_STOP_SIGNAL);
  }
}

//===============================================================
// Helper Function to Set Motor State based on Emitter Command
//===============================================================
void setMotorState(int state) {
  Serial.print("[Motor Action] Setting state to: "); Serial.println(state);
  if (!motorfront.attached() || !motorback.attached()) {
      Serial.println("!!! Error: Motors not attached!");
      return; // 如果电机未附加，则不执行任何操作
  }

  switch (state) {
    case 0: // 停止
      stopBothMotors();
      break;
    case 1: // 只有 motorfront 转动
      Serial.println("  -> MotorFront: ON, MotorBack: OFF");
      motorfront.writeMicroseconds(MOTOR_FORWARD_SIGNAL);
      motorback.writeMicroseconds(MOTOR_STOP_SIGNAL);
      break;
    case 2: // 只有 motorback 转动
      Serial.println("  -> MotorFront: OFF, MotorBack: ON");
      motorfront.writeMicroseconds(MOTOR_STOP_SIGNAL);
      motorback.writeMicroseconds(MOTOR_FORWARD_SIGNAL);
      break;
    default: // 未知状态，安全起见停止
      Serial.print("  -> Unknown state ("); Serial.print(state); Serial.println("), stopping motors.");
      stopBothMotors();
      break;
  }
}


//===============================================================
// SETUP
//===============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Multi-Client TCP Server + Motor Control ---");

  // --- Initialize Motors ---
  Serial.println("Initializing Motors...");
  // 为 ESP32Servo 分配定时器 (通常推荐)
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // 设置 PWM 频率 (标准 50Hz)
  motorfront.setPeriodHertz(50);
  motorback.setPeriodHertz(50);

  // 附加电机到引脚 (设置脉宽范围 1000-2000us)
  motorfront.attach(motorFrontPin, 1000, 2000);
  motorback.attach(motorBackPin, 1000, 2000);

  Serial.print("MotorFront on GPIO "); Serial.print(motorFrontPin);
  Serial.print(", MotorBack on GPIO "); Serial.println(motorBackPin);

  // 确保启动时电机停止
  stopBothMotors();
  delay(500); // 短暂延时

  // --- Set up Access Point ---
  Serial.print("Setting up Access Point: "); Serial.println(ssid);
  if (!WiFi.softAP(ssid, password)) {
      Serial.println("AP Setup Failed!"); while(1) delay(1000);
  }
  Serial.print("AP IP address: "); Serial.println(WiFi.softAPIP());

  // --- Start TCP server ---
  server.begin();
  Serial.print("TCP Server started on port: "); Serial.println(ServerPort);
  Serial.println("Waiting for clients (Force='F', EmitterL='L', EmitterR='R')...");
}

//===============================================================
// MAIN LOOP
//===============================================================
void loop() {
  unsigned long currentMillis = millis();

  handleNewConnections();       // Check for and identify new clients
  handleClientData();           // Process data from connected clients
  cleanupDisconnectedClients(); // Check for and handle disconnections

  // Periodic Status Print
  if (currentMillis - lastStatusPrintTime >= statusPrintInterval) {
    lastStatusPrintTime = currentMillis;
    printConnectionStatus();
  }

  delay(10); // Small delay for stability
}

//===============================================================
// Function to Send Selection State Back to Emitters
//===============================================================
void sendSelectionStateToEmitters() {
  char stateForL = (selectedEmitter == 1) ? '1' : '0'; // '1' if L selected, else '0'
  char stateForR = (selectedEmitter == 2) ? '1' : '0'; // '1' if R selected, else '0'

  if (emitterLConnected && clientEmitterL.connected()) {
    clientEmitterL.print(stateForL);
    Serial.print(">>> Feedback Sent -> Emitter-L: '"); Serial.print(stateForL); Serial.println("'");
  }

  if (emitterRConnected && clientEmitterR.connected()) {
    clientEmitterR.print(stateForR);
     Serial.print(">>> Feedback Sent -> Emitter-R: '"); Serial.print(stateForR); Serial.println("'");
  }
}

//===============================================================
// Function to Handle New Client Connections
//===============================================================
// void handleNewConnections() {
//     // ... (这部分代码与你原来的相同，无需修改) ...
//   WiFiClient newClient = server.available();

//   if (newClient) {
//     Serial.println("\n--------------------------");
//     Serial.print("New client trying to connect! IP: ");
//     Serial.println(newClient.remoteIP());

//     unsigned long startTime = millis();
//     while (!newClient.available() && millis() - startTime < 1000) { delay(10); }

//     if (newClient.available()) {
//       char id = newClient.read();
//       Serial.print("Client sent identifier: '"); Serial.print(id); Serial.println("'");

//       bool assigned = false; // Flag to track if assigned

//       if (id == forceID && !forceConnected) {
//         clientForce = newClient; forceConnected = true; assigned = true;
//         Serial.println(">>> Assigned as Force Sensor ('F') Client.");
//       } else if (id == emitterL_ID && !emitterLConnected) {
//         clientEmitterL = newClient; emitterLConnected = true; assigned = true;
//         Serial.println(">>> Assigned as Emitter-Left ('L') Client.");
//         // Send initial state back to newly connected Emitter L
//         sendSelectionStateToEmitters();
//       } else if (id == emitterR_ID && !emitterRConnected) {
//         clientEmitterR = newClient; emitterRConnected = true; assigned = true;
//         Serial.println(">>> Assigned as Emitter-Right ('R') Client.");
//         // Send initial state back to newly connected Emitter R
//         sendSelectionStateToEmitters();
//       }

//       if (!assigned) { // If not assigned (unknown ID or duplicate)
//         Serial.print("!!! Unknown identifier or client type '"); Serial.print(id);
//         Serial.println("' already connected. Refusing connection.");
//         newClient.stop();
//       }
//     } else {
//       Serial.println("!!! Client did not send identifier within timeout. Disconnecting.");
//       newClient.stop();
//     }
//      Serial.println("--------------------------");
//   }
// }
//===============================================================
// Function to Handle New Client Connections (允许覆盖旧连接)
//===============================================================
void handleNewConnections() {
  WiFiClient newClient = server.available();

  if (newClient) {
    Serial.println("\n--------------------------");
    Serial.print("New client trying to connect! IP: ");
    Serial.println(newClient.remoteIP());

    // 给客户端一点时间发送它的标识符
    unsigned long startTime = millis();
    while (!newClient.available() && millis() - startTime < 1000) { // 等待最多1秒
        delay(10);
    }

    if (newClient.available()) {
      char id = newClient.read(); // 读取标识符
      Serial.print("Client sent identifier: '"); Serial.print(id); Serial.println("'");

      bool assigned = false; // 标记是否成功分配

      // --- 处理 Force Sensor ('F') ---
      if (id == forceID) {
        if (forceConnected) { // 如果 'F' 已经连接过
          Serial.println(">>> Force client ('F') is reconnecting. Closing old connection first.");
          clientForce.stop(); // **显式关闭旧的连接对象**
        } else {
          Serial.println(">>> Force client ('F') connected for the first time.");
        }
        clientForce = newClient;     // **用新的连接替换（或设置）**
        forceConnected = true;       // 确保标志为 true
        assigned = true;
        Serial.println(">>> Assigned as Force Sensor ('F') Client.");
        // 当控制端重连时，重置选择并停止电机可能是最安全的
        stopBothMotors();
        selectedEmitter = 0;
        sendSelectionStateToEmitters(); // 通知 Emitters 状态变化

      // --- 处理 Emitter-Left ('L') ---
      } else if (id == emitterL_ID) {
        if (emitterLConnected) { // 如果 'L' 已经连接过
          Serial.println(">>> Emitter-Left ('L') is reconnecting. Closing old connection first.");
          clientEmitterL.stop(); // **显式关闭旧的连接对象**
          // 如果重连的是当前选中的 Emitter，先停止电机
          if (selectedEmitter == 1) {
              Serial.println(">>> Stopping motors as selected Emitter-L is reconnecting.");
              stopBothMotors();
              // selectedEmitter 保持为 1 或根据需要重置，这里保持可能更好，让它重连后能立即控制
          }
        } else {
           Serial.println(">>> Emitter-Left ('L') connected for the first time.");
        }
        clientEmitterL = newClient;  // **用新的连接替换（或设置）**
        emitterLConnected = true;    // 确保标志为 true
        assigned = true;
        Serial.println(">>> Assigned as Emitter-Left ('L') Client.");
        // 发送当前选择状态给这个刚（重）连接的 Emitter
        sendSelectionStateToEmitters();

      // --- 处理 Emitter-Right ('R') ---
      } else if (id == emitterR_ID) {
        if (emitterRConnected) { // 如果 'R' 已经连接过
          Serial.println(">>> Emitter-Right ('R') is reconnecting. Closing old connection first.");
          clientEmitterR.stop(); // **显式关闭旧的连接对象**
           // 如果重连的是当前选中的 Emitter，先停止电机
          if (selectedEmitter == 2) {
              Serial.println(">>> Stopping motors as selected Emitter-R is reconnecting.");
              stopBothMotors();
               // selectedEmitter 保持为 2
          }
        } else {
            Serial.println(">>> Emitter-Right ('R') connected for the first time.");
        }
        clientEmitterR = newClient; // **用新的连接替换（或设置）**
        emitterRConnected = true;   // 确保标志为 true
        assigned = true;
        Serial.println(">>> Assigned as Emitter-Right ('R') Client.");
        // 发送当前选择状态给这个刚（重）连接的 Emitter
        sendSelectionStateToEmitters();
      }

      // --- 处理未分配的情况 ---
      if (!assigned) {
        // 现在这个分支只会捕获真正未知的标识符
        Serial.print("!!! Unknown identifier '"); Serial.print(id);
        Serial.println("'. Refusing connection.");
        newClient.stop(); // 关闭这个无法识别的客户端连接
      }
    } else {
      Serial.println("!!! Client connected but did not send identifier within timeout. Disconnecting.");
      newClient.stop(); // 关闭连接
    }
     Serial.println("--------------------------");
  }
}
//===============================================================
// Function to Process Data from Connected Clients
//===============================================================
void handleClientData() {
  // --- Handle Force Sensor Client ---
  if (forceConnected && clientForce.connected() && clientForce.available()) {
    bool stateChanged = false; // Flag to check if selection changed
    int previousSelectedEmitter = selectedEmitter; // Store previous state

    while (clientForce.available()) {
      char receivedChar = clientForce.read();
      Serial.print("Data from Force Sensor ('F'): '");
      Serial.print(receivedChar); Serial.println("'");

      if (receivedChar == '1') {
        if (selectedEmitter != 1) {
          Serial.println("==> Selecting LEFT Emitter <==");
          selectedEmitter = 1;
        }
      } else if (receivedChar == '2') {
        if (selectedEmitter != 2) {
          Serial.println("==> Selecting RIGHT Emitter <==");
          selectedEmitter = 2;
        }
      } else if (receivedChar == '0') {
         if (selectedEmitter != 0) {
            Serial.println("==> Deselecting Emitter (None) <==");
            selectedEmitter = 0;
         }
      } else if (receivedChar != '\n' && receivedChar != '\r') {
         Serial.println("  (Ignoring unexpected force data)");
      }
    }

    // Check if selection actually changed
    if (selectedEmitter != previousSelectedEmitter) {
        stateChanged = true;
        Serial.println(">>> Emitter selection changed <<<");
        stopBothMotors(); // *** 重要：选择变化时停止电机 ***
        sendSelectionStateToEmitters(); // 发送反馈给 Emitters
    }
  }

  // --- Handle Emitter-Left Client ---
  if (emitterLConnected && clientEmitterL.connected() && clientEmitterL.available()) {
    while (clientEmitterL.available()) {
      char receivedChar = clientEmitterL.read();
      Serial.print("Data from Emitter-Left ('L'): '");
      Serial.print(receivedChar);

      if (selectedEmitter == 1) { // 只有当 Left 被选中时才处理
        if (receivedChar >= '0' && receivedChar <= '2') {
          int motionState = receivedChar - '0';
          Serial.print("' [Processing Left State="); Serial.print(motionState); Serial.println("]");
          // *** 根据状态控制电机 ***
          setMotorState(motionState);
        } else if (receivedChar != '\n' && receivedChar != '\r') {
           Serial.println("' [Ignoring unexpected emitter data]");
        } else { Serial.println("'"); } // Just newline
      } else { Serial.println("' [Ignoring - Not Selected]"); }
    }
  }

  // --- Handle Emitter-Right Client ---
  if (emitterRConnected && clientEmitterR.connected() && clientEmitterR.available()) {
     while (clientEmitterR.available()) {
      char receivedChar = clientEmitterR.read();
      Serial.print("Data from Emitter-Right ('R'): '");
      Serial.print(receivedChar);

      if (selectedEmitter == 2) { // 只有当 Right 被选中时才处理
         if (receivedChar >= '0' && receivedChar <= '2') {
          int motionState = receivedChar - '0';
          Serial.print("' [Processing Right State="); Serial.print(motionState); Serial.println("]");
          // *** 根据状态控制电机 ***
          setMotorState(motionState);
         } else if (receivedChar != '\n' && receivedChar != '\r') {
            Serial.println("' [Ignoring unexpected emitter data]");
         } else { Serial.println("'"); } // Just newline
      } else { Serial.println("' [Ignoring - Not Selected]"); }
    }
  }
}

//===============================================================
// Function to Check and Cleanup Disconnected Clients
//===============================================================
void cleanupDisconnectedClients() {
    if (forceConnected && !clientForce.connected()) {
        Serial.println("\n--------------------------");
        Serial.println("!!! Force Sensor ('F') Client Disconnected.");
        Serial.println("!!! Stopping motors as control is lost.");
        Serial.println("--------------------------");
        stopBothMotors(); // <--- 停止电机
        selectedEmitter = 0; // <--- 重置选择状态
        forceConnected = false;
        sendSelectionStateToEmitters(); // 通知 Emitters
    }
    if (emitterLConnected && !clientEmitterL.connected()) {
        Serial.println("\n--------------------------");
        Serial.println("!!! Emitter-Left ('L') Client Disconnected.");
        if (selectedEmitter == 1) { // 如果断开的是当前选中的
           Serial.println("!!! Stopping motors as selected emitter disconnected.");
           stopBothMotors(); // <--- 停止电机
           selectedEmitter = 0; // <--- 重置选择状态
           sendSelectionStateToEmitters(); // 通知 (可能存在的) Emitter-R
        }
        Serial.println("--------------------------");
        emitterLConnected = false;
    }
    if (emitterRConnected && !clientEmitterR.connected()) {
        Serial.println("\n--------------------------");
        Serial.println("!!! Emitter-Right ('R') Client Disconnected.");
         if (selectedEmitter == 2) { // 如果断开的是当前选中的
           Serial.println("!!! Stopping motors as selected emitter disconnected.");
           stopBothMotors(); // <--- 停止电机
           selectedEmitter = 0; // <--- 重置选择状态
           sendSelectionStateToEmitters(); // 通知 (可能存在的) Emitter-L
        }
        Serial.println("--------------------------");
        emitterRConnected = false;
    }
}

//===============================================================
// Function to Print Current Connection Status
//===============================================================
void printConnectionStatus() {
    Serial.print("[Status] Connected Clients: ");
    bool anyConnected = false;
    if (forceConnected) { Serial.print("Force('F') "); anyConnected = true;}
    if (emitterLConnected) { Serial.print("EmitterL('L') "); anyConnected = true;}
    if (emitterRConnected) { Serial.print("EmitterR('R') "); anyConnected = true;}
    if (!anyConnected) { Serial.print("None"); }
    Serial.print(" | Selected Emitter: ");
    if (selectedEmitter == 1) { Serial.println("Left ('L')"); }
    else if (selectedEmitter == 2) { Serial.println("Right ('R')"); }
    else { Serial.println("None"); }

    // 可以选择在这里打印当前电机的目标信号值（用于调试）
    // Serial.print("  MotorFront Target: "); Serial.print(motorfront.readMicroseconds());
    // Serial.print(" us | MotorBack Target: "); Serial.println(motorback.readMicroseconds());
}