#include <Arduino.h>
#include "FirebaseHelper.h"
#include "SensorHandler.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const int IR_SENSOR_PIN = 19;
bool lastSensorState = HIGH;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); // UTC

void setupSensor() {
  pinMode(IR_SENSOR_PIN, INPUT);
  timeClient.begin();
}

String getFormattedTimestamp(time_t rawTime) {
  struct tm *timeinfo = gmtime(&rawTime);
  char timestamp[25];
  sprintf(timestamp, "%04d-%02d-%02d_%02d-%02d-%02d",
          timeinfo->tm_year + 1900,
          timeinfo->tm_mon + 1,
          timeinfo->tm_mday,
          timeinfo->tm_hour,
          timeinfo->tm_min,
          timeinfo->tm_sec);
  return String(timestamp);
}

void handleMotionSensor() {
  bool currentSensorState = digitalRead(IR_SENSOR_PIN);

  if (lastSensorState == HIGH && currentSensorState == LOW) {
    Serial.println("Motion detected. Recording new package...");

    timeClient.update();
    time_t rawTime = timeClient.getEpochTime();
    String timestamp = getFormattedTimestamp(rawTime);

    // Get last known package number
    int packageNumber = 1; // default
    if (Firebase.RTDB.getInt(&fbdo, "/lastPackageNumber")) {
      packageNumber = fbdo.intData() + 1;
    }

    // Create JSON with incremented number
    FirebaseJson json;
    json.set("arrival_time", timestamp);
    json.set("pickup_time", "");
    json.set("packagePresent", true);
    
    // Push to Firebase
    if (Firebase.RTDB.pushJSON(&fbdo, "/packages", &json)) {
      String newKey = fbdo.pushName();
      Firebase.RTDB.setInt(&fbdo, "/lastPackageNumber", packageNumber);
      Serial.printf("Package #%d recorded.\n", packageNumber);
    } else {
      Serial.println("Failed to record package arrival.");
    }
  }

  lastSensorState = currentSensorState;
}

