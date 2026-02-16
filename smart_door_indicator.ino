
/* Pin Configurations */
const int door_sensor = 2;
const int red_led = 8;
const int green_led = 9;
const int buzzer = 10;


/* Logical State Definitions */
#define DOOR_OPEN HIGH
#define DOOR_CLOSED LOW


/* Debounce Mechanisms */
unsigned long last_debounce_time = 0;
unsigned long debounce_delay = 50;

int last_reading = HIGH;
int stable_state = HIGH;


/* Main Method */
void setup()
{
    Serial.begin(9600);

    pinMode(door_sensor, INPUT_PULLUP);
    pinMode(red_led, OUTPUT);
    pinMode(green_led, OUTPUT);
    pinMode(buzzer, OUTPUT);

    digitalWrite(green_led, HIGH);
    digitalWrite(red_led, LOW);
    digitalWrite(buzzer, LOW);
}

/* Main Execution Loop Which has the following Flow:
    1. Read sensor input.
    2. Detect change from previous reading.
    3. Apply debounce timing.
    4. Update stable state if confirmed.
    5. Trigger appropriate response function.
 */
void loop()
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
                digitalWrite(green_led, HIGH);
                digitalWrite(red_led, LOW);
                digitalWrite(buzzer, LOW);
                Serial.println("[INFO] Door Closed");
            }
            else
            {
                digitalWrite(green_led, LOW);
                digitalWrite(red_led, HIGH);
                Serial.println("[ALERT] Door Opened");

                for (int i = 0; i < 3; i++)
                {
                    digitalWrite(buzzer, HIGH);
                    delay(200);
                    digitalWrite(buzzer, LOW);
                    delay(200);
                }
            }
        }
    }

    last_reading = reading;
}
