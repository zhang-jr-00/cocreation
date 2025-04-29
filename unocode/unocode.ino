#include <Servo.h>

Servo motorESC1;
Servo motorESC2;

const int ledPin = 10;  // Built-in  13
unsigned long previousMillis = 0;
int ledState = LOW;
int currentStatus = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  while (!Serial);  // Wait for serial connection
  Serial.println("Status Receiver Ready");
  motorESC1.attach(13);  // ESC 1 connected to pin D13
  motorESC2.attach(12);  // ESC 2 connected to pin D12

  // Set both ESCs to neutral (stop initially)
  motorESC1.writeMicroseconds(1500);
  motorESC2.writeMicroseconds(1500);

  delay(3000); // Wait 5 seconds for ESC initialization
}

void loop() {
  // Check for incoming serial data
  if (Serial.available() > 0) {
    String received = Serial.readStringUntil('\n');
    received.trim();  // Remove any whitespace/newline
    
    int newStatus = received.toInt();
    
    if (newStatus >= 0 && newStatus <= 2) {
      currentStatus = newStatus;
      Serial.print("Status changed to: ");
      Serial.println(currentStatus);
      
      // Immediately update LED state when status changes
      if (currentStatus == 0) {
        digitalWrite(ledPin, LOW);
      }
    }
  }

  // Handle LED blinking patterns
  unsigned long currentMillis = millis();
  
  switch(currentStatus) {
    case 0:  // Off
      digitalWrite(ledPin, LOW);
       // Forward (60% speed)
      motorESC1.writeMicroseconds(1500);
      motorESC2.writeMicroseconds(1500);
      delay(500); // Run for 1 second
      break;
      
    case 1:  // Blink (500ms interval)
      motorESC1.writeMicroseconds(1500);
      motorESC2.writeMicroseconds(1800);
      delay(280); // Run for 1 second
      motorESC1.writeMicroseconds(1500);
      motorESC2.writeMicroseconds(1800);
      delay(220); // Run for 1 second
      // if (currentMillis - previousMillis >= 500) {
      //   previousMillis = currentMillis;
      //   ledState = !ledState;
      //   digitalWrite(ledPin, ledState);
      // }
      break;
      
    case 2:  // Fast blink (250ms interval)
      motorESC1.writeMicroseconds(1800);
      motorESC2.writeMicroseconds(1500);
      delay(250); // Run for 1 second
      motorESC1.writeMicroseconds(1200);
      motorESC2.writeMicroseconds(1500);
      delay(250); // Run for 1 second
      // if (currentMillis - previousMillis >= 250) {
      //   previousMillis = currentMillis;
      //   ledState = !ledState;
      //   digitalWrite(ledPin, ledState);
      // }
      break;
  }
}
