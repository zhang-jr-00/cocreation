#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

// --- Identifier ---
// !!! IMPORTANT: SET TO 'L' FOR LEFT EMITTER, 'R' FOR RIGHT EMITTER !!!
const char myIdentifier = 'L'; // OR 'R'

// --- NeoPixel Settings ---
#define NEOPIXEL_PIN 4
#define NUMPIXELS    8
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
bool rainbowEffectActive = false;
uint16_t rainbowCycleJ = 0;
unsigned long lastRainbowUpdateTime = 0;
const unsigned long rainbowUpdateInterval = 20;

// --- MPU6050 Settings ---
const int MPU = 0x68;
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float offsetAccX = 0, offsetAccY = 0, offsetAccZ = 0;
float offsetGyroX = 0, offsetGyroY = 0, offsetGyroZ = 0;
bool calibrated = false;
unsigned long calibrationStartTime = 0;
const unsigned long CALIBRATION_DURATION = 10000;
long calibrationSampleCount = 0;
double sumAccX = 0, sumAccY = 0, sumAccZ = 0;
double sumGyroX = 0, sumGyroY = 0, sumGyroZ = 0;

// --- Motion Detection Settings ---
const float MOTION_THRESHOLD_Z = 0.4;
const float MOTION_THRESHOLD_GYRO = 20;
#define STATE_STABLE 0
#define STATE_UPDOWN 1
#define STATE_LEFTRIGHT 2
const int HISTORY_SIZE = 3;
bool upDownHistory[HISTORY_SIZE] = {false};
bool leftRightHistory[HISTORY_SIZE] = {false};
int historyIndex = 0;

// --- WiFi Settings ---
const char *ssid = "ESP32_AP_Server";
const char *password = "123456789";
const char *serverIP = "192.168.4.1";
const uint ServerPort = 8080;

// --- WiFi Client Object ---
WiFiClient client;
bool serverConnected = false;

// --- Control & Timing Settings ---
unsigned long lastSampleTime = 0;
const unsigned long SAMPLE_INTERVAL = 200;
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 500;

//===============================================================
// SETUP
//===============================================================
void setup() {
  Serial.begin(115200);
  Serial.print("\n--- MPU6050 WiFi Emitter (ID: '"); // Identify itself in Serial
  Serial.print(myIdentifier);
  Serial.println("') ---");

  // NeoPixel Initialization
  strip.begin(); strip.setBrightness(50); strip.clear(); strip.show();
  Serial.println("NeoPixels Initialized Off.");

  // Initialize MPU6050
  Wire.begin();
  Wire.beginTransmission(MPU); Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission(true);
  Wire.beginTransmission(MPU); Wire.write(0x1C); Wire.write(0x08); Wire.endTransmission(true);
  Wire.beginTransmission(MPU); Wire.write(0x1B); Wire.write(0x00); Wire.endTransmission(true);
  Serial.println("MPU6050 Initialized.");
  delay(100);

  // Start Calibration Process
  Serial.println("Starting calibration. Keep sensor still for 10 seconds...");
  calibrationStartTime = millis();
  startCalibration();
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
        Serial.print("Sending identifier: '"); Serial.print(myIdentifier); Serial.println("'");
        client.print(myIdentifier); // <<< SEND IDENTIFIER
        client.flush(); // Try to ensure it gets sent before potentially other data
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
      // If reconnected, ID was already sent by connectToServer
  }
  // If already connected, proceed to send motion state
  if (client.connected()) {
      char dataToSend = state + '0';
      // Don't print every single send to reduce serial clutter maybe?
      // Serial.print("Sending state (stable): "); Serial.print(state);
      // Serial.print(" ('"); Serial.print(dataToSend); Serial.println("')");
      client.print(dataToSend); // Send the motion state character
      if (!client.connected()) {
           // Serial.println("WARNING: Client disconnected immediately after sending data.");
           serverConnected = false;
      }
  } else {
       Serial.println("ERROR: Connection lost before sending data.");
       serverConnected = false;
  }
}

//===============================================================
// Non-Blocking NeoPixel Rainbow Update Function
//===============================================================
void updateRainbowEffect() {
    for(uint16_t i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + rainbowCycleJ) & 255));
    }
    strip.show();
    rainbowCycleJ++;
    if (rainbowCycleJ >= 256 * 5) { rainbowCycleJ = 0; }
}

//===============================================================
// MAIN LOOP
//===============================================================
void loop() {
    unsigned long currentMillis = millis();

    // --- Phase 1: Calibration ---
    if (!calibrated) {
        collectCalibrationData();
        if (currentMillis - calibrationStartTime >= CALIBRATION_DURATION) {
            finishCalibration(); calibrated = true;
            Serial.println("\nCalibration Complete!");
            // Print offsets... (omitted for brevity, but keep them in your code)
            Serial.println("Attempting WiFi Connection...");
            WiFi.begin(ssid, password);
        }
        return;
    }

    // --- Phase 2: WiFi Connection Management ---
    if (WiFi.status() != WL_CONNECTED) {
        static unsigned long lastWifiCheck = 0;
        if (currentMillis - lastWifiCheck > 1000) { Serial.print("."); lastWifiCheck = currentMillis; }
        if (rainbowEffectActive) { rainbowEffectActive = false; strip.clear(); strip.show(); }
        if (serverConnected) { serverConnected = false; client.stop(); }
        return;
    }

    // --- Phase 3: Connected and Running ---
    if (!rainbowEffectActive) { // Start effect ONCE when WiFi connects
        Serial.println("\nWiFi Connected!"); Serial.print("IP Address: "); Serial.println(WiFi.localIP());
        Serial.println("Starting non-blocking rainbow effect...");
        rainbowEffectActive = true; rainbowCycleJ = 0; lastRainbowUpdateTime = currentMillis;
        connectToServer(); // Initial server connection attempt (sends ID)
    }

    // --- Update NeoPixel Effect ---
    if (rainbowEffectActive && (currentMillis - lastRainbowUpdateTime >= rainbowUpdateInterval)) {
        lastRainbowUpdateTime = currentMillis;
        updateRainbowEffect();
    }

    // --- Read Sensor Data ---
    if (currentMillis - lastSampleTime >= SAMPLE_INTERVAL) {
        lastSampleTime = currentMillis;
        readAndCorrectData();
        // Keep the IMU Data printout for debugging
        Serial.print("IMU Data -> Acc(g): "); Serial.print("X="); Serial.print(AccX, 4); Serial.print(" Y="); Serial.print(AccY, 4); Serial.print(" Z="); Serial.print(AccZ, 4);
        Serial.print(" | Gyro(dps): "); Serial.print("X="); Serial.print(GyroX, 2); Serial.print(" Y="); Serial.print(GyroY, 2); Serial.print(" Z="); Serial.println(GyroZ, 2);
        detectCurrentMotion();
    }

    // --- Send Motion Data ---
    if (currentMillis - lastSendTime >= SEND_INTERVAL) {
        lastSendTime = currentMillis;
        int currentState = determineOverallState();
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
// MPU6050 Functions (Unchanged) - Keep all as they were
//===============================================================
// void startCalibration()...
// void collectCalibrationData()...
// void finishCalibration()...
// void readRawData()...
// void readAndCorrectData()...
// void detectCurrentMotion()...
// int determineOverallState()...
// (Make sure you copy the full MPU6050 functions from the previous version)
