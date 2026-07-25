#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#define TRIG 5
#define ECHO 18
#define RAIN 19
#define FLOW 27
#define SOIL 34

#define HISTORY_SIZE 10
#define LOOP_DELAY 2000        // 2 seconds - good for demo

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const char* ssid = "Arth";
const char* password = "11111112";

String apiKey = "AIzaSyAxyL_QmQZD6QB0GiXNMgK7mnuVNhvSJwk";
String databaseURL = "https://flood-monitoring-dde28-default-rtdb.firebaseio.com";

float previousLevel = 0;
float flowRate = 0;
volatile int pulseCount = 0;

float levelHistory[HISTORY_SIZE] = {0};
bool rainHistory[HISTORY_SIZE] = {false};
int historyIndex = 0;
float previousRiseRate = 0;

bool TEST_MODE = true;   // Set to false for real deployment

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n===== FLOOD MONITOR - FULL VERSION WITH FLOW RATE =====");

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT_PULLUP);
  pinMode(RAIN, INPUT);
  pinMode(FLOW, INPUT);
  pinMode(SOIL, INPUT);

  attachInterrupt(digitalPinToInterrupt(FLOW), pulseCounter, FALLING);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  config.api_key = apiKey.c_str();
  config.database_url = databaseURL.c_str();
  Firebase.signUp(&config, &auth, "", "");
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (Firebase.ready()) Serial.println("✅ Firebase READY");
  else Serial.println("❌ Firebase NOT ready");

  Serial.println("===== SYSTEM READY - FLOW RATE ENABLED =====\n");
}

float readWaterLevel() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(15);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH, 40000UL);
  if (duration == 0) return previousLevel > 0 ? previousLevel : 0;

  float distance = duration * 0.034 / 2;
  return constrain((100.0 - distance) / 100.0 * 100.0, 0, 100);
}

