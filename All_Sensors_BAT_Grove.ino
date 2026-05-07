#include <Arduino.h>
#include <Wire.h> 
#include <rgb_lcd.h> // The specific library for Grove LCDs
#include <ens190.h>
#include <SoftwareSerial.h>
#include "DHT.h"

// --- CONFIGURABLE VALUES ---
int waterThreshold = 600; 
#define DHTTYPE DHT11     

// --- PIN DEFINITIONS ---
const int buttonPin = 7;
const int ledPin = 6;     
const int dhtPin = 2;     
const int trigPin = 9;    
const int echoPin = 8;    
#define TdsSensorPin A1   
#define pHSensorPin A0    
#define waterLevelPin A2  
#define rxPin 10          
#define txPin 11          

// --- OBJECTS ---
rgb_lcd lcd; // Grove LCD object
ENS190 ens190;
SoftwareSerial softwareSerial(rxPin, txPin); 
DHT dht(dhtPin, DHTTYPE);

int displayMode = 0; 
const int totalModes = 6; 

float humidity, temperatureC;
float tdsValue = 0, pHValue = 0, distance = 0;

void setup() {
  Serial.begin(9600);
  
  // Initialize Grove LCD
  lcd.begin(16, 2);
  lcd.setRGB(0, 255, 0); // Set backlight to Green
  
  dht.begin();
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  softwareSerial.begin(9600);
  ens190.begin(&softwareSerial);
  
  lcd.setCursor(0, 0);
  lcd.print("Grove System");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
  delay(1500);
  lcd.clear();
}

void loop() {
  checkButton();
  readSensors();
  handleWaterLed();
  updateDisplay();
}

void checkButton() {
  static bool lastState = HIGH;
  bool currentState = digitalRead(buttonPin);
  if (lastState == HIGH && currentState == LOW) {
    displayMode = (displayMode + 1) % totalModes;
    lcd.clear();
    delay(200); 
  }
  lastState = currentState;
}

void readSensors() {
  static unsigned long lastSample = 0;
  if (millis() - lastSample < 1000) return; 
  lastSample = millis();

  humidity = dht.readHumidity();
  temperatureC = dht.readTemperature();

  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  distance = (pulseIn(echoPin, HIGH) * 0.0343) / 2;

  float phVolts = analogRead(pHSensorPin) * 5.0 / 1024.0;
  pHValue = 3.5 * phVolts; 

  float tdsVolts = analogRead(TdsSensorPin) * 5.0 / 1024.0;
  tdsValue = (133.42 * pow(tdsVolts, 3) - 255.86 * pow(tdsVolts, 2) + 857.39 * tdsVolts) * 0.5;
}

void handleWaterLed() {
  int level = analogRead(waterLevelPin);
  if (level < waterThreshold) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}

void updateDisplay() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint < 800) return;
  lastPrint = millis();

  lcd.setCursor(0, 0);
  switch (displayMode) {
    case 0: 
      lcd.print("Temp/Humidity: ");
      lcd.setCursor(0, 1);
      lcd.print(temperatureC, 1); lcd.print("C ");
      lcd.print(humidity, 1); lcd.print("%  ");
      break;
    case 1:
      lcd.print("pH Level:      ");
      lcd.setCursor(0, 1);
      lcd.print(pHValue, 2);
      break;
    case 2:
      lcd.print("TDS Value:     ");
      lcd.setCursor(0, 1);
      lcd.print(tdsValue, 0); lcd.print(" ppm   ");
      break;
    case 3:
      lcd.print("Water Level:   ");
      lcd.setCursor(0, 1);
      lcd.print(analogRead(waterLevelPin)); lcd.print("      ");
      break;
    case 4:
      lcd.print("Distance:      ");
      lcd.setCursor(0, 1);
      lcd.print(distance, 1); lcd.print(" cm    ");
      break;
    case 5:
      ens190.update();
      lcd.print("CO2 Level:     ");
      lcd.setCursor(0, 1);
      lcd.print(ens190.getCo2()); lcd.print(" ppm   ");
      break;
  }
}