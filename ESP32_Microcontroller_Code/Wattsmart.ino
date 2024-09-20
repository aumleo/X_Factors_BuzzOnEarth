#define BLYNK_TEMPLATE_ID "TMPL3cvqphiWn"
#define BLYNK_TEMPLATE_NAME "WattSmart Wizard"
#define BLYNK_PRINT Serial

#include <LiquidCrystal_I2C.h>
#include "EmonLib.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Constants for calibration
const float vCalibration = 45.5;
const float currCalibration = 0.15;

// Cost per kWh (you can adjust this)
const float costPerKWh = 10.0; // Assuming 10 USD per kWh

// Blynk and WiFi credentials
char auth[] = "3P3MOCTij9_YMXJC8UvHsjSM3V4SxYRP";
char ssid[] = "Redmijk";
char pass[] = "jitendra";

// Define the relay pin
const int relayPin = 23;

// EnergyMonitor instance
EnergyMonitor emon;

// Timer for regular updates
BlynkTimer timer;

// Variables for energy calculation and cost estimation
float kWh = 0.0;
float estimatedCost = 0.0;
unsigned long lastMillis = millis();

// EEPROM address for kWh variable
const int addrKWh = 0;

// Initialize the LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  // Initialize the relay pin
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Set relay to off initially

  EEPROM.begin(sizeof(kWh)); // Allocate enough bytes for the kWh float value
  readKWhFromEEPROM();

  emon.voltage(35, vCalibration, 1.7); // Voltage: input pin, calibration, phase_shift
  emon.current(34, currCalibration);    // Current: input pin, calibration

  // Pre-calibration cycle
  for (int i = 0; i < 10; i++) { // Run calcVI 10 times to stabilize
    emon.calcVI(20, 2000);
    delay(500);
  }

  timer.setInterval(5000L, sendEnergyDataToBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
  relay_control(); // Check and execute relay commands from HMI
}

// Function to control the relay
void relay_control() {
  static uint8_t Buffer[9];
  
  if (dwin.available()) {
    for (int i = 0; i <= 8; i++) { // Store the whole frame in the buffer array.
      Buffer[i] = dwin.read();
    }

    if (Buffer[0] == 0X5A) {
      // Example command addresses might need to be adjusted
      switch (Buffer[4]) {
        case 0x37: // Example address for relay control
          if (Buffer[8] == 1) {
            digitalWrite(relayPin, HIGH);
            Serial.println("Relay ON");
          } else {
            digitalWrite(relayPin, LOW);
            Serial.println("Relay OFF");
          }
          break;
        // Add cases for other relays or controls if needed
        default:
          break;
      }
    }
  }
}

// Blynk function to control the relay
BLYNK_WRITE(V4) { // Assuming you use Virtual Pin V4 for the relay control
  int relayState = param.asInt();
  digitalWrite(relayPin, relayState); // Control the relay
}

void sendEnergyDataToBlynk() {
  emon.calcVI(20, 2000); // Calculate all. No.of half wavelengths (crossings), time-out

  // Calculate energy consumed in kWh
  unsigned long currentMillis = millis();
  kWh += emon.realPower * (currentMillis - lastMillis) / 3600000000.0;
  estimatedCost = kWh * costPerKWh;
  lastMillis = currentMillis;

  Serial.printf("Vrms: %.2fV\tIrms: %.4fA\tPower: %.4fW\tPower Factor: %.2f\tkWh: %.5fkWh\tEstimated Cost: $%.2f\n",
                emon.Vrms, emon.Irms, emon.realPower, emon.powerFactor, kWh, estimatedCost);

  saveKWhToEEPROM();

  // Display data on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vrms: ");
  lcd.print(emon.Vrms);
  lcd.print(" V");

  lcd.setCursor(0, 1);
  lcd.print("Irms: ");
  lcd.print(emon.Irms);
  lcd.print(" A");

  lcd.setCursor(0, 2);
  lcd.print("Power: ");
  lcd.print(emon.realPower);
  lcd.print(" W");

  lcd.setCursor(0, 3);
  lcd.print("Cost: $");
  lcd.print(estimatedCost);

  // Update Blynk
  Blynk.virtualWrite(V0, emon.Vrms);
  Blynk.virtualWrite(V1, emon.Irms);
  Blynk.virtualWrite(V2, emon.realPower);
  Blynk.virtualWrite(V3, kWh);
}

void readKWhFromEEPROM() {
  EEPROM.get(addrKWh, kWh); 
  if (isnan(kWh)) {
    kWh = 0.0;
    saveKWhToEEPROM();
  }
}

void saveKWhToEEPROM() {
  EEPROM.put(addrKWh, kWh);
  EEPROM.commit();
}
