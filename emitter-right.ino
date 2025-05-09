// #include <Wire.h>
// #include <WiFi.h>
// #include <WiFiClient.h>
// #include <Adafruit_NeoPixel.h>

// #ifdef __AVR__
//   #include <avr/power.h>
// #endif

// // --- Identifier ---
// // !!! IMPORTANT: SET TO 'L' FOR LEFT EMITTER, 'R' FOR RIGHT EMITTER !!!
// const char myIdentifier = 'L'; // OR 'R'

// // --- NeoPixel Settings ---
// #define NEOPIXEL_PIN 4  // GPIO Pin for NeoPixels
// #define NUMPIXELS    8  // Number of NeoPixels
// Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
// bool rainbowEffectActive = false;
// uint16_t rainbowCycleJ = 0;
// unsigned long lastRainbowUpdateTime = 0;
// const unsigned long rainbowUpdateInterval = 20; // ms between rainbow updates

// // --- MPU6050 Settings ---
// const int MPU = 0x68; // MPU6050 I2C address
// float AccX, AccY, AccZ;
// float GyroX, GyroY, GyroZ;
// float offsetAccX = 0, offsetAccY = 0, offsetAccZ = 0;
// float offsetGyroX = 0, offsetGyroY = 0, offsetGyroZ = 0;
// bool calibrated = false;
// unsigned long calibrationStartTime = 0;
// const unsigned long CALIBRATION_DURATION = 10000; // Calibration time 10 seconds
// long calibrationSampleCount = 0;
// double sumAccX = 0, sumAccY = 0, sumAccZ = 0; // Use double for sums
// double sumGyroX = 0, sumGyroY = 0, sumGyroZ = 0;

// // --- Motion Detection Settings ---
// const float MOTION_THRESHOLD_Z = 0.4;    // Adjust as needed
// const float MOTION_THRESHOLD_GYRO = 20; // Adjust as needed
// #define STATE_STABLE 0
// #define STATE_UPDOWN 1
// #define STATE_LEFTRIGHT 2
// const int HISTORY_SIZE = 3;
// bool upDownHistory[HISTORY_SIZE] = {false};
// bool leftRightHistory[HISTORY_SIZE] = {false};
// int historyIndex = 0;

// // --- WiFi Settings ---
// const char *ssid = "ESP32_AP_Server"; // Receiver's SSID
// const char *password = "123456789"; // Receiver's password
// const char *serverIP = "192.168.4.1"; // Receiver's IP address
// const uint ServerPort = 8080;       // Receiver's port

// // --- WiFi Client Object ---
// WiFiClient client;
// bool serverConnected = false;

// // --- Control & Timing Settings ---
// unsigned long lastSampleTime = 0;
// const unsigned long SAMPLE_INTERVAL = 200; // Read sensor data interval
// unsigned long lastSendTime = 0;
// const unsigned long SEND_INTERVAL = 500; // Send data to server interval

// //===============================================================
// // SETUP
// //===============================================================
// void setup() {
//   Serial.begin(115200);
//   Serial.print("\n--- MPU6050 WiFi Emitter (ID: '"); // Identify itself in Serial
//   Serial.print(myIdentifier);
//   Serial.println("') ---");

//   // NeoPixel Initialization
//   strip.begin(); strip.setBrightness(50); strip.clear(); strip.show();
//   Serial.println("NeoPixels Initialized Off.");

//   // Initialize MPU6050
//   Wire.begin(); // Consider Wire.begin(SDA_PIN, SCL_PIN); if not default
//   Wire.beginTransmission(MPU); Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission(true); // Wake up
//   Wire.beginTransmission(MPU); Wire.write(0x1C); Wire.write(0x08); Wire.endTransmission(true); // Accel +/- 4g
//   Wire.beginTransmission(MPU); Wire.write(0x1B); Wire.write(0x00); Wire.endTransmission(true); // Gyro +/- 250 dps
//   Serial.println("MPU6050 Initialized.");
//   delay(100);

//   // Start Calibration Process
//   Serial.println("Starting calibration. Keep sensor still for 10 seconds...");
//   calibrationStartTime = millis();
//   startCalibration(); // Call the function defined below
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
//         client.flush(); // Try to ensure it gets sent
//         delay(50); // Short delay after sending ID
//         serverConnected = true;
//         return true;
//     } else {
//         Serial.println("Connection failed!");
//         serverConnected = false;
//         return false;
//     }
// }

