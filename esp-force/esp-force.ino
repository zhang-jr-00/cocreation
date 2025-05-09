// #include <WiFi.h>
// #include <WiFiClient.h>

// // --- Sensor Pins ---
// const int FORCE_SENSOR_LEFT_PIN = 34;  // GPIO 34 (Analog input) - Check your board!
// const int FORCE_SENSOR_RIGHT_PIN =35; // GPIO 35 (Analog input) - Check your board!

// // --- Sensor Threshold ---
// // Adjust this value based on your sensor readings when pressed vs not pressed
// const int FORCE_THRESHOLD = 1500; // Example threshold (out of 4095 for ESP32 ADC)

// // --- WiFi Settings ---
// const char *ssid = "ESP32_AP_Server"; // Receiver's SSID
// const char *password = "123456789"; // Receiver's password
// const char *serverIP = "192.168.4.1"; // Receiver's IP address
// const uint ServerPort = 8080;       // Receiver's port

// // --- Client & State ---
// WiFiClient client;
// bool serverConnected = false;
// const char myIdentifier = 'F'; // Identifier for this Force-ESP
// unsigned long lastSendTime = 0;
// const unsigned long SEND_INTERVAL = 200; // How often to check sensors and potentially send (ms)
// char lastSentState = '0'; // Keep track of last sent state to avoid flooding

// //===============================================================
// // SETUP
// //===============================================================
// void setup() {
//   Serial.begin(115200);
//   Serial.println("\n--- Force Sensor ESP (ID: 'F') ---");

//   // Initialize sensor pins (optional for analog, but good practice)
//   pinMode(FORCE_SENSOR_LEFT_PIN, INPUT);
//   pinMode(FORCE_SENSOR_RIGHT_PIN, INPUT);

//   Serial.println("Attempting WiFi Connection...");
//   WiFi.begin(ssid, password);
// }

// //===============================================================
// // Function to Connect/Reconnect & Send Identifier
// //===============================================================
// bool connectToServer() {
//     if (client.connected() && serverConnected) return true;
//     Serial.print("Attempting connection to server ");
//     Serial.print(serverIP); Serial.print(":"); Serial.println(ServerPort);
//     client.stop(); delay(50);
//     if (client.connect(serverIP, ServerPort)) {
//         Serial.println("Connection established!");
//         Serial.print("Sending identifier: '"); Serial.print(myIdentifier); Serial.println("'");
//         client.print(myIdentifier); // <<< SEND IDENTIFIER
//         client.flush();
//         delay(50);
//         serverConnected = true;
//         return true;
//     } else {
//         Serial.println("Connection failed!");
//         serverConnected = false;
//         return false;
//     }
// }

// //===============================================================
// // Send Force State Function
// //===============================================================
// void sendForceState(char state) {
//   if (!serverConnected || !client.connected()) {
//       Serial.println("Not connected to server. Attempting reconnect...");
//       if (!connectToServer()) { // connectToServer now also sends ID
//           Serial.println("Reconnect failed. Skipping send.");
//           return;
//       }
//       // If reconnected, ID was already sent by connectToServer
//   }
//   // If already connected, proceed to send force state
//   if (client.connected()) {
//       Serial.print("Sending force state: '"); Serial.print(state); Serial.println("'");
//       client.print(state); // Send the force state character ('1' or '2')
//       if (!client.connected()) {
//            Serial.println("WARNING: Client disconnected immediately after sending data.");
//            serverConnected = false;
//       }
//   } else {
//        Serial.println("ERROR: Connection lost before sending data.");
//        serverConnected = false;
//   }
// }


// //===============================================================
// // MAIN LOOP
// //===============================================================
// void loop() {
//   unsigned long currentMillis = millis();

