// Define Blynk credentials and template info
#define BLYNK_TEMPLATE_ID "TMPL3cvqphiWn"
#define BLYNK_TEMPLATE_NAME "Watts Smart Wizard"
#define BLYNK_PRINT Serial

#include <LiquidCrystal_I2C.h>    // For LCD display
#include <EmonLib.h>              // For energy monitoring
#include <EEPROM.h>               // For storing data in EEPROM
#include <WiFi.h>                 // For Wi-Fi connection
#include <WiFiClient.h>           // For client communication
#include <BlynkSimpleEsp32.h>     // For Blynk connection
#include <HTTPClient.h>           // For HTTP POST requests

// Initialize LCD and energy monitor
LiquidCrystal_I2C lcd(0x27, 20, 4);
EnergyMonitor emon;

// Blynk credentials // HIDDEN for security
char auth[] = "3P3MOCTij9_YMXJC8UvHsjSM3V4Sx###";
char ssid[] = "Redm###";
char pass[] = "##";

// Energy monitor calibration constants
#define vCalibration 83.3
#define currCalibration 0.50

BlynkTimer timer;
float kWh = 0;
unsigned long lastmillis = millis();

// Function to send data to Blynk and Intel Tiber Cloud
void myTimerEvent()
{
  emon.calcVI(20, 2000);  // Sample the current and voltage
  kWh += emon.apparentPower * (millis() - lastmillis) / 3600000000.0;
  lastmillis = millis();

  // Store data in EEPROM
  EEPROM.put(0, emon.Vrms);
  EEPROM.put(4, emon.Irms);
  EEPROM.put(8, emon.apparentPower);
  EEPROM.put(12, kWh);

  // Display data on Serial Monitor
  Serial.print("Vrms: ");
  Serial.print(emon.Vrms, 2);
  Serial.print("V\tIrms: ");
  Serial.print(emon.Irms, 4);
  Serial.print("A\tPower: ");
  Serial.print(emon.apparentPower, 4);
  Serial.print("W\tkWh: ");
  Serial.println(kWh, 5);

  // Update LCD display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vrms: ");
  lcd.print(emon.Vrms, 2);
  lcd.print(" V");
  lcd.setCursor(0, 1);
  lcd.print("Irms: ");
  lcd.print(emon.Irms, 4);
  lcd.print(" A");
  lcd.setCursor(0, 2);
  lcd.print("Power: ");
  lcd.print(emon.apparentPower, 4);
  lcd.print(" W");
  lcd.setCursor(0, 3);
  lcd.print("kWh: ");
  lcd.print(kWh, 4);

  // Send data to Blynk
  Blynk.virtualWrite(V0, emon.Vrms);
  Blynk.virtualWrite(V1, emon.Irms);
  Blynk.virtualWrite(V2, emon.apparentPower);
  Blynk.virtualWrite(V3, kWh);

  // Send data to Intel Tiber Cloud via HTTP
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://tiber-cloud-url.com/data"); // Replace it with Intel Tiber Cloud endpoint
    http.addHeader("Content-Type", "application/json");

    // Create JSON payload
    String postData = "{\"Vrms\":";
    postData += String(emon.Vrms, 2);
    postData += ",\"Irms\":";
    postData += String(emon.Irms, 4);
    postData += ",\"Power\":";
    postData += String(emon.apparentPower, 4);
    postData += ",\"kWh\":";
    postData += String(kWh, 5);
    postData += "}";

    // Send the HTTP POST request
    int httpResponseCode = http.POST(postData);

    // Print response
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response: " + response);
    } else {
      Serial.println("Error in sending POST request: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize Blynk connection
  Blynk.begin(auth, ssid, pass);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize energy monitor
  emon.voltage(35, vCalibration, 1.7);  // Voltage calibration
  emon.current(34, currCalibration);    // Current calibration

  // Timer to run the function every 5 seconds
  timer.setInterval(5000L, myTimerEvent);

  // Display initial message on LCD
  lcd.setCursor(3, 0);
  lcd.print("Wattsmart Wizard");
  delay(3000);
  lcd.clear();
}

void loop()
{
  // Run Blynk and timer
  Blynk.run();
  timer.run();
}
