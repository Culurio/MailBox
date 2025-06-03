#include "LockHandler.h"
#include <Arduino.h>
#include "FirebaseHelper.h"

const int REED_SWITCH_PIN = 23;
bool lastLockState = HIGH;

void setupLockSwitch() {
  pinMode(REED_SWITCH_PIN, INPUT_PULLUP);
}

void handleLockState() {
  bool currentState = digitalRead(REED_SWITCH_PIN); 

  if (currentState != lastLockState) {
    lastLockState = currentState;

    // Determine door status (true for closed, false for open)
    //I have inverted the logic soo the buzzer doesnt go on everytime
    
    bool doorClosed = (currentState == LOW); 

    if (doorClosed) {
      Serial.println("Magnet detected: Door is CLOSED (locked)");
    } else {
      Serial.println("Magnet not detected: Door is OPEN (unlocked or removed)");
    }

    // Send door status to Firebase
    if (Firebase.RTDB.setBool(&fbdo, "/closed", doorClosed)) {
      Serial.println("Door status updated to Firebase.");
    } else {
      Serial.printf("Firebase Error: %s\n", fbdo.errorReason().c_str());
    }
  }

  delay(100); // debounce delay
}