//   // --- WiFi Connection Management ---
//   if (WiFi.status() != WL_CONNECTED) {
//     static unsigned long lastWifiCheck = 0;
//     if (currentMillis - lastWifiCheck > 1000) {
//       Serial.print(".");
//       lastWifiCheck = currentMillis;
//     }
//     if (serverConnected) { // If WiFi drops, mark server disconnected
//       Serial.println("\nWiFi disconnected.");
//       serverConnected = false;
//       client.stop();
//     }
//     return; // Wait for WiFi
//   }

//   // --- Ensure Server Connection ---
//   // Try to connect ONCE after WiFi is up, or if connection drops later
//   if (!serverConnected) {
//       Serial.println("\nWiFi Connected!"); // Print only when reconnecting or first time
//       Serial.print("IP Address: "); Serial.println(WiFi.localIP());
//       connectToServer(); // Attempt connection (sends ID if successful)
//   }

//   // --- Read Sensors and Send Data Periodically ---
//   if (serverConnected && (currentMillis - lastSendTime >= SEND_INTERVAL)) {
//     lastSendTime = currentMillis;

//     // Read sensor values
//     int leftValue = analogRead(FORCE_SENSOR_LEFT_PIN);
//     int rightValue = analogRead(FORCE_SENSOR_RIGHT_PIN);

//     // Optional: Print sensor values for debugging threshold
//     // Serial.print("Sensor Values -> L: "); Serial.print(leftValue);
//     // Serial.print(" R: "); Serial.println(rightValue);

//     char currentState = '0'; // Default to '0' (no press)

//     // Determine state (Prioritize Left if both pressed?)
//     if (leftValue > FORCE_THRESHOLD) {
//       currentState = '1'; // Left pressed
//     } else if (rightValue > FORCE_THRESHOLD) {
//       currentState = '2'; // Right pressed
//     }
//     // else currentState remains '0'

//     // Send state ONLY if it has changed since last send
//     if (currentState != lastSentState) {
//         sendForceState(currentState);
//         lastSentState = currentState; // Update last sent state
//     }
//   }

//   // Small delay if not sending
//   if (currentMillis - lastSendTime < SEND_INTERVAL) {
//       delay(10);
//   }
// }

#include <WiFi.h>
#include <WiFiClient.h>

// --- Sensor Pins ---
const int FORCE_SENSOR_LEFT_PIN = 34;  // GPIO 34 (Analog input) - Check your board!
const int FORCE_SENSOR_RIGHT_PIN = 35; // GPIO 35 (Analog input) - Check your board!

// --- Sensor Threshold ---
// Adjust this value based on your sensor readings when pressed vs not pressed
const int FORCE_THRESHOLD = 1500; // Example threshold (out of 4095 for ESP32 ADC)

// --- WiFi Settings ---
const char *ssid = "ESP32_AP_Server"; // Receiver's SSID
const char *password = "123456789"; // Receiver's password
const char *serverIP = "192.168.4.1"; // Receiver's IP address
const uint ServerPort = 8080;       // Receiver's port

// --- Client & State ---
WiFiClient client;
bool serverConnected = false;
const char myIdentifier = 'F'; // Identifier for this Force-ESP
unsigned long lastCheckTime = 0; // Renamed from lastSendTime for clarity
const unsigned long CHECK_INTERVAL = 100; // How often to check sensors (ms) - can be faster
char lastSentState = '0'; // Keep track of last *sent* state ('0', '1', or '2')

//===============================================================
// SETUP
//===============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Force Sensor ESP (ID: 'F' - Sends Only on Press/Switch) ---");

  // Initialize sensor pins
  pinMode(FORCE_SENSOR_LEFT_PIN, INPUT);
  pinMode(FORCE_SENSOR_RIGHT_PIN, INPUT);

  Serial.println("Attempting WiFi Connection...");
  WiFi.begin(ssid, password);
}

