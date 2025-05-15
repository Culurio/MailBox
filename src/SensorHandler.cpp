#include "SensorHandler.h"
#include <Arduino.h>
#include "FirebaseHelper.h"

// IR sensor pin
const int IR_SENSOR_PIN = 19;
bool lastSensorState = HIGH;

void setupSensor() {
  pinMode(IR_SENSOR_PIN, INPUT);
}

// Check for package drop and update Firebase
void handleMotionSensor() {
  bool currentSensorState = digitalRead(IR_SENSOR_PIN);

  if (lastSensorState == HIGH && currentSensorState == LOW) {
    Serial.println("Motion edge detected: adding 1 to /packages");

    int numPackages = 0;
    if (Firebase.RTDB.getInt(&fbdo, "/packages", &numPackages)) {
      Firebase.RTDB.setInt(&fbdo, "/packages", numPackages + 1);
    } else {
      Firebase.RTDB.setInt(&fbdo, "/packages", 1);
    }
  }

  lastSensorState = currentSensorState;
}
