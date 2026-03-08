/* ============================================================
   SMART IOT DOOR ACCESS MONITORING SYSTEM
   ------------------------------------------------------------
   Device: ESP32
   Features:
      • OLED user interface
      • Buzzer + LED feedback
      • MQTT event publishing
      • Firebase cloud logging
      • NTP time synchronization
   ============================================================ */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include <WiFiUdp.h>
#include <NTPClient.h>

/* ============================================================
   HARDWARE DEFINITIONS
   ============================================================ */

#define BUZZER    13
#define GREEN_LED 26
#define BTN_GRANT 14
#define BTN_DENY  12

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* ============================================================
   NETWORK CONFIGURATION
   ============================================================ */

const char* ssid = "Wokwi-GUEST";
const char* password = "";

/* Firebase Realtime Database endpoint */
const char* firebase_url =
"https://smartdooriot-fad5e-default-rtdb.firebaseio.com/access_logs.json";

/* MQTT Broker */
const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic  = "iot/smartdoor/access";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

/* ============================================================
   NTP CONFIGURATION
   ============================================================ */

WiFiUDP ntpUDP;

/* NTP server with 1 minute refresh interval */
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

/* ============================================================
   WIFI CONNECTION
   ============================================================ */

void connectWiFi() {

  Serial.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" connected");
}

/* ============================================================
   MQTT CONNECTION HANDLING
   ============================================================ */

void reconnectMQTT() {

  while (!mqttClient.connected()) {

    Serial.print("Attempting MQTT connection...");

    String clientId = "SmartDoorNode01-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {

      Serial.println("connected");

    } else {

      Serial.print("failed rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");

      delay(5000);
    }
  }
}

/* ============================================================
   GET CURRENT TIMESTAMP (NTP)
   ============================================================ */

String getTimestamp() {

  timeClient.update();

  unsigned long epochTime = timeClient.getEpochTime();

  struct tm *ptm = gmtime((time_t *)&epochTime);

  char buffer[25];

  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
          ptm->tm_year + 1900,
          ptm->tm_mon + 1,
          ptm->tm_mday,
          ptm->tm_hour,
          ptm->tm_min,
          ptm->tm_sec);

  return String(buffer);
}

/* ============================================================
   OLED DISPLAY FUNCTIONS
   ============================================================ */

void showMessage(const char* line1,
                 const char* line2 = "",
                 bool invert = false) {

  display.clearDisplay();

  if (invert) {
    display.fillScreen(WHITE);
    display.setTextColor(BLACK);
  } else {
    display.setTextColor(WHITE);
  }

  display.setTextSize(2);
  display.setCursor(0, 8);
  display.println(line1);

  display.setTextSize(1);
  display.setCursor(0, 46);
  display.println(line2);

  display.display();
}

/* ============================================================
   BUZZER ALERT
   ============================================================ */

void beep(int freq, int duration) {

  tone(BUZZER, freq, duration);

  delay(duration + 50);

  noTone(BUZZER);
}

/* ============================================================
   MQTT EVENT PUBLISH
   ============================================================ */

void publishEvent(const char* status) {

  StaticJsonDocument<200> doc;

  doc["device"] = "SmartDoorNode01";
  doc["status"] = status;
  doc["timestamp"] = getTimestamp();

  char buffer[256];

  serializeJson(doc, buffer);

  mqttClient.publish(mqtt_topic, buffer);

  Serial.print("MQTT Sent: ");
  Serial.println(buffer);
}

/* ============================================================
   FIREBASE CLOUD LOGGING
   ============================================================ */

void logToFirebase(const char* status) {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;

  http.begin(firebase_url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;

  doc["device"] = "SmartDoorNode01";
  doc["status"] = status;
  doc["timestamp"] = getTimestamp();

  String payload;

  serializeJson(doc, payload);

  int httpResponseCode = http.POST(payload);

  Serial.print("Firebase Response: ");
  Serial.println(httpResponseCode);

  http.end();
}

/* ============================================================
   ACCESS EVENTS
   ============================================================ */

void grantAccess() {

  Serial.println("[ACCESS GRANTED]");

  publishEvent("granted");
  logToFirebase("granted");

  showMessage("ACCESS", "GRANTED ✓", true);

  digitalWrite(GREEN_LED, HIGH);

  beep(1000,150);
  delay(80);
  beep(1300,150);

  delay(2000);

  digitalWrite(GREEN_LED, LOW);
}

void denyAccess() {

  Serial.println("[ACCESS DENIED]");

  publishEvent("denied");
  logToFirebase("denied");

  showMessage("ACCESS", "DENIED X", false);

  for(int i=0;i<3;i++){
    beep(300,100);
    delay(80);
  }

  delay(2000);
}

/* ============================================================
   SYSTEM INITIALIZATION
   ============================================================ */

void setup() {

  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(BTN_GRANT, INPUT_PULLUP);
  pinMode(BTN_DENY, INPUT_PULLUP);

  digitalWrite(GREEN_LED, LOW);

  /* OLED initialization */
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED initialization failed");

    while (true);
  }

  showMessage("SmartDoor", "Booting...");

  /* Network initialization */
  connectWiFi();

  mqttClient.setServer(mqtt_server, 1883);

  /* NTP initialization */
  timeClient.begin();
  timeClient.update();

  showMessage("SmartDoor", "Ready");

  Serial.println("System Ready");
}

/* ============================================================
   MAIN LOOP
   ============================================================ */

void loop() {

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }

  mqttClient.loop();

  delay(10);

  /* Button: Grant Access */
  if (digitalRead(BTN_GRANT) == LOW) {

    delay(30);

    grantAccess();

    showMessage("SmartDoor", "Waiting...");
  }

  /* Button: Deny Access */
  if (digitalRead(BTN_DENY) == LOW) {

    delay(30);

    denyAccess();

    showMessage("SmartDoor", "Waiting...");
  }
}