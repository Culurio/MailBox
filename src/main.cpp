#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>
#include <addons/RTDBHelper.h>
#include "../lib/secret.h"
#include "WiFiHelper.h"
#include "FirebaseHelper.h"
#include "LedController.h"
#include "SensorHandler.h"
#include "LockHandler.h"

Servo myServo;

unsigned long sendDataPrevMillis = 0;
const int BUTTON_PIN = 22;
const int BUZZER_PIN = 15;
bool lastButtonState = LOW;

void syncLedAndServo();
void handleButtonState();
void checkBuzzerCondition();

void setup()
{
  Serial.begin(115200);
  connectToWiFi();
  connectToFirebase();
  
  setupLedPins();
  setupSensor();
  setupLockSwitch();
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  myServo.attach(18);
  myServo.write(0);
}

void loop()
{
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    updateMailboxStatusColor();
    handleMotionSensor();
    handleLockState();
    syncLedAndServo();
    handleButtonState();
    checkBuzzerCondition();
  }
}

void syncLedAndServo()
{
  bool lock;
  if (Firebase.RTDB.getBool(&fbdo, "/led/locked", &lock))
  {
    Serial.printf("Received state: %d\n", lock);
    myServo.write(lock ? 90 : 0);
  }
  else
  {
    Serial.printf("Firebase Error: %s\n", fbdo.errorReason().c_str());
  }
}


void handleButtonState() {
  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    Serial.println("Button pressed: Toggling lock state.");

    bool locked;
    if (Firebase.RTDB.getBool(&fbdo, "/led/locked", &locked)) {
      bool newLockedState = !locked;

      if (Firebase.RTDB.setBool(&fbdo, "/led/locked", newLockedState)) {
        Serial.printf("Lock state updated to %s in Firebase.\n", newLockedState ? "LOCK" : "UNLOCK");
      } else {
        Serial.printf("Firebase Error: %s\n", fbdo.errorReason().c_str());
      }
    } else {
      Serial.printf("Firebase Error (read): %s\n", fbdo.errorReason().c_str());
    }
  }

  lastButtonState = currentButtonState;
  delay(100);
}

void checkBuzzerCondition() {
  bool locked = false;
  bool closed = true;

  // Read lock state
  if (Firebase.RTDB.getBool(&fbdo, "/led/locked", &locked)) {
    Serial.printf("Locked state: %s\n", locked ? "true" : "false");
  } else {
    Serial.printf("Firebase Error (locked): %s\n", fbdo.errorReason().c_str());
  }

  // Read door state
  if (Firebase.RTDB.getBool(&fbdo, "/closed", &closed)) {
    Serial.printf("Closed state: %s\n", closed ? "true" : "false");
  } else {
    Serial.printf("Firebase Error (closed): %s\n", fbdo.errorReason().c_str());
  }

  // Determine buzzer state
  if (locked && !closed) {
    Serial.println("Buzzer condition met: turning ON buzzer.");
    digitalWrite(BUZZER_PIN, HIGH);  
  } else {
    digitalWrite(BUZZER_PIN, LOW);   
  }
}