void loop() {
  float waterLevel = readWaterLevel();
  float riseRate = (waterLevel - previousLevel) / (LOOP_DELAY / 60000.0f);
  previousLevel = waterLevel;

  bool isRaining = (digitalRead(RAIN) == LOW);
  int soilRaw = analogRead(SOIL);
  float soilPercent = map(soilRaw, 4095, 0, 0, 100);

  // Flow Rate
  flowRate = pulseCount / 7.5f;   // Your sensor calibration
  pulseCount = 0;

  // Flood Risk (Flow rate now included)
  float floodRisk = (0.4 * waterLevel) + 
                    (0.25 * riseRate * 10) + 
                    (0.2 * (isRaining ? 100 : 0)) + 
                    (0.10 * soilPercent) + 
                    (0.05 * flowRate * 2);   // Flow contribution

  // ================== CAUSE CLASSIFICATION (Improved with Flow Rate) ==================
  String cause = "Normal";
  String reason = "";

  if (TEST_MODE) {
    // Demo-friendly triggers
    if (waterLevel > 65 && flowRate < 8) {
      cause = "Drain Blockage";
      reason = "High water + very low flow";
    } 
    else if (isRaining && waterLevel > 45 && riseRate > 0.8) {
      cause = "Heavy Rain Flood";
      reason = "Raining + rising water + wet soil";
    } 
    else if (!isRaining && riseRate > 1.5 && flowRate > 5) {
      cause = "River Overflow";
      reason = "Fast rise + good flow (River)";
    } 
    else if (riseRate > 3.0 && flowRate > 12) {
      cause = "Dam Release";
      reason = "Very fast rise + high flow";
    }
  } else {
    // Original logic with flow rate
    if (isRaining && soilPercent > 70 && riseRate > 1) {
      cause = "Heavy Rain Flood";
      reason = "Heavy rain + saturated soil";
    }
    else if (!isRaining && riseRate > 3 && flowRate > 10) {
      cause = "River Overflow";
      reason = "No rain + high flow + fast rise";
    }
    else if (isRaining && flowRate < 5 && waterLevel > 60) {
      cause = "Drain Blockage";
      reason = "Rain + blocked flow + high level";
    }
    else if (!isRaining && riseRate > 5 && flowRate > 20) {
      cause = "Dam Release";
      reason = "No rain + very fast rise + high flow";
    }
  }

  // Update history for prediction
  levelHistory[historyIndex] = waterLevel;
  rainHistory[historyIndex] = isRaining;
  historyIndex = (historyIndex + 1) % HISTORY_SIZE;

  // Prediction Calculation
  float avgRiseRate = 0;
  int valid = 0;
  for (int i = 1; i < HISTORY_SIZE; i++) {
    int prev = (historyIndex - i + HISTORY_SIZE) % HISTORY_SIZE;
    int curr = (prev + 1) % HISTORY_SIZE;
    avgRiseRate += (levelHistory[curr] - levelHistory[prev]);
    valid++;
  }
  if (valid > 0) avgRiseRate /= valid;

  int rainCount = 0;
  for (int i = 0; i < HISTORY_SIZE; i++) if (rainHistory[i]) rainCount++;
  float rainDurationFactor = (rainCount / (float)HISTORY_SIZE) * 100;

  float acceleration = riseRate - previousRiseRate;
  previousRiseRate = riseRate;

  float predictionScore = (0.35 * constrain(avgRiseRate * 10, 0, 100)) +
                          (0.25 * constrain(acceleration * 20, -50, 100)) +
                          (0.20 * soilPercent) +
                          (0.15 * flowRate * 2) +           // Flow rate in prediction
                          (0.05 * rainDurationFactor);

  float futureLevel20 = waterLevel + (avgRiseRate * 20) + (0.5 * acceleration * 400);
  float futureLevel40 = waterLevel + (avgRiseRate * 40) + (0.5 * acceleration * 1600);

  String prediction = "No Prediction";
  if (predictionScore > 75 && futureLevel40 > 75) {
    prediction = "High Probability Flood in ~40 min";
  } else if (predictionScore > 60 && futureLevel20 > 70) {
    prediction = "High Probability Flood in ~20 min";
  } else if (predictionScore > 45) {
    prediction = "Moderate Flood Risk Developing";
  }

  // ================== DISPLAY ==================
  Serial.println("\n==================================================");
  Serial.printf("Water Level : %.1f%%   |   Rise Rate: %.2f %%/min\n", waterLevel, riseRate);
  Serial.printf("Rain: %s   |   Soil: %.1f%%   |   Flow Rate: %.2f\n", 
                isRaining ? "YES" : "NO", soilPercent, flowRate);
  Serial.printf("Flood Risk  : %.1f%%   |   Cause: %s\n", floodRisk, cause.c_str());
  Serial.println("Reason: " + reason);
  Serial.printf("Prediction  : %s   (Score: %.1f)\n", prediction.c_str(), predictionScore);
  Serial.printf("Future → 20min: %.1f%%    |    40min: %.1f%%\n", futureLevel20, futureLevel40);
  Serial.println("==================================================");

  // Upload to Firebase
  if (Firebase.ready()) {
    bool ok = true;
    ok &= Firebase.RTDB.setFloat(&fbdo, "/FloodData/WaterLevel", waterLevel);
    ok &= Firebase.RTDB.setFloat(&fbdo, "/FloodData/RiseRate", riseRate);
    ok &= Firebase.RTDB.setFloat(&fbdo, "/FloodData/FlowRate", flowRate);
    ok &= Firebase.RTDB.setFloat(&fbdo, "/FloodData/SoilMoisture", soilPercent);
    ok &= Firebase.RTDB.setBool(&fbdo, "/FloodData/IsRaining", isRaining);
    ok &= Firebase.RTDB.setString(&fbdo, "/FloodData/Cause", cause);
    ok &= Firebase.RTDB.setFloat(&fbdo, "/FloodData/Risk", floodRisk);
    ok &= Firebase.RTDB.setString(&fbdo, "/FloodData/Prediction", prediction);
    ok &= Firebase.RTDB.setFloat(&fbdo, "/FloodData/FutureLevel20", futureLevel20);
    ok &= Firebase.RTDB.setFloat(&fbdo, "/FloodData/FutureLevel40", futureLevel40);

    if (ok) Serial.println("✅ All data uploaded to Firebase");
    else Serial.println("❌ Upload failed");
  }

  delay(LOOP_DELAY);
}