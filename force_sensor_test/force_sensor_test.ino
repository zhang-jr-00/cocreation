const int FORCE_SENSOR_LEFT_PIN = 34;  // GPIO 34 (ADC1_CH6) - Connect Left Sensor Here
const int FORCE_SENSOR_RIGHT_PIN = 35; // GPIO 35 (ADC1_CH7) - Connect Right Sensor Here

// --- Timing ---
unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 200; // How often to read and print (ms)

//===============================================================
// SETUP
//===============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Force Sensor Test ---");
  Serial.println("Reading analog values from pins:");
  Serial.print("  Left Sensor Pin: "); Serial.println(FORCE_SENSOR_LEFT_PIN);
  Serial.print("  Right Sensor Pin: "); Serial.println(FORCE_SENSOR_RIGHT_PIN);
  Serial.println("Ensure sensors are connected to these pins.");

  // Initialize sensor pins as input (optional for analog, good practice)
  // Note: Pins >= 34 do not have internal pull-ups/pull-downs.
  pinMode(FORCE_SENSOR_LEFT_PIN, INPUT);
  pinMode(FORCE_SENSOR_RIGHT_PIN, INPUT);
}

//===============================================================
// MAIN LOOP
//===============================================================
void loop() {
  unsigned long currentMillis = millis();

  // Read and print sensor values periodically
  if (currentMillis - lastReadTime >= READ_INTERVAL) {
    lastReadTime = currentMillis;

    // Read the raw analog values (0-4095 on ESP32)
    int leftValue = analogRead(FORCE_SENSOR_LEFT_PIN);
    int rightValue = analogRead(FORCE_SENSOR_RIGHT_PIN);

    // Print the values to the Serial Monitor
    Serial.print("Left Sensor: ");
    Serial.print(leftValue);
    Serial.print("  |  Right Sensor: ");
    Serial.println(rightValue);
  }

  // A small delay can be added if needed, but the interval check handles timing.
  // delay(10);
}

