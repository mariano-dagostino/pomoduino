#include "LedControl.h"

// You can get LedControl.h library from https://github.com/wayoda/LedControl

/* Create a new LedControl variable.
 * We use pins 12,11 and 10 on the Arduino for the SPI interface
 * Pin 12 is connected to the DATA IN-pin of the first MAX7221
 * Pin 11 is connected to the CS of the first MAX7221
 * Pin 10 is connected to the CLK-pin of the first MAX7221
 * There will only be a single MAX7221 attached to the arduino
 */
const int DIN = 12;
const int CS  = 11;
const int CLK = 10;
LedControl lc = LedControl(DIN, CLK, CS, 1);

const int MODE_NONE = 0;
const int MODE_WORK = 1;
const int MODE_REST = 2;

const int WORK_DURATION = 25;
const int REST_DURATION = 5;

// The number of microseconds a minute takes to complete (60000).
// If you want to try this faster, change it to something like 200.
const unsigned long MINUTE_DURATION = 60000;

const int BELL_OUTPUT = 2;
const int WORK_BUTTON = 3;
const int REST_BUTTON = 4;

int minutes         = WORK_DURATION;
int lastUpdate      = 0;
int currentMode     = MODE_NONE;

unsigned long startedAt = 0;
unsigned long minutesElapsed = 0;

int workCompleted = 0; // Number of work cycles completted.
int restCompleted = 0; // Number of rest cycles completted.

void clearMatrix() {
    lc.shutdown(0, false);
    lc.setIntensity(0, 1);
    lc.clearDisplay(0);
    refreshLedCounters();
}

void setup() {
    clearMatrix();
    currentMode = MODE_NONE;
    pinMode(WORK_BUTTON, INPUT);
    pinMode(REST_BUTTON,  INPUT);
    pinMode(BELL_OUTPUT, OUTPUT);
}

void work() {
    currentMode = MODE_WORK;
    startedAt = millis();
    minutes = WORK_DURATION;
    lastUpdate = 0;
    clearMatrix();
    for (int x = 0; x < 5; x++) {
        lc.setColumn(0, x, numberToLed(5));
    }
    refreshLedCounters();
}

void rest() {
    currentMode = MODE_REST;
    startedAt = millis();
    minutes = REST_DURATION;
    lastUpdate = 0;
    clearMatrix();
    lc.setColumn(0, 0, numberToLed(5));
    refreshLedCounters();
}

void updatePomodoro() {
    if (currentMode == MODE_NONE) {
      return;
    }

    minutesElapsed = (millis() - startedAt) / MINUTE_DURATION;

    if (minutesElapsed <= minutes && minutesElapsed > lastUpdate) {
        // A new minute just pass
        lastUpdate = lastUpdate + 1;
        renderMatrix(minutes - lastUpdate);
    }

    if (lastUpdate == minutes) {
        currentModeCompleted();
    }
}

void renderMatrix(int number) {
    int column = number % 5;
    int row = number / 5;
    lc.setColumn(0, row, numberToLed(column));
    refreshLedCounters();
}

void refreshLedCounters() {
    // To be able to render more than 8 groups of work
    // the following convention was used.
    // |*| This is a led on. | | this is a led off.

    /**

      |*| < This represents one round completed.
      |*| < This represents one round completed.
      | |
      | |
      | |
      | |
      | |
      |*| < This represents 4 completed rounds.

      So the previous representation means you completed 6
      rounds of work or rest depending of the column.
    **/

    lc.setRow(0, 0, numberToLed(workCompleted % 4));
    lc.setRow(0, 1, numberToLed(restCompleted % 4));

    for (int y = 0; y < 4; y++) {
        if (workCompleted / 4 >= y + 1) {
            lc.setLed(0, 0, y, true);
        }
        if (restCompleted / 4 >= y + 1) {
            lc.setLed(0, 1, y, true);
        }
    }
}

/**
 * Converts a number to its led representation
 * 8 = B11111111
 * 7 = B01111111
 * 3 = B00000111
 */
int numberToLed(int number) {
    return (1 << number) - 1;
}

void bellBeep(int beeps = 2) {
    for (int x = 0; x < beeps; x++) {
        digitalWrite(BELL_OUTPUT, HIGH);
        delay(100);
        digitalWrite(BELL_OUTPUT, LOW);
        delay(300);
    }
}

void currentModeCompleted() {
    if (currentMode == MODE_WORK) {
        workCompleted++;
    }
    if (currentMode == MODE_REST) {
        restCompleted++;
    }

    currentMode = MODE_NONE;
    bellBeep();
    refreshLedCounters();
}

void loop() {
    if (digitalRead(WORK_BUTTON) == HIGH) {
        work();
    }

    if (digitalRead(REST_BUTTON) == HIGH) {
        rest();
    }

    updatePomodoro();
}
