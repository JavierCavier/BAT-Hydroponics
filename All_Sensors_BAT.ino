#include <Arduino.h>
#include <LiquidCrystal.h>
#include <ens190.h>
#include <SoftwareSerial.h>

// --- PIN DEFINITIONS ---
const int buttonPin = 7;
const int trigPin = 9;   // [cite: 10]
const int echoPin = 8;   // [cite: 11]
#define TdsSensorPin A1  // 
#define pHSensorPin A0   // 
#define waterLevelPin A2 // 
#define rxPin 10         // 
#define txPin 11         // 

// --- OBJECTS & CONSTANTS ---
LiquidCrystal lcd(12, 13, 5, 4, 3, 2); 
ENS190 ens190;
SoftwareSerial softwareSerial(rxPin, txPin); // [cite: 2]

int displayMode = 0; // 0: pH, 1: TDS, 2: Water, 3: Dist, 4: CO2
const int totalModes = 5;

// TDS Variables
#define SCOUNT 30
int analogBuffer[SCOUNT];
int analogBufferIndex = 0;
float tdsValue = 0;

// pH Variables
#define ArrayLenth 40
int pHArray[ArrayLenth];
int pHArrayIndex = 0;
float pHValue = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(TdsSensorPin, INPUT);
  
  softwareSerial.begin(9600); // [cite: 4]
  ens190.begin(&softwareSerial); // [cite: 5]
  
  lcd.print("System Loading");
  delay(1000);
}

void loop() {
  checkButton();
  updateSensors();
  updateDisplay();
}

// --- BUTTON LOGIC ---
void checkButton() {
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(buttonPin);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    displayMode = (displayMode + 1) % totalModes;
    lcd.clear();
    delay(200); // Debounce
  }
  lastButtonState = currentButtonState;
}

// --- SENSOR PROCESSING ---
void updateSensors() {
  // 1. pH Sampling [cite: 41, 42]
  static unsigned long phTime = 0;
  if (millis() - phTime > 20) {
    pHArray[pHArrayIndex++] = analogRead(pHSensorPin);
    if (pHArrayIndex == ArrayLenth) pHArrayIndex = 0;
    float voltage = (analogRead(pHSensorPin) * 5.0) / 1024.0;
    pHValue = 3.5 * voltage; 
    phTime = millis();
  }

  // 2. TDS Sampling [cite: 28, 35]
  static unsigned long tdsTime = 0;
  if (millis() - tdsTime > 40) {
    analogBuffer[analogBufferIndex++] = analogRead(TdsSensorPin);
    if (analogBufferIndex == SCOUNT) analogBufferIndex = 0;
    tdsTime = millis();
    // Simplified TDS math for integration
    float voltage = analogRead(TdsSensorPin) * 5.0 / 1024.0;
    tdsValue = (133.42 * pow(voltage, 3) - 255.86 * pow(voltage, 2) + 857.39 * voltage) * 0.5;
  }
}

// --- LCD & SERIAL OUTPUT ---
void updateDisplay() {
  static unsigned long printTime = 0;
  if (millis() - printTime < 800) return;
  printTime = millis();

  lcd.setCursor(0, 0);
  switch (displayMode) {
    case 0: // pH
      lcd.print("pH Level:      ");
      lcd.setCursor(0, 1);
      lcd.print(pHValue, 2);
      Serial.print("pH: "); Serial.println(pHValue);
      break;

    case 1: // TDS
      lcd.print("TDS Value:     ");
      lcd.setCursor(0, 1);
      lcd.print(tdsValue, 0); lcd.print(" ppm    ");
      Serial.print("TDS: "); Serial.println(tdsValue);
      break;

    case 2: // Water Level [cite: 16]
      lcd.print("Water Value:   ");
      lcd.setCursor(0, 1);
      lcd.print(analogRead(waterLevelPin));
      Serial.print("Water: "); Serial.println(analogRead(waterLevelPin));
      break;

    case 3: // Distance [cite: 13]
      digitalWrite(trigPin, LOW); delayMicroseconds(2);
      digitalWrite(trigPin, HIGH); delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      float dist = (pulseIn(echoPin, HIGH) * 0.0343) / 2;
      lcd.print("Distance:      ");
      lcd.setCursor(0, 1);
      lcd.print(dist); lcd.print(" cm    ");
      Serial.print("Dist: "); Serial.println(dist);
      break;

    case 4: // CO2 [cite: 9]
      ens190.update();
      lcd.print("CO2 Level:     ");
      lcd.setCursor(0, 1);
      lcd.print(ens190.getCo2()); lcd.print(" ppm    ");
      Serial.print("CO2: "); Serial.println(ens190.getCo2());
      break;
  }
}