// //===============================================================
// // Send Data Function (Stable Connection Version) - Sends Motion State
// //===============================================================
// void sendDataStable(int state) {
//   if (!serverConnected || !client.connected()) {
//       Serial.println("Not connected to server. Attempting reconnect...");
//       if (!connectToServer()) { // connectToServer now also sends ID
//           Serial.println("Reconnect failed. Skipping send.");
//           return;
//       }
//   }
//   if (client.connected()) {
//       char dataToSend = state + '0';
//       Serial.print(">>> Sending Motion State to Receiver: ");
//       // Serial.print(state);
//       // Serial.print(" ('");
//       Serial.print(dataToSend);
//       Serial.println("')");
//       client.print(dataToSend); // Send the motion state character
//       if (!client.connected()) {
//            // Serial.println("WARNING: Client disconnected immediately after sending data.");
//            serverConnected = false;
//       }

//   } else {
//        Serial.println("ERROR: Connection lost before sending data.");
//        serverConnected = false;
//   }
// }

// //===============================================================
// // Non-Blocking NeoPixel Rainbow Update Function
// //===============================================================
// void updateRainbowEffect() {
//     for(uint16_t i=0; i< strip.numPixels(); i++) {
//       strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + rainbowCycleJ) & 255));
//     }
//     strip.show();
//     rainbowCycleJ++;
//     // Optional: Reset J after 5 cycles (256*5 = 1280 steps)
//     if (rainbowCycleJ >= 256 * 5) { rainbowCycleJ = 0; }
//     // Or just let it wrap: rainbowCycleJ++;
// }

// //===============================================================
// // MAIN LOOP
// //===============================================================
// void loop() {
//     unsigned long currentMillis = millis();

//     // --- Phase 1: Calibration ---
//     if (!calibrated) {
//         collectCalibrationData(); // Call the function defined below
//         if (currentMillis - calibrationStartTime >= CALIBRATION_DURATION) {
//             finishCalibration(); // Call the function defined below
//             calibrated = true;
//             Serial.println("\nCalibration Complete!");
//             Serial.print("  Acc Offsets: X="); Serial.print(offsetAccX); Serial.print(", Y="); Serial.print(offsetAccY); Serial.print(", Z="); Serial.println(offsetAccZ);
//             Serial.print("  Gyro Offsets: X="); Serial.print(offsetGyroX); Serial.print(", Y="); Serial.print(offsetGyroY); Serial.print(", Z="); Serial.println(offsetGyroZ);
//             Serial.println("Attempting WiFi Connection...");
//             WiFi.begin(ssid, password);
//         }
//         return;
//     }

//     // --- Phase 2: WiFi Connection Management ---
//     if (WiFi.status() != WL_CONNECTED) {
//         static unsigned long lastWifiCheck = 0;
//         if (currentMillis - lastWifiCheck > 1000) { Serial.print("."); lastWifiCheck = currentMillis; }
//         if (rainbowEffectActive) { rainbowEffectActive = false; strip.clear(); strip.show(); }
//         if (serverConnected) { serverConnected = false; client.stop(); }
//         return;
//     }

//     // --- Phase 3: Connected and Running ---
//     if (!rainbowEffectActive) { // Start effect ONCE when WiFi connects
//         Serial.println("\nWiFi Connected!"); Serial.print("IP Address: "); Serial.println(WiFi.localIP());
//         Serial.println("Starting non-blocking rainbow effect...");
//         rainbowEffectActive = true; rainbowCycleJ = 0; lastRainbowUpdateTime = currentMillis;
//         connectToServer(); // Initial server connection attempt (sends ID)
//     }

//     // --- Update NeoPixel Effect ---
//     if (rainbowEffectActive && (currentMillis - lastRainbowUpdateTime >= rainbowUpdateInterval)) {
//         lastRainbowUpdateTime = currentMillis;
//         updateRainbowEffect();
//     }

//     // --- Read Sensor Data ---
//     if (currentMillis - lastSampleTime >= SAMPLE_INTERVAL) {
//         lastSampleTime = currentMillis;
//         readAndCorrectData(); // Call the function defined below
//         // Keep the IMU Data printout for debugging
//         Serial.print("IMU Data -> Acc(g): "); Serial.print("X="); Serial.print(AccX, 4); Serial.print(" Y="); Serial.print(AccY, 4); Serial.print(" Z="); Serial.print(AccZ, 4);
//         Serial.print(" | Gyro(dps): "); Serial.print("X="); Serial.print(GyroX, 2); Serial.print(" Y="); Serial.print(GyroY, 2); Serial.print(" Z="); Serial.println(GyroZ, 2);
//         detectCurrentMotion(); // Call the function defined below
//     }

//     // --- Send Motion Data ---
//     if (currentMillis - lastSendTime >= SEND_INTERVAL) {
//         lastSendTime = currentMillis;
//         int currentState = determineOverallState(); // Call the function defined below
//         sendDataStable(currentState); // Sends motion state (0, 1, 2)
//     }
// }

// //===============================================================
// // NeoPixel Wheel Function (Unchanged)
// //===============================================================
// uint32_t Wheel(byte WheelPos) {
//   WheelPos = 255 - WheelPos;
//   if(WheelPos < 85) return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
//   if(WheelPos < 170) { WheelPos -= 85; return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3); }
//   WheelPos -= 170; return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
// }

// //===============================================================
// // MPU6050 Functions (DEFINITIONS NOW INCLUDED)
// //===============================================================

// // Initialize calibration variables
// void startCalibration() {
//   sumAccX = 0; sumAccY = 0; sumAccZ = 0;
//   sumGyroX = 0; sumGyroY = 0; sumGyroZ = 0;
//   calibrationSampleCount = 0;
// }

// // Collect data during calibration period
// void collectCalibrationData() {
//   readRawData(); // Read raw values first
//   // Accumulate sums
//   sumAccX += AccX; sumAccY += AccY; sumAccZ += AccZ;
//   sumGyroX += GyroX; sumGyroY += GyroY; sumGyroZ += GyroZ;
//   calibrationSampleCount++;

//   // Optional progress print
//   static int lastProgress = -1;
//   int progress = (millis() - calibrationStartTime) * 100 / CALIBRATION_DURATION;
//   if (progress != lastProgress && progress % 10 == 0) {
//       Serial.printf("Calibration Progress: %d%%\n", progress);
//       lastProgress = progress;
//   }
//   delay(10); // Small delay between readings during calibration
// }

// // Calculate final offsets after calibration duration
// void finishCalibration() {
//   if (calibrationSampleCount > 0) {
//     // Calculate average readings
//     offsetAccX = sumAccX / calibrationSampleCount;
//     offsetAccY = sumAccY / calibrationSampleCount;
//     // Assume Z axis is aligned with gravity during calibration. Subtract 1g.
//     offsetAccZ = (sumAccZ / calibrationSampleCount) - 1.0;
//     offsetGyroX = sumGyroX / calibrationSampleCount;
//     offsetGyroY = sumGyroY / calibrationSampleCount;
//     offsetGyroZ = sumGyroZ / calibrationSampleCount;
//   } else {
//       Serial.println("Error: No calibration samples collected! Using zero offsets.");
//       // Set offsets to zero if calibration failed
//       offsetAccX = 0; offsetAccY = 0; offsetAccZ = 0;
//       offsetGyroX = 0; offsetGyroY = 0; offsetGyroZ = 0;
//   }
// }

// // Read raw data from MPU6050 registers
// void readRawData() {
//   Wire.beginTransmission(MPU);
//   Wire.write(0x3B); // Start reading from ACCEL_XOUT_H register
//   Wire.endTransmission(false); // Keep connection active for reading
//   // Request 14 bytes (6 Accel, 2 Temp, 6 Gyro)
//   Wire.requestFrom(MPU, 14, true); // Send stop message after reading

//   // Read Accel data and convert based on sensitivity (+/- 4g -> 8192 LSB/g)
//   AccX = (Wire.read() << 8 | Wire.read()) / 8192.0;
//   AccY = (Wire.read() << 8 | Wire.read()) / 8192.0;
//   AccZ = (Wire.read() << 8 | Wire.read()) / 8192.0;

//   // Skip temperature data (2 bytes)
//   Wire.read(); Wire.read();

//   // Read Gyro data and convert based on sensitivity (+/- 250 dps -> 131 LSB/dps)
//   GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
//   GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
//   GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
// }

// // Read raw data and apply calibration offsets
// void readAndCorrectData() {
//   readRawData(); // Get the latest raw sensor readings
//   // Subtract the calculated offsets
//   AccX -= offsetAccX;
//   AccY -= offsetAccY;
//   AccZ -= offsetAccZ; // Z offset already accounts for gravity subtraction
//   GyroX -= offsetGyroX;
//   GyroY -= offsetGyroY;
//   GyroZ -= offsetGyroZ;
// }

// // Check current sensor readings against thresholds
// void detectCurrentMotion() {
//   // Use absolute values of *corrected* data
//   float absAccZ_deviation = abs(AccZ); // Deviation from calibrated zero (gravity removed)
//   float absGyroX = abs(GyroX);
//   float absGyroZ = abs(GyroZ);

//   // Check thresholds
//   bool isUpDown = (absAccZ_deviation > MOTION_THRESHOLD_Z);
//   bool isLeftRight = (absGyroX > MOTION_THRESHOLD_GYRO || absGyroZ > MOTION_THRESHOLD_GYRO);

//   // Update history buffer
//   upDownHistory[historyIndex] = isUpDown;
//   leftRightHistory[historyIndex] = isLeftRight;
//   // Move to next index in circular buffer
//   historyIndex = (historyIndex + 1) % HISTORY_SIZE;
// }

// // Determine overall state based on recent history
// int determineOverallState() {
//   int upDownCount = 0;
//   int leftRightCount = 0;

//   // Count occurrences in history buffer
//   for (int i = 0; i < HISTORY_SIZE; i++) {
//     if (upDownHistory[i]) upDownCount++;
//     if (leftRightHistory[i]) leftRightCount++;
//   }

//   // Check if motion was sustained (more than half the history window)
//   bool sustainedUpDown = (upDownCount > HISTORY_SIZE / 2);
//   bool sustainedLeftRight = (leftRightCount > HISTORY_SIZE / 2);

//   // Determine final state with priority (Up/Down > Left/Right > Stable)
//   if (sustainedUpDown) {
//     return STATE_UPDOWN; // State 1
//   } else if (sustainedLeftRight) {
//     return STATE_LEFTRIGHT; // State 2
//   } else {
//     return STATE_STABLE; // State 0
//   }
// }






#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

// --- Identifier ---
// !!! IMPORTANT: SET TO 'L' FOR LEFT EMITTER, 'R' FOR RIGHT EMITTER !!!
const char myIdentifier = 'R'; // OR 'R'

// --- NeoPixel Settings ---
#define NEOPIXEL_PIN 4  // GPIO Pin for NeoPixels
#define NUMPIXELS    8  // Number of NeoPixels
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
bool rainbowEffectActive = false; // Controlled by receiver feedback now
uint16_t rainbowCycleJ = 0;
unsigned long lastRainbowUpdateTime = 0;
const unsigned long rainbowUpdateInterval = 20; // ms between rainbow updates

// --- MPU6050 Settings ---
const int MPU = 0x68; // MPU6050 I2C address
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float offsetAccX = 0, offsetAccY = 0, offsetAccZ = 0;
float offsetGyroX = 0, offsetGyroY = 0, offsetGyroZ = 0;
bool calibrated = false;
unsigned long calibrationStartTime = 0;
const unsigned long CALIBRATION_DURATION = 10000; // Calibration time 10 seconds
long calibrationSampleCount = 0;
double sumAccX = 0, sumAccY = 0, sumAccZ = 0; // Use double for sums
double sumGyroX = 0, sumGyroY = 0, sumGyroZ = 0;

// --- Motion Detection Settings ---
const float MOTION_THRESHOLD_Z = 0.4;    // Adjust as needed
const float MOTION_THRESHOLD_GYRO = 20; // Adjust as needed
#define STATE_STABLE 0
#define STATE_UPDOWN 1
#define STATE_LEFTRIGHT 2
const int HISTORY_SIZE = 3;
bool upDownHistory[HISTORY_SIZE] = {false};
bool leftRightHistory[HISTORY_SIZE] = {false};
int historyIndex = 0;

// --- WiFi Settings ---
const char *ssid = "ESP32_AP_Server"; // Receiver's SSID
const char *password = "123456789"; // Receiver's password
const char *serverIP = "192.168.4.1"; // Receiver's IP address
const uint ServerPort = 8080;       // Receiver's port