//===============================================================
// Function to Connect/Reconnect & Send Identifier
//===============================================================
bool connectToServer() {
    if (client.connected() && serverConnected) return true;
    Serial.print("Attempting connection to server ");
    Serial.print(serverIP); Serial.print(":"); Serial.println(ServerPort);
    client.stop(); delay(50);
    if (client.connect(serverIP, ServerPort)) {
        Serial.println("Connection established!");
        Serial.print(">>> Sending Identifier to Receiver: '"); Serial.print(myIdentifier); Serial.println("'");
        client.print(myIdentifier); // <<< SEND IDENTIFIER
        client.flush();
        delay(50);
        serverConnected = true;
        return true;
    } else {
        Serial.println("Connection failed!");
        serverConnected = false;
        return false;
    }
}

//===============================================================
// Send Force State Function
//===============================================================
void sendForceState(char state) {
  if (!serverConnected || !client.connected()) {
      Serial.println("Not connected to server. Attempting reconnect...");
      if (!connectToServer()) {
          Serial.println("Reconnect failed. Skipping send.");
          return; // Exit if reconnect fails
      }
  }
  // If connected (or just reconnected)
  if (client.connected()) {
      Serial.print(">>> Sending force state to Receiver: '"); Serial.print(state); Serial.println("'");
      client.print(state); // Send the force state character ('1' or '2')
      if (!client.connected()) {
           Serial.println("WARNING: Client disconnected immediately after sending data.");
           serverConnected = false;
      }
  } else {
       Serial.println("ERROR: Connection lost before sending data.");
       serverConnected = false;
  }
}


//===============================================================
// MAIN LOOP
//===============================================================
void loop() {
  unsigned long currentMillis = millis();

  // --- WiFi Connection Management ---
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastWifiCheck = 0;
    if (currentMillis - lastWifiCheck > 1000) {
      Serial.print(".");
      lastWifiCheck = currentMillis;
    }
    if (serverConnected) { // If WiFi drops, mark server disconnected
      Serial.println("\nWiFi disconnected.");
      serverConnected = false;
      client.stop();
      lastSentState = '0'; // Reset state if disconnected
    }
    return; // Wait for WiFi
  }

  // --- Ensure Server Connection ---
  if (!serverConnected) {
      Serial.println("\nWiFi Connected!");
      Serial.print("IP Address: "); Serial.println(WiFi.localIP());
      connectToServer(); // Attempt connection (sends ID if successful)
  }

  // --- Read Sensors and Send Data Periodically ---
  if (serverConnected && (currentMillis - lastCheckTime >= CHECK_INTERVAL)) {
    lastCheckTime = currentMillis;

    // Read sensor values
    int leftValue = analogRead(FORCE_SENSOR_LEFT_PIN);
    int rightValue = analogRead(FORCE_SENSOR_RIGHT_PIN);

    // Determine current actual state based on sensors
    char currentState = '0';
    if (leftValue > FORCE_THRESHOLD) {
      currentState = '1'; // Left pressed
    } else if (rightValue > FORCE_THRESHOLD) {
      currentState = '2'; // Right pressed
    }
    // else: currentState remains '0'

    // --- Logic to Send Only on Press or Direct Switch ---
    // Check if the current state is a pressed state ('1' or '2')
    if (currentState == '1' || currentState == '2') {
        // Check if this pressed state is different from the last *sent* state
        if (currentState != lastSentState) {
            sendForceState(currentState); // Send '1' or '2'
            lastSentState = currentState; // Update the last *sent* state
        }
        // If currentState is the same as lastSentState (e.g., still holding '1'), do nothing.
    } else { // currentState is '0' (neither sensor is pressed)
        // If the last *sent* state was a pressed state, reset it to '0'
        // This signifies the release, but we don't send '0'.
        // This allows the *next* press (e.g., pressing '1' again) to be detected as a change.
        if (lastSentState != '0') {
            // Serial.println("Info: Sensor released, resetting lastSentState to '0'."); // Optional debug
            lastSentState = '0';
        }
        // If currentState is '0' and lastSentState was already '0', do nothing.
    }
    // --- End Logic ---
  }

  // Small delay to prevent busy-looping if not checking sensors
  if (currentMillis - lastCheckTime < CHECK_INTERVAL) {
      delay(10);
  }
}
