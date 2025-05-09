#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h> // Required for some Adafruit examples, okay on ESP32
#endif

// --- NeoPixel Settings ---
#define NEOPIXEL_PIN 4  // GPIO Pin for NeoPixels (ensure this is correct for your setup)
#define NUMPIXELS    8  // Number of NeoPixels in your strip/ring
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// --- Effect State Management ---
enum EffectPhase {
  PHASE_DARK,
  PHASE_RAINBOW
};
EffectPhase currentPhase = PHASE_DARK; // Start with the dark phase
unsigned long phaseStartTime = 0;      // Time when the current phase started

const unsigned long DARK_DURATION = 5000;    // 5 seconds for dark phase
const unsigned long RAINBOW_DURATION = 10000; // 20 seconds for rainbow phase

// --- Non-Blocking Rainbow Effect State ---
bool rainbowEffectActive = false; // To enable/disable the rainbow updates
uint16_t rainbowCycleJ = 0;       // Counter for the rainbow cycle 'j' position
unsigned long lastRainbowUpdateTime = 0; // millis() of last NeoPixel update
const unsigned long rainbowUpdateInterval = 20; // ms between rainbow updates (adjust for speed)

//===============================================================
// SETUP
//===============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n--- NeoPixel Cyclical Dark/Rainbow Tester ---");

  // NeoPixel Initialization
  #if defined (__AVR_ATtiny85__) // Trinket specific code (safe to keep)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  strip.begin();             // Initialize NeoPixel strip
  strip.setBrightness(100);   // Set brightness (0-255) - adjust as needed
  strip.clear();             // Set all pixels to 'off' initially
  strip.show();              // Apply the clear command
  Serial.println("NeoPixels Initialized Off.");

  // Initialize the first phase (DARK)
  currentPhase = PHASE_DARK;
  phaseStartTime = millis();
  rainbowEffectActive = false; // Ensure rainbow is not active initially
  Serial.println("Starting initial 5-second DARK phase...");
}

//===============================================================
// Non-Blocking NeoPixel Rainbow Update Function
//===============================================================
void updateRainbowEffect() {
    if (!rainbowEffectActive) return; // Don't update if effect is not active

    // Calculate colors and update strip for each pixel
    for(uint16_t i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + rainbowCycleJ) & 255));
    }
    strip.show(); // Send the updated colors to the strip

    rainbowCycleJ++; // Advance to the next step in the rainbow cycle
    if (rainbowCycleJ >= 256 * 5) { // Optional: Reset J after 5 full cycles
       rainbowCycleJ = 0;
    }
}

//===============================================================
// NeoPixel Wheel Function (Calculates a color based on position)
//===============================================================
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos; // Invert to match common examples
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//===============================================================
// MAIN LOOP
//===============================================================
void loop() {
    unsigned long currentMillis = millis();

    // State machine for DARK and RAINBOW phases
    switch (currentPhase) {
      case PHASE_DARK:
        if (currentMillis - phaseStartTime >= DARK_DURATION) {
          Serial.println("Dark phase ended. Starting 20-second RAINBOW phase...");
          currentPhase = PHASE_RAINBOW;      // Switch to rainbow phase
          phaseStartTime = currentMillis;      // Reset phase timer
          rainbowEffectActive = true;        // Enable the rainbow effect
          rainbowCycleJ = 0;                 // Reset rainbow position
          lastRainbowUpdateTime = currentMillis; // Set time for first rainbow update
        }
        break;

      case PHASE_RAINBOW:
        // Update the rainbow effect if it's active and time for the next frame
        if (rainbowEffectActive && (currentMillis - lastRainbowUpdateTime >= rainbowUpdateInterval)) {
            lastRainbowUpdateTime = currentMillis; // Record the time of this update
            updateRainbowEffect();                 // Update pixel colors
        }

        // Check if the rainbow duration has passed
        if (currentMillis - phaseStartTime >= RAINBOW_DURATION) {
          Serial.println("Rainbow phase ended. Starting 5-second DARK phase...");
          currentPhase = PHASE_DARK;        // Switch back to dark phase
          phaseStartTime = currentMillis;       // Reset phase timer
          rainbowEffectActive = false;         // Disable the rainbow effect
          strip.clear();                      // Turn all pixels off
          strip.show();                       // Apply the change
        }
        break;
    }
    // A very small delay can be added if desired, but the timing is handled by millis()
    // delay(1);
}
