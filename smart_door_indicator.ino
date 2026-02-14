
/*
============================================================
Smart Door Indicator System
------------------------------------------------------------
Detects door open/close state using reed switch or button
Provides LED and Buzzer alerts
Logs events to Serial Monitor
Designed for Wokwi / Arduino Uno
============================================================
*/

const int door_sensor = 2;
const int red_led = 8;
const int green_led = 9;
const int buzzer = 10;

#define DOOR_OPEN HIGH
#define DOOR_CLOSED LOW

unsigned long last_debounce_time = 0;
unsigned long debounce_delay = 50;

int last_reading = HIGH;
int stable_state = HIGH;

void setup()
{
    Serial.begin(9600);

    pinMode(door_sensor, INPUT_PULLUP);
    pinMode(red_led, OUTPUT);
    pinMode(green_led, OUTPUT);
    pinMode(buzzer, OUTPUT);

    InitializeSystem();
}

void loop()
{
    HandleDoorState();
}

void InitializeSystem()
{
    digitalWrite(green_led, HIGH);
    digitalWrite(red_led, LOW);
    digitalWrite(buzzer, LOW);

    Serial.println("=================================");
    Serial.println(" Smart Door Indicator Initialized");
    Serial.println("=================================");
}

void HandleDoorState()
{
    int reading = digitalRead(door_sensor);

    if (reading != last_reading)
    {
        last_debounce_time = millis();
    }

    if ((millis() - last_debounce_time) > debounce_delay)
    {
        if (reading != stable_state)
        {
            stable_state = reading;

            if (stable_state == DOOR_CLOSED)
            {
                DoorClosed();
            }
            else
            {
                DoorOpened();
            }
        }
    }

    last_reading = reading;
}

void DoorClosed()
{
    digitalWrite(green_led, HIGH);
    digitalWrite(red_led, LOW);
    digitalWrite(buzzer, LOW);

    Serial.println("[INFO] Door Closed");
}

void DoorOpened()
{
    digitalWrite(green_led, LOW);
    digitalWrite(red_led, HIGH);

    Serial.println("[ALERT] Door Opened");

    TriggerAlert();
}

void TriggerAlert()
{
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(buzzer, HIGH);
        delay(200);
        digitalWrite(buzzer, LOW);
        delay(200);
    }
}
