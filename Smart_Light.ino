#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
WiFiServer server(80);                  
const int ledPin = D5; 
// Replace with your network credentials
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "" // Replace with your bot token
#define CHAT_ID "" // Replace with your chat ID

// Pin where the LDR is connected
#define LDR_PIN A0

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(ledPin, OUTPUT);
  // Connect to Wi-Fi
  Serial.print("Connecting to Wifi SSID ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());

  // Synchronize time with NTP server
  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org");
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println("Time retrieved: " + String(now));

  // Notify Telegram Bot of startup
  bot.sendMessage(CHAT_ID, "Bot started up", "");
  server.begin();
}

void loop() {
  // Read LDR value
 WiFiClient client = server.available(); 
  if (client) {
    Serial.println("Client connected!");
    String request = client.readStringUntil('\r');
    Serial.print("Request: ");
    Serial.println(request);
    client.flush(); 
    if (request.indexOf("LED=ON") != -1) {
      digitalWrite(ledPin, HIGH);       
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nLED is ON");
    }
    if (request.indexOf("LED=OFF") != -1) {
      digitalWrite(ledPin, LOW);      
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nLED is OFF");
    }
    client.stop();
    Serial.println("Client disconnected.");
  int ldrValue = analogRead(LDR_PIN);
  Serial.println("LDR Value: " + String(ldrValue));

  // Check if LDR value exceeds the threshold
  if (ldrValue > 20) {
    Serial.println("LDR value exceeds threshold. Sending Telegram message...");
    bot.sendMessage(CHAT_ID, "Alert! LDR value is above 20: " + String(ldrValue), "");
    delay(1000); // Avoid spamming messages (10 seconds delay)
  }

  delay(1000); // Main loop delay
}
}