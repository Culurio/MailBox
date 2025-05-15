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

void syncLedAndServo();

void setup()
{
  Serial.begin(115200);

  setupLedPins();
  setupSensor();
  setupLockSwitch();

  myServo.attach(18);
  myServo.write(0);

  connectToWiFi();
  connectToFirebase();
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
  }
}

void syncLedAndServo()
{
  int ledState;
  if (Firebase.RTDB.getInt(&fbdo, "/led/state", &ledState))
  {
    Serial.printf("Received state: %d\n", ledState);
    // digitalWrite(LED_PIN, ledState == 1 ? HIGH : LOW);
    myServo.write(ledState == 1 ? 90 : 0);
  }
  else
  {
    Serial.printf("Firebase Error: %s\n", fbdo.errorReason().c_str());
  }
}
