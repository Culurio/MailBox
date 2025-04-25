#include <Arduino.h>
#include "WiFi.h"
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <ESP32Servo.h>
#include "../lib/secret.h"


unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
bool lastSensorState = HIGH;

// Pins
const int IR_SENSOR_PIN = 19;
const int RED_PIN = 21;
const int GREEN_PIN = 2;
const int BLUE_PIN = 4;
const int SERVO_PIN = 18;

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

Servo myServo;

void connectToWifi();
void connectToFirebase();
void handleMotionSensor();
void syncLedAndServo();
void setColor(int redValue, int greenValue, int blueValue);
void updateMailboxStatusColor();

void setup()
{
  Serial.begin(115200);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  setColor(255, 0, 0); // Red Colo
  setColor(0,  255, 0); // Green Color
  setColor(0, 0, 255); // Blue Color
  pinMode(IR_SENSOR_PIN, INPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(0);

  connectToWifi();

  connectToFirebase();
}

void loop()
{
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    updateMailboxStatusColor();
    handleMotionSensor();
    syncLedAndServo();
  }
}

void handleMotionSensor()
{
  bool currentSensorState = digitalRead(IR_SENSOR_PIN);

  if (lastSensorState == HIGH && currentSensorState == LOW)
  {
    Serial.println("Motion edge detected: adding 1 to /packages");

    int numPackages = 0;
    if (Firebase.RTDB.getInt(&fbdo, "/packages", &numPackages))
    {
      Firebase.RTDB.setInt(&fbdo, "/packages", numPackages + 1);
    }
    else
    {
      Firebase.RTDB.setInt(&fbdo, "/packages", 1);
    }
  }

  lastSensorState = currentSensorState;
}

void updateMailboxStatusColor()
{
  int packageCount = 0;
  int servoState = 0;

  bool hasPackage = Firebase.RTDB.getInt(&fbdo, "/packages", &packageCount) && packageCount > 0;
  bool hasServoState = Firebase.RTDB.getInt(&fbdo, "/led/state", &servoState);

  if (hasPackage) {
    setColor(0, 0, 255);
  } else if (hasServoState) {
    if (servoState == 1) {
      setColor(255, 0, 0);
    } else {
      setColor(0, 255, 0);
    }
  } else {
    setColor(255, 255, 255);
  }
}

void syncLedAndServo()
{
  int ledState;
  if (Firebase.RTDB.getInt(&fbdo, "/led/state", &ledState))
  {
    Serial.printf("Received state: %d\n", ledState);
    //digitalWrite(LED_PIN, ledState == 1 ? HIGH : LOW);
    myServo.write(ledState == 1 ? 90 : 0);
  }
  else
  {
    Serial.printf("Firebase Error: %s\n", fbdo.errorReason().c_str());
  }
}

void connectToWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void setColor(int redValue, int greenValue, int blueValue)
{
  analogWrite(RED_PIN, redValue);
  analogWrite(GREEN_PIN, greenValue);
  analogWrite(BLUE_PIN, blueValue);
}

void connectToFirebase()
{
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  auth.user.email = EMAIL;
  auth.user.password = PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}