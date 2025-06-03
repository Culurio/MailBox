#include "LedController.h"
#include <Arduino.h>
#include "FirebaseHelper.h"

const int RED_PIN = 21;
const int GREEN_PIN = 2;
const int BLUE_PIN = 4;

void setupLedPins() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void setColor(int red, int green, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

void updateMailboxStatusColor() {
  int packageCount = 0;
  bool servoState = 0;

  bool hasPackage = Firebase.RTDB.getInt(&fbdo, "/lastPackageNumber", &packageCount) && packageCount > 0;
  bool hasServoState = Firebase.RTDB.getBool(&fbdo, "/led/locked", &servoState);

  if (hasPackage) {
    setColor(0, 0, 255); 
  } else if (hasServoState) {
    if (servoState) {
      setColor(255, 0, 0); 
    } else {
      setColor(0, 255, 0); 
    }
  } else {
    setColor(255, 255, 255);
  }
}
