#include "LockHandler.h"
#include <Arduino.h>
#include "FirebaseHelper.h"

const int REED_SWITCH_PIN = 23;
bool lastLockState = HIGH;

void setupLockSwitch() {
  pinMode(REED_SWITCH_PIN, INPUT_PULLUP);
}

void handleLockState() {
  bool currentState = digitalRead(REED_SWITCH_PIN); // LOW = closed (magnet nearby)
  
  if (currentState != lastLockState) {
    lastLockState = currentState;

    if (currentState == LOW) {
      Serial.println("Magnet detected: Door is CLOSED (locked)");
    } else {
      Serial.println("Magnet not detected: Door is OPEN (unlocked or removed)");
    }
  }

  delay(100); // Debounce and reduce serial spam
}