// --- WiFi Client Object ---
WiFiClient client;
bool serverConnected = false;

// --- Control & Timing Settings ---
unsigned long lastSampleTime = 0;
const unsigned long SAMPLE_INTERVAL = 200; // Read sensor data interval
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 500; // Send data to server interval

//===============================================================
// SETUP
//===============================================================
void setup() {
  Serial.begin(115200);
  Serial.print("\n--- MPU6050 WiFi Emitter (ID: '");
  Serial.print(myIdentifier);
  Serial.println("') ---");

  // NeoPixel Initialization
  strip.begin(); strip.setBrightness(50); strip.clear(); strip.show();
  Serial.println("NeoPixels Initialized Off.");

  // Initialize MPU6050
  Wire.begin(); // Consider Wire.begin(SDA_PIN, SCL_PIN); if not default
  Wire.beginTransmission(MPU); Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission(true); // Wake up
  Wire.beginTransmission(MPU); Wire.write(0x1C); Wire.write(0x08); Wire.endTransmission(true); // Accel +/- 4g
  Wire.beginTransmission(MPU); Wire.write(0x1B); Wire.write(0x00); Wire.endTransmission(true); // Gyro +/- 250 dps
  Serial.println("MPU6050 Initialized.");
  delay(100);

  // Start Calibration Process
  Serial.println("Starting calibration. Keep sensor still for 10 seconds...");
  calibrationStartTime = millis();
  startCalibration(); // Call the function defined below
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
        client.flush(); // Try to ensure it gets sent
        delay(50); // Short delay after sending ID
        serverConnected = true;
        return true;
    } else {
        Serial.println("Connection failed!");
        serverConnected = false;
        return false;
    }
}

//===============================================================
// Send Data Function (Stable Connection Version) - Sends Motion State
//===============================================================
void sendDataStable(int state) {
  if (!serverConnected || !client.connected()) {
      Serial.println("Not connected to server. Attempting reconnect...");
      if (!connectToServer()) { // connectToServer now also sends ID
          Serial.println("Reconnect failed. Skipping send.");
          return;
      }
  }
  if (client.connected()) {
      char dataToSend = state + '0';
      Serial.print(">>> Sending Motion State to Receiver: "); Serial.print(dataToSend); Serial.println("')"); // Corrected print
      client.print(dataToSend); // Send the motion state character
      if (!client.connected()) { serverConnected = false; }
  } else {
       Serial.println("ERROR: Connection lost before sending data.");
       serverConnected = false;
  }
}

//===============================================================
// Non-Blocking NeoPixel Rainbow Update Function
//===============================================================
void updateRainbowEffect() {
    if (!rainbowEffectActive) return; // Don't update if effect is off

    for(uint16_t i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + rainbowCycleJ) & 255));
    }
    strip.show();
    rainbowCycleJ++;
    // Optional: Reset J after 5 cycles (256*5 = 1280 steps)
    if (rainbowCycleJ >= 256 * 5) { rainbowCycleJ = 0; }
    // Or just let it wrap: rainbowCycleJ++;
}

//===============================================================
// Function to Handle Feedback from Receiver
//===============================================================
void handleReceiverFeedback() {
    if (serverConnected && client.connected() && client.available()) {
        while (client.available()) { // Read all available chars
            char receivedChar = client.read();
            Serial.print("<<< Received feedback from Receiver: '");
            Serial.print(receivedChar);
            Serial.println("'");

            if (receivedChar == '1') { // Selected
                if (!rainbowEffectActive) {
                    Serial.println("--- Starting Rainbow Effect (Selected) ---");
                    rainbowEffectActive = true;
                    rainbowCycleJ = 0; // Reset effect position
                    lastRainbowUpdateTime = millis(); // Set time for immediate update
                }
            } else if (receivedChar == '0') { // Not selected
                if (rainbowEffectActive) {
                    Serial.println("--- Stopping Rainbow Effect (Not Selected) ---");
                    rainbowEffectActive = false;
                    strip.clear(); // Turn off LEDs
                    strip.show();
                }
            } else if (receivedChar != '\n' && receivedChar != '\r') {
                Serial.println("   (Ignoring unexpected feedback character)");
            }
        }
    }
}


