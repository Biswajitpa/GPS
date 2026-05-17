#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ThingSpeak API
String apiKey = "YOUR_THINGSPEAK_API_KEY";
const char* server = "http://api.thingspeak.com/update";

// GPS setup
TinyGPSPlus gps;
HardwareSerial GPSserial(2); // UART2 for ESP32 (RX=16, TX=17)

void setup() {
  Serial.begin(115200);

  // GPS Serial
  GPSserial.begin(9600, SERIAL_8N1, 16, 17);

  // WiFi connect
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
}

void loop() {
  while (GPSserial.available() > 0) {
    gps.encode(GPSserial.read());
  }

  if (gps.location.isUpdated()) {

    float lat = gps.location.lat();
    float lng = gps.location.lng();
    float speed = gps.speed.kmph();

    Serial.println("------ GPS DATA ------");
    Serial.print("Latitude: ");
    Serial.println(lat, 6);
    Serial.print("Longitude: ");
    Serial.println(lng, 6);
    Serial.print("Speed km/h: ");
    Serial.println(speed);

    // Send to ThingSpeak
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      String url = String(server) +
                   "?api_key=" + apiKey +
                   "&field1=" + String(lat, 6) +
                   "&field2=" + String(lng, 6) +
                   "&field3=" + String(speed);

      http.begin(url);

      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.print("Data sent to cloud. Code: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Error sending data: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    }
  }

  delay(15000); // ThingSpeak limit (15 sec)
}