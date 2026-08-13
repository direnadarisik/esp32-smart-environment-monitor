#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Wi-Fi settings
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// Pin definitions
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define LED_PIN 2
#define BUZZER_PIN 5

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WebServer server(80);

float temperature = 0.0;
float humidity = 0.0;

const float temperatureLimit = 30.0;

void updateSensorData() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Update OLED display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Environment Monitor");

  display.setCursor(0, 20);
  display.print("Temperature: ");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 35);
  display.print("Humidity: ");
  display.print(humidity, 1);
  display.println(" %");

  display.display();

  // Temperature warning
  if (temperature >= temperatureLimit) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000);
  } else {
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C | Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>ESP32 Environment Monitor</title>";
  html += "</head><body>";

  html += "<h1>ESP32 Environment Monitor</h1>";

  html += "<h2>Temperature: ";
  html += String(temperature, 1);
  html += " &deg;C</h2>";

  html += "<h2>Humidity: ";
  html += String(humidity, 1);
  html += " %</h2>";

  if (temperature >= temperatureLimit) {
    html += "<h2>WARNING: High Temperature!</h2>";
  } else {
    html += "<h2>Status: Normal</h2>";
  }

  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  dht.begin();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting to Wi-Fi...");
  display.display();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Wi-Fi Connected");
  display.setCursor(0, 20);
  display.println(WiFi.localIP());
  display.display();

  // Start web server
  server.on("/", handleRoot);
  server.begin();

  Serial.println("Web server started.");
}

void loop() {
  updateSensorData();
  server.handleClient();

  delay(2000);
}