//===============================================================
// MAIN LOOP
//===============================================================
void loop() {
    unsigned long currentMillis = millis();

    // --- Phase 1: Calibration ---
    if (!calibrated) {
        collectCalibrationData(); // Call the function defined below
        if (currentMillis - calibrationStartTime >= CALIBRATION_DURATION) {
            finishCalibration(); // Call the function defined below
            calibrated = true;
            Serial.println("\nCalibration Complete!");
            Serial.print("  Acc Offsets: X="); Serial.print(offsetAccX); Serial.print(", Y="); Serial.print(offsetAccY); Serial.print(", Z="); Serial.println(offsetAccZ);
            Serial.print("  Gyro Offsets: X="); Serial.print(offsetGyroX); Serial.print(", Y="); Serial.print(offsetGyroY); Serial.print(", Z="); Serial.println(offsetGyroZ);
            Serial.println("Attempting WiFi Connection...");
            WiFi.begin(ssid, password);
        }
        return;
    }

    // --- Phase 2: WiFi Connection Management ---
    if (WiFi.status() != WL_CONNECTED) {
        static unsigned long lastWifiCheck = 0;
        if (currentMillis - lastWifiCheck > 1000) { Serial.print("."); lastWifiCheck = currentMillis; }
        // Ensure effect is off if WiFi drops
        if (rainbowEffectActive) {
             Serial.println("\nWiFi disconnected, stopping effect.");
             rainbowEffectActive = false; strip.clear(); strip.show();
        }
        if (serverConnected) { serverConnected = false; client.stop(); }
        return;
    }

    // --- Phase 3: Connected and Running ---
    // Ensure server connection (will also send ID if reconnecting)
    if (!serverConnected) {
        Serial.println("\nWiFi Connected! Attempting server connection...");
        connectToServer();
        // Don't automatically start rainbow effect here anymore
    }

    // --- Check for Feedback from Receiver ---
    handleReceiverFeedback(); // Check for '0' or '1'

    // --- Update NeoPixel Effect (if active) ---
    if (rainbowEffectActive && (currentMillis - lastRainbowUpdateTime >= rainbowUpdateInterval)) {
        lastRainbowUpdateTime = currentMillis;
        updateRainbowEffect();
    }

    // --- Read Sensor Data ---
    if (currentMillis - lastSampleTime >= SAMPLE_INTERVAL) {
        lastSampleTime = currentMillis;
        readAndCorrectData(); // Call the function defined below
        // Keep the IMU Data printout for debugging
        Serial.print("IMU Data -> Acc(g): "); Serial.print("X="); Serial.print(AccX, 4); Serial.print(" Y="); Serial.print(AccY, 4); Serial.print(" Z="); Serial.print(AccZ, 4);
        Serial.print(" | Gyro(dps): "); Serial.print("X="); Serial.print(GyroX, 2); Serial.print(" Y="); Serial.print(GyroY, 2); Serial.print(" Z="); Serial.println(GyroZ, 2);
        detectCurrentMotion(); // Call the function defined below
    }

    // --- Send Motion Data ---
    if (currentMillis - lastSendTime >= SEND_INTERVAL) {
        lastSendTime = currentMillis;
        int currentState = determineOverallState(); // Call the function defined below
        sendDataStable(currentState); // Sends motion state (0, 1, 2)
    }
}

//===============================================================
// NeoPixel Wheel Function (Unchanged)
//===============================================================
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  if(WheelPos < 170) { WheelPos -= 85; return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3); }
  WheelPos -= 170; return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//===============================================================
// MPU6050 Functions (DEFINITIONS NOW INCLUDED)
//===============================================================

// Initialize calibration variables
void startCalibration() {
  sumAccX = 0; sumAccY = 0; sumAccZ = 0;
  sumGyroX = 0; sumGyroY = 0; sumGyroZ = 0;
  calibrationSampleCount = 0;
}

// Collect data during calibration period
void collectCalibrationData() {
  readRawData(); // Read raw values first
  // Accumulate sums
  sumAccX += AccX; sumAccY += AccY; sumAccZ += AccZ;
  sumGyroX += GyroX; sumGyroY += GyroY; sumGyroZ += GyroZ;
  calibrationSampleCount++;

  // Optional progress print
  static int lastProgress = -1;
  int progress = (millis() - calibrationStartTime) * 100 / CALIBRATION_DURATION;
  if (progress != lastProgress && progress % 10 == 0) {
      Serial.printf("Calibration Progress: %d%%\n", progress);
      lastProgress = progress;
  }
  delay(10); // Small delay between readings during calibration
}

