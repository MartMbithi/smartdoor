#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUZZER    13
#define GREEN_LED 26
#define BTN_GRANT 14   // Press to simulate authorized access
#define BTN_DENY  12   // Press to simulate denied access

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void beep(int freq, int duration) {
  tone(BUZZER, freq, duration);
  delay(duration + 50);
  noTone(BUZZER);
}

void showMessage(const char* line1, const char* line2 = "", bool invert = false) {
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

void grantAccess() {
  Serial.println("[ACCESS GRANTED]");
  showMessage("ACCESS", "GRANTED ✓", true);
  digitalWrite(GREEN_LED, HIGH);
  beep(1000, 150); delay(80); beep(1300, 150);
  delay(2000);
  digitalWrite(GREEN_LED, LOW);
}

void denyAccess() {
  Serial.println("[ACCESS DENIED]");
  showMessage("ACCESS", "DENIED  X", false);
  for (int i = 0; i < 3; i++) { beep(300, 100); delay(80); }
  delay(2000);
}

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  pinMode(BTN_GRANT, INPUT_PULLUP);
  pinMode(BTN_DENY,  INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed!"); while (true);
  }

  showMessage("SmartDoor", "Waiting...");
  Serial.println("Ready. Press btn on GPIO14=Grant, GPIO12=Deny");
}

void loop() {
  if (digitalRead(BTN_GRANT) == LOW) {
    delay(30); // debounce
    grantAccess();
    showMessage("SmartDoor", "Waiting...");
  }
  if (digitalRead(BTN_DENY) == LOW) {
    delay(30);
    denyAccess();
    showMessage("SmartDoor", "Waiting...");
  }
}