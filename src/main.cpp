#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fi credentials
#define WIFI_SSID "Malik"
#define WIFI_PASSWORD "Malik@2005"

// Firebase credentials
#define API_KEY "AIzaSyBPoVlzc5CVAN4IoRH8eJNT0rIPkbhp6SM"
#define DATABASE_URL "https://malikee-dd96b-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "sayyadmalik05@gmail.com"
#define USER_PASSWORD "Malik@2152005"

// DHT setup
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Ultrasonic setup
#define TRIG_PIN 13
#define ECHO_PIN 12
const float JAR_HEIGHT_CM = 17.0;

// Firebase setup
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);

  Wire.begin(18, 19);
  lcd.begin(16, 2);
  lcd.backlight();
  dht.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Read ultrasonic
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  float duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;

  float fillPercent = (1.0 - (distance / JAR_HEIGHT_CM)) * 100.0;
  fillPercent = constrain(fillPercent, 0, 100);

  // LCD Display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("T:%.1fC H:%.1f%%", temp, hum);
  lcd.setCursor(0, 1);
  lcd.printf("Fill: %.1f%%", fillPercent);

  // Serial Debug
  Serial.printf("Temp: %.1f°C, Hum: %.1f%%, Fill: %.1f%%\n", temp, hum, fillPercent);

  // Firebase Upload
  if (Firebase.ready()) {
    Firebase.RTDB.setFloat(&fbdo, "/sensor/temperature", temp);
    Firebase.RTDB.setFloat(&fbdo, "/sensor/humidity", hum);
    Firebase.RTDB.setFloat(&fbdo, "/sensor/fill_percentage", fillPercent);
  }

  delay(10000);  // every 10 seconds
}