// Calculate final offsets after calibration duration
void finishCalibration() {
  if (calibrationSampleCount > 0) {
    // Calculate average readings
    offsetAccX = sumAccX / calibrationSampleCount;
    offsetAccY = sumAccY / calibrationSampleCount;
    // Assume Z axis is aligned with gravity during calibration. Subtract 1g.
    offsetAccZ = (sumAccZ / calibrationSampleCount) - 1.0;
    offsetGyroX = sumGyroX / calibrationSampleCount;
    offsetGyroY = sumGyroY / calibrationSampleCount;
    offsetGyroZ = sumGyroZ / calibrationSampleCount;
  } else {
      Serial.println("Error: No calibration samples collected! Using zero offsets.");
      // Set offsets to zero if calibration failed
      offsetAccX = 0; offsetAccY = 0; offsetAccZ = 0;
      offsetGyroX = 0; offsetGyroY = 0; offsetGyroZ = 0;
  }
}

// Read raw data from MPU6050 registers
void readRawData() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Start reading from ACCEL_XOUT_H register
  Wire.endTransmission(false); // Keep connection active for reading
  // Request 14 bytes (6 Accel, 2 Temp, 6 Gyro)
  Wire.requestFrom(MPU, 14, true); // Send stop message after reading

  // Read Accel data and convert based on sensitivity (+/- 4g -> 8192 LSB/g)
  AccX = (Wire.read() << 8 | Wire.read()) / 8192.0;
  AccY = (Wire.read() << 8 | Wire.read()) / 8192.0;
  AccZ = (Wire.read() << 8 | Wire.read()) / 8192.0;

  // Skip temperature data (2 bytes)
  Wire.read(); Wire.read();

  // Read Gyro data and convert based on sensitivity (+/- 250 dps -> 131 LSB/dps)
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
}

// Read raw data and apply calibration offsets
void readAndCorrectData() {
  readRawData(); // Get the latest raw sensor readings
  // Subtract the calculated offsets
  AccX -= offsetAccX;
  AccY -= offsetAccY;
  AccZ -= offsetAccZ; // Z offset already accounts for gravity subtraction
  GyroX -= offsetGyroX;
  GyroY -= offsetGyroY;
  GyroZ -= offsetGyroZ;
}

// Check current sensor readings against thresholds
void detectCurrentMotion() {
  // Use absolute values of *corrected* data
  float absAccZ_deviation = abs(AccZ-1); // Deviation from calibrated zero (gravity removed)
  float absGyroX = abs(GyroX);
  float absGyroZ = abs(GyroZ);

  // Check thresholds
  bool isUpDown = (absAccZ_deviation > MOTION_THRESHOLD_Z);
  bool isLeftRight = (absGyroX > MOTION_THRESHOLD_GYRO || absGyroZ > MOTION_THRESHOLD_GYRO);

  // Update history buffer
  upDownHistory[historyIndex] = isUpDown;
  leftRightHistory[historyIndex] = isLeftRight;
  // Move to next index in circular buffer
  historyIndex = (historyIndex + 1) % HISTORY_SIZE;
}

// Determine overall state based on recent history
int determineOverallState() {
  int upDownCount = 0;
  int leftRightCount = 0;

  // Count occurrences in history buffer
  for (int i = 0; i < HISTORY_SIZE; i++) {
    if (upDownHistory[i]) upDownCount++;
    if (leftRightHistory[i]) leftRightCount++;
  }

  // Check if motion was sustained (more than half the history window)
  bool sustainedUpDown = (upDownCount > HISTORY_SIZE / 2);
  bool sustainedLeftRight = (leftRightCount > HISTORY_SIZE / 2);

  // Determine final state with priority (Up/Down > Left/Right > Stable)
  if (sustainedUpDown) {
    return STATE_UPDOWN; // State 1
  } else if (sustainedLeftRight) {
    return STATE_LEFTRIGHT; // State 2
  } else {
    return STATE_STABLE; // State 0
  }
}
