#include <WiFi.h>

// WiFi credentials - replace with your hotspot credentials
const char* ssid = "jingru";
const char* password = "12345678";

// Server details (your computer's IP address when hosting the hotspot)
const char* serverIP = "192.168.137.1"; // Change to your computer's IP address
const int serverPort = 8888;

WiFiClient client;
unsigned long lastConnectionAttempt = 0;
const int connectionInterval = 5000; // Try to reconnect every 5 seconds

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32 Status Receiver");
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Connected to WiFi with IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }
  
  // Try to connect to server if not connected
  if (!client.connected()) {
    unsigned long currentTime = millis();
    
    // Attempt to connect at regular intervals
    if (currentTime - lastConnectionAttempt > connectionInterval) {
      lastConnectionAttempt = currentTime;
      Serial.print("Connecting to server at ");
      Serial.print(serverIP);
      Serial.print(":");
      Serial.println(serverPort);
      
      if (client.connect(serverIP, serverPort)) {
        Serial.println("Connected to server!");
      } else {
        Serial.println("Connection failed");
      }
    }
  }
  
  // If connected, check for incoming data
  if (client.connected()) {
    if (client.available()) {
      String statusCode = client.readStringUntil('\n');
      statusCode.trim();
      
      Serial.print("Received status code: ");
      Serial.println(statusCode);
      
      // Process status code
      int status = statusCode.toInt();
      switch (status) {
        case 0:
          Serial.println("Status: STABLE");
          break;
        case 1:
          Serial.println("Status: UPDOWN");
          break;
        case 2:
          Serial.println("Status: LEFTRIGHT");
          break;
        default:
          Serial.println("Unknown status code");
      }
    }
  }
  
  delay(100); // Small delay to prevent CPU hogging
}
