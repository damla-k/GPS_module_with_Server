#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Replace with your network credentials
const char* ssid = "damla";
const char* password = "123-67-34-32";

//const char* ssid = "SUPERONLINE_WiFi_76AB";
//const char* password = "KC4FUAAKRKHY";

// Python server details
const char* serverUrl = "http://192.168.43.86:5000/update"; // Correct endpoint

// GPS Module
HardwareSerial SerialGPS(1); // Use UART 1 for GPS
TinyGPSPlus gps;

void setup() {
  // Start Serial Monitor
  Serial.begin(115200);

  // Start GPS Serial
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Reconnect if Wi-Fi is disconnected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.begin(ssid, password);
    delay(5000); // Wait for reconnection
  }

  // Process GPS data
  while (SerialGPS.available() > 0) {
    char c = SerialGPS.read();
    Serial.write(c);  // Print raw GPS data to Serial Monitor
    gps.encode(c);
  }

  if (gps.location.isValid()) {
    Serial.println("Valid GPS data found.");
    // Prepare GPS data as JSON
    String gpsJson = "{\"latitude\":" + String(gps.location.lat(), 6) +
                     ",\"longitude\":" + String(gps.location.lng(), 6) +
                     ",\"altitude\":" + String(gps.altitude.meters()) +
                     ",\"speed\":" + String(gps.speed.kmph()) +
                     ",\"satellites\":" + String(gps.satellites.value()) + "}";

    // Send GPS data to the Python server
    sendDataToServer(gpsJson);
  } else {
    Serial.println("\nWaiting for valid GPS data...");
  }

  delay(1000); // Send data every 5 seconds
}

void sendDataToServer(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected. Attempting to send data...");

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    Serial.println("Sending POST request...");
    int httpResponseCode = http.POST(data);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending data. HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Error details: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected. Cannot send data.");
  }
}
