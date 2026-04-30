#include <Arduino.h>
#include <ens190.h>
#include <SoftwareSerial.h>

// Define new pins for Arduino Uno (Avoids Pins 0 and 1)
#define rxPin 10  // Connect to ENS190 TX
#define txPin 11  // Connect to ENS190 RX

#define ENS190_DEFAULT_BAUD_RATE    9600
#define OUTPUT_BAUD_RATE            9600

ENS190 ens190;
SoftwareSerial softwareSerial(rxPin, txPin);

void setup()
{
    // Hardware Serial for the Serial Monitor
    Serial.begin(OUTPUT_BAUD_RATE);
    while(!Serial); // Wait for Serial Monitor to open
    Serial.println("ENS190 SoftwareSerial Test");

    // Start SoftwareSerial for the sensor
    softwareSerial.begin(ENS190_DEFAULT_BAUD_RATE);
    
    // Pass the SoftwareSerial object to the library
    ens190.begin(&softwareSerial);

    while (ens190.init() == false)
    {
        Serial.println("Error -- The ENS190 is not connected.");
        delay(1000);
    }

    Serial.print("ENS190 Serial No.: ");
    Serial.println((unsigned long)ens190.serialNumber);

    Serial.print("FW version: ");
    Serial.println((char*)ens190.fwVersion);
}

void loop()
{
    if (ens190.update() == RESULT_OK)
    {
        Serial.print("CO2[ppm]: ");
        Serial.println(ens190.getCo2());
    }
    
    delay(4000);
}
