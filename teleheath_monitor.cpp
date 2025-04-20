Code:
#define BLYNK_TEMPLATE_ID "TMPL66kx7AtR_"
#define BLYNK_TEMPLATE_NAME "Telehealth"
#include <Wire.h>
#include <Blynk.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define BLYNK_PRINT Serial
// Blynk and Wi-Fi credentials
char auth[] = "a_37QCTlvSKhfAn8wo2NVKSD_0W_Jfv4";
char ssid[] = "Tenda";
char pass[] = "sharma123";
// Pin configuration
#define DHTTYPE DHT11
#define DHT_PIN 18             // DHT11 sensor pin
#define DS18B20_PIN 5          // DS18B20 temperature sensor pin
#define HW827_PIN 34           // Analog pin for HW827 sensor
#define OLED_RESET  -1        // OLED reset pin (not used)
// Create SSD1306 OLED display object
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
// Define reporting period for serial updates (in milliseconds)
#define REPORTING_PERIOD_MS 1000
// Sensor objects
DHT dht(DHT_PIN, DHTTYPE);
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
// Variables for storing sensor data
float roomTemperature, roomHumidity;
float bodyTemperature, heartRate;
uint32_t lastReportTime = 0;
// Constants
const int threshold = 600;    // Adjust threshold based on your HW827 sensor's output (Test and tune this)
bool pulseDetected = false;   // Flag for pulse detection
unsigned long lastPulseTime = 0;
unsigned long pulseInterval = 0;
unsigned long lastDebounceTime = 0;  // For debouncing the pulse signal
unsigned long debounceDelay = 50;     // Delay in ms for debouncing
void setup() {
Serial.begin(115200);
Blynk.begin(auth, ssid, pass);
    // Initialize OLED display
 if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
 Serial.println("SSD1306 allocation failed.");
 while (1); // Halt if OLED initialization fails
 }
 display.clearDisplay();
 display.setTextSize(1);
 display.setTextColor(SSD1306_WHITE);
 display.display();
    // Initialize sensors
 dht.begin();
 sensors.begin();
 Serial.println("Initializing sensors...");
}
void loop() {
Blynk.run();
    // Read DHT11 sensor data
 roomTemperature = dht.readTemperature();
 roomHumidity = dht.readHumidity();
    // Read DS18B20 body temperature data
 sensors.requestTemperatures();
 bodyTemperature = sensors.getTempCByIndex(0);
    // Check for valid DS18B20 reading
 if (bodyTemperature == DEVICE_DISCONNECTED_C) {
 Serial.println("Error: DS18B20 not detected.");
bodyTemperature = 0.0;
}
    // Read HW827 heart rate sensor data
 int hw827Value = analogRead(HW827_PIN);
    // Implement debouncing to filter out noise
 if (millis() - lastDebounceTime > debounceDelay) {
if (hw827Value > threshold && !pulseDetected) {
pulseDetected = true;
unsigned long currentTime = millis();
 if (lastPulseTime > 0) {
 pulseInterval = currentTime - lastPulseTime;
 if (pulseInterval > 0) {
                    // Calculate heart rate (BPM)
heartRate = 60000.0 / pulseInterval; // Convert milliseconds to BPM
 }
 }
lastPulseTime = currentTime;
} else if (hw827Value < threshold) {
pulseDetected = false;
}
lastDebounceTime = millis();
}
    // Ensure readings are updated at the defined interval
if (millis() - lastReportTime > REPORTING_PERIOD_MS) {
Serial.print("Heart Rate (HW827): ");
Serial.print(heartRate);
Serial.println(" bpm");
Serial.print("Room Temperature: ");
Serial.print(roomTemperature);
Serial.println("°C");
Serial.print("Room Humidity: ");
Serial.print(roomHumidity);
Serial.println("%");
Serial.print("Body Temperature (DS18B20): ");
Serial.print(bodyTemperature);
Serial.println("°C");
        // Update OLED display
display.clearDisplay();
display.setCursor(0, 0);
display.println("Telehealth Monitoring");
display.println("--------------------");
display.print("Heart Rate: ");
display.print(heartRate);
display.println(" bpm");
display.print("Room Temp: ");
display.print(roomTemperature);
display.println(" C");
display.print("Humidity: ");
display.print(roomHumidity);
display.println(" %");
display.print("Body Temp: ");
display.print(bodyTemperature);
display.println(" C");
display.display();
        // Send data to Blynk
Blynk.virtualWrite(V3, roomTemperature);       // Room Temperature
 Blynk.virtualWrite(V4, roomHumidity);          // Room Humidity
 Blynk.virtualWrite(V5, heartRate);             // Heart rate (BPM)
 Blynk.virtualWrite(V7, bodyTemperature);       // Body Temperature
lastReportTime = millis();
}
}
