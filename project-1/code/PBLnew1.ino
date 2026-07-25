#include <TinyStepper_28BYJ_48.h>
#include <Servo.h>

// Definitions
Servo servo1;
#define IR_SENSOR 5     // IR sensor pin
#define PROXIMITY 6     // Inductive Proximity sensor pin
#define BUZZER 12       // Buzzer pin
int SOIL_SENSOR = A0;   // Soil moisture sensor pin
int soil = 0;           // Variable to store soil reading
int fsoil = 0;          // Variable to store averaged soil reading

// Create stepper object
TinyStepper_28BYJ_48 stepper;

void setup() {
  Serial.begin(9600);

  // Pin modes
  pinMode(PROXIMITY, INPUT);  
  pinMode(IR_SENSOR, INPUT);   
  pinMode(BUZZER, OUTPUT);    

  // Attach servo to pin 7
  servo1.attach(7);

  // Initialize servo position
  servo1.write(180);
  delay(1000);
  servo1.write(70);
  delay(1000);

  // Initialize stepper motor
  stepper.connectToPins(8, 9, 10, 11);
}

void loop() {
  fsoil = 0;  

  // 🔹 Read Metal Detection Sensor (Inductive Proximity Sensor)
  int metalDetected = digitalRead(PROXIMITY);
  Serial.print("Proximity Sensor (Metal Detection): ");
  Serial.println(metalDetected);

  if (metalDetected == 1) {  // **Metal Waste Detected**
    Serial.println("⚠️ Metal Waste Detected! Rotating bin...");

    tone(BUZZER, 1000, 500);
    delay(500);

    // **Move bin to Metal Waste position (180°)**
    Serial.println("🔄 Moving bin to Metal Waste position...");
    stepper.moveRelativeInSteps(1024);  // Move to Metal Waste
    delay(1000);

    // **Open bin lid**
    servo1.write(180);
    delay(1000);
    servo1.write(70);
    delay(1000);

    // **Reset bin to original position**
    Serial.println("↩ Returning bin to original position...");
    stepper.moveRelativeInSteps(-1024);
    delay(1000);
  }

  // 🔹 Read IR Sensor (General Object Detection)
  int irDetected = digitalRead(IR_SENSOR);
  Serial.print("IR Sensor Value: ");
  Serial.println(irDetected);

  if (irDetected == 0) { // **Waste Detected**
    Serial.println("⚠️ General Waste Detected!");
    tone(BUZZER, 1000, 500);
    delay(1000);

    // Read Soil Moisture Sensor
    soil = 0;
    for (int i = 0; i < 3; i++) {  
      int reading = analogRead(SOIL_SENSOR);
      reading = constrain(reading, 485, 1023);
      soil += map(reading, 485, 1023, 100, 0);
      delay(75);
    }
    fsoil = soil / 3;
    Serial.print("🌱 Soil Moisture: ");
    Serial.print(fsoil);
    Serial.println("%");

    if (fsoil > 20) { // **Wet Waste Detected**
      Serial.println("🔄 Moving bin to Wet Waste position...");
      stepper.moveRelativeInSteps(512);  // Move to Wet Waste
      delay(1000);

      servo1.write(180);
      delay(1000);
      servo1.write(70);
      delay(1000);

      Serial.println("↩ Returning bin to original position...");
      stepper.moveRelativeInSteps(-512);
      delay(1000);
    } else {  // **Dry Waste Detected**
      Serial.println("🔄 Moving bin to Dry Waste position...");
      stepper.moveRelativeInSteps(0);  // Dry Waste is already at original position
      delay(1000);

      servo1.write(180);
      delay(1000);
      servo1.write(70);
      delay(1000);
    }
  }
}
