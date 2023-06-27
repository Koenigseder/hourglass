#include <string.h>
#include "LedControl.h"

// Digital pins
int buttonPin = 7;
int buzzerPin = 6;
int ledMatrixCS = 10;
int ledMatrixCLK = 11;
int ledMatrixDIN = 12;

// Analog pins
uint8_t rotationSensorY = A0;

LedControl lc = LedControl(ledMatrixDIN, ledMatrixCLK, ledMatrixCS, 2); // Initialize the matrix controller

/*
Modes:
  0 → Endless (default)
  1 → 1 min
  2 → 2 min
  3 → 5 min
  4 → 10 min
  5 → 15 min
  6 → 20 min
  7 → 30 min
  8 → 45 min
  9 → 60 min
*/
int currentMode = 0;

bool buttonAlreadyPressed = false; // Check if the button was already pressed
bool alreadyRun = false; // Check if the whole animation cycle has finished

unsigned long animationDelayTime = 100; // Delay between animations
unsigned long delayBetweenCycles = 1; // Delay between the next corn fall

unsigned int ledIntensity = 4; // Intensity of the LEDs

int segmentOnTop = 0; // Index of upper matrix
int segmentOnBottom = 1; // Index of lower matrix


int numRowsAndColumns = 6; // Zero-indexed | How much rows and colums should be filled in the lower matrix
int fallLengthOfCorn = 7; // Zero-indexed | How far the corn should fall

// First start points
int startRowCord = 5;
int startColCord = 3;

// Cache next corns to animate in the upper matrix
int nextRowCord = startRowCord;
int nextColCord = startColCord;

int currentIteration = 1; // Used to determine where to start with the animation of the upper matrix
int currentFactor = -1; // Needed for the calculation of the animation

// Variables for the upper matrix, used to determine when to start with the next line
int countCornPerRow[] = {6, 7, 8, 7, 6, 5, 4, 3, 2, 1};
int indexOfRow = 0;
int loopIteration = 1;

uint16_t y_axis = 0; // Rotation of the sensor
int rotationAngle = 33;

byte upperMatrixSetup[] = {B11111111, B11111111, B11111111, B01111111, B00111111, B00011111, B00001111, B00000111}; // Pre-defined pattern

// Pre-defined patters for displaying the times
byte infinite[] = {B00000000, B00110000, B01001000, B01001100, B00110010, B00010010, B00001100, B00000000};
byte zero[] = {B00000000, B00001100, B00010010, B00100010, B01000100, B01001000, B00110000, B00000000};
byte one[] = {B00000000, B00000000, B00000100, B00001000, B00010000, B00111000, B00000000, B00000000};
byte two[] = {B00000000, B00001000, B00000100, B00111110, B01000000, B01000000, B00110000, B00000000};
byte three[] = {B00000000, B00001100, B00010010, B00110010, B01001000, B01000000, B00110000, B00000000};
byte four[] = {B00000000, B00001000, B00010000, B00101000, B01000100, B00001000, B00010000, B00000000};
byte five[] = {B00000000, B00001100, B00010010, B00010010, B01001000, B00101000, B00010000, B00000000};
byte six[] = {B00000000, B00001100, B00010010, B00010010, B00001100, B01001000, B00110000, B00000000};

byte oneFlipped[] = {B00000000, B00000000, B00011100, B00001000, B00010000, B00100000, B00000000, B00000000};
byte twoFlipped[] = {B00000000, B00001100, B00000010, B00000010, B01111100, B00100000, B00010000, B00000000};
byte fiveFlipped[] = {B00000000, B00001000, B00010100, B00010010, B01001000, B01001000, B00110000, B00000000};

// Placeholder array for the different combinations of numbers to display times
byte upperMatrixNumber[8];
byte lowerMatrixNumber[8];

// Define some notes
#define NOTE_C4  262
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440

#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_G5  784

void setup() {
  pinMode(buttonPin, INPUT);

  lc.shutdown(segmentOnTop, false);
  lc.shutdown(segmentOnBottom, false);

  lc.setIntensity(segmentOnTop, ledIntensity);
  lc.setIntensity(segmentOnBottom, ledIntensity);

  // Check the rotation on startup
  y_axis = analogRead(rotationSensorY) / 10;
  if (y_axis < 35) {
    segmentOnTop = 0;
    segmentOnBottom = 1;
  } else {
    segmentOnTop = 1;
    segmentOnBottom = 0;
  }

  animateFadeInOut();
}

void loop() {
  animateSegments(); // Play animations

  delay(500);

  y_axis = analogRead(rotationSensorY) / 10; // Read the y-rotation

  // Check if clock was rotated
  if (y_axis < rotationAngle && segmentOnTop == 1) {
    segmentOnTop = 0;
    segmentOnBottom = 1;
    resetGlobals();
    animateFadeInOut();
  } else if (y_axis >= rotationAngle && segmentOnTop == 0) {
    segmentOnTop = 1;
    segmentOnBottom = 0;
    resetGlobals();
    animateFadeInOut();
  }
}

// Change the current mode
void changeCurrentMode(int buttonState) {
  long currentMillis = millis();
  long endMillis = currentMillis + 3000;

  while (currentMillis < endMillis) {
    currentMillis = millis();

    buttonState = digitalRead(buttonPin);

    if (buttonState == HIGH && !buttonAlreadyPressed) {

      buttonAlreadyPressed = true;
        
      if (currentMode == 9) {
        currentMode = 0;
      } else {
        currentMode++;
      }

      if (currentMode == 0) {
        delayBetweenCycles = 1;
      } else if (currentMode == 1) {
        delayBetweenCycles = 612;
      } else if (currentMode == 2) {
        delayBetweenCycles = 1836;
      } else if (currentMode == 3) {
        delayBetweenCycles = 5508;
      } else if (currentMode == 4) {
        delayBetweenCycles = 11628;
      } else if (currentMode == 5) {
        delayBetweenCycles = 17748;
      } else if (currentMode == 6) {
        delayBetweenCycles = 23868;
      } else if (currentMode == 7) {
        delayBetweenCycles = 36108;
      } else if (currentMode == 8) {
        delayBetweenCycles = 54468;
      } else if (currentMode == 9) {
        delayBetweenCycles = 72828;
      }

      showTimes(); // Display the current time
      endMillis = currentMillis + 3000;
    }

    if (buttonState == LOW && buttonAlreadyPressed) {
      buttonAlreadyPressed = false;
    }
  }

  resetGlobals();
  animateFadeInOut();
}

// Reset global variables to start a fresh animation cycle
void resetGlobals() {
  alreadyRun = false;

  fallLengthOfCorn = 7;

  startRowCord = 5;
  startColCord = 3;

  nextRowCord = startRowCord;
  nextColCord = startColCord;

  currentIteration = 1;
  currentFactor = -1;

  indexOfRow = 0;
  loopIteration = 1;
}

// Do a cool fade in and out
void animateFadeInOut() {
  // Fade out
  for (int i = 0; i < 8; i++) {
    lc.setRow(segmentOnTop, i, 0xFF);
    lc.setRow(segmentOnBottom, 7 - i, 0xFF);
    delay(25);
  }

  // Fade in
  for (int i = 0; i < 8; i++) {
    lc.setRow(segmentOnTop, i, upperMatrixSetup[i]);
    lc.setRow(segmentOnBottom, 7 - i, 0x00);
    delay(25);
  }
}

// Display the time according to the current mode
void showTimes() {
  if (currentMode == 0) {
    memcpy(upperMatrixNumber, infinite, 8);
    memcpy(lowerMatrixNumber, infinite, 8);
  } else if (currentMode == 1) {
    memcpy(upperMatrixNumber, zero, 8);
    memcpy(lowerMatrixNumber, oneFlipped, 8);
  } else if (currentMode == 2) {
    memcpy(upperMatrixNumber, zero, 8);
    memcpy(lowerMatrixNumber, twoFlipped, 8);
  } else if (currentMode == 3) {
    memcpy(upperMatrixNumber, zero, 8);
    memcpy(lowerMatrixNumber, fiveFlipped, 8);
  } else if (currentMode == 4) {
    memcpy(upperMatrixNumber, one, 8);
    memcpy(lowerMatrixNumber, zero, 8);
  } else if (currentMode == 5) {
    memcpy(upperMatrixNumber, one, 8);
    memcpy(lowerMatrixNumber, fiveFlipped, 8);
  } else if (currentMode == 6) {
    memcpy(upperMatrixNumber, two, 8);
    memcpy(lowerMatrixNumber, zero, 8);
  } else if (currentMode == 7) {
    memcpy(upperMatrixNumber, three, 8);
    memcpy(lowerMatrixNumber, zero, 8);
  } else if (currentMode == 8) {
    memcpy(upperMatrixNumber, four, 8);
    memcpy(lowerMatrixNumber, fiveFlipped, 8);
  } else if (currentMode == 9) {
    memcpy(upperMatrixNumber, six, 8);
    memcpy(lowerMatrixNumber, zero, 8);
  }

  for (int i = 0; i < 8; i++) {
    lc.setRow(segmentOnTop, i, 0xFF);
    lc.setRow(segmentOnBottom, 7 - i, 0xFF);
    delay(25);
  }

  for (int i = 0; i < 8; i++) {
    lc.setRow(segmentOnTop, i, upperMatrixNumber[i]);
    lc.setRow(segmentOnBottom, 7 - i, lowerMatrixNumber[7 - i]);
    delay(25);
  }
}

// Animate the two segments
void animateSegments() {
  // Check if the animation has finished
  if (alreadyRun) {
    if (currentMode == 0) {
      resetGlobals();
      animateFadeInOut();
    }

    return;
  }

  for (int i = 0; i <= numRowsAndColumns; i++) {
    // Cache how many corns are on each side
    int numCornsRightSide = 0;
    int numCornsLeftSide = 0;

    for (int j = 0; j <= i * 2; j++) {

      long currentMillis = millis();
      long endMillis = currentMillis + delayBetweenCycles;

      while (currentMillis < endMillis) {
        currentMillis = millis();

        int buttonState = digitalRead(buttonPin);
        if (buttonState == HIGH) {
          changeCurrentMode(buttonState);
          return;
        }      

        y_axis = analogRead(rotationSensorY) / 10; // Read the y-rotation

        // Check if the clock was rotated
        if (y_axis < 35 && segmentOnTop == 1) {
          segmentOnTop = 0;
          segmentOnBottom = 1;

          animateFadeInOut();
          resetGlobals();

          return;
        } else if (y_axis >= 35 && segmentOnTop == 0) {
          segmentOnTop = 1;
          segmentOnBottom = 0;

          animateFadeInOut();
          resetGlobals();

          return;
        }
      }

      // Remove the corn from the upper matrix accordingly
      if (loopIteration > countCornPerRow[indexOfRow]) {
        currentIteration++;
        indexOfRow++;
        loopIteration = 1;

        if (currentIteration % 2 == 0) {
          startRowCord--;
        } else {
          startColCord++;
        }

        nextRowCord = startRowCord;
        nextColCord = startColCord;
        currentFactor = -1;
      }

      animateStraightSandcornFall(); // Animate the straight corn fall

      loopIteration++;

      long sandCornToFall = 0; // Determines the side where the corn is going to fall down; 0 = right; 1 = left

      // Check if there is still place for corns on each side; if there is, choose a random side
      if (numCornsRightSide == i) {
        sandCornToFall = 1;
      } else if (numCornsLeftSide == i) {
        sandCornToFall = 0;
      } else {
        sandCornToFall = random(0, 2);
      }

      // Animate corn fall on right side
      if (sandCornToFall == 0) {
        for (int k = i; k > numCornsRightSide; k--) {
          lc.setLed(segmentOnBottom, 7 - (k - 1), i, true);
          delay(animationDelayTime);
          if ((k - 1) != numCornsRightSide) {
            lc.setLed(segmentOnBottom, 7 - (k - 1), i, false);
          }
        }
        numCornsRightSide++;
      }

      // Animate corn fall on left side
      if (sandCornToFall == 1) {
        for (int k = i; k > numCornsLeftSide; k--) {
          lc.setLed(segmentOnBottom, 7 - i, (k - 1), true);
          delay(animationDelayTime);
          if ((k - 1) != numCornsLeftSide) {
            lc.setLed(segmentOnBottom, 7 - i, (k - 1), false);
          }
        }
        numCornsLeftSide++;
      }
    }

    lc.setLed(segmentOnBottom, 7 - i, i, true);
    fallLengthOfCorn--;
  }

  alreadyRun = true;

  // Only play the finishing melody when not in endless mode
  if (currentMode != 0) {
    playFinishedMelody();
  }
}

// Animate the straight sandcorn fall
void animateStraightSandcornFall() {

  // Remove corn from the upper matrix
  lc.setLed(segmentOnTop, nextRowCord, nextColCord, false);

  nextRowCord += loopIteration * currentFactor;
  nextColCord += loopIteration * currentFactor;

  currentFactor *= -1;

  // Let the corn fall down
  for (int i = 0; i <= fallLengthOfCorn; i++) {
    lc.setLed(segmentOnBottom, i, 7 - i, true);
    delay(animationDelayTime);
    lc.setLed(segmentOnBottom, i, 7 - i, false);
  }
}

// Play a finishing sound
void playFinishedMelody() {
  int tempo = 200;

  // Super Mario World Game Over melody
  int melody[] = {
    NOTE_A4, 2,
    NOTE_F4, 4,
    NOTE_C4, 4,
    NOTE_A4, 2,
    NOTE_E4, 2,
    NOTE_G4, 2,

    NOTE_C5, 4,
    NOTE_D5, 4,
    NOTE_G5, 1
  };

  int notes = sizeof(melody) / sizeof(melody[0]) / 2;

  // This calculates the duration of a whole note in ms
  int wholenote = (60000 * 4) / tempo;

  int divider = 0, noteDuration = 0;

  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
    // Calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // Regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // Dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // Increases the duration in half for dotted notes
    }

    // We only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzerPin, melody[thisNote], noteDuration*0.9);

    // Wait for the specified duration before playing the next note
    delay(noteDuration);
    
    // Stop the waveform generation before the next note
    noTone(buzzerPin);
  }

  pinMode(buzzerPin, LOW); // Needed to avoid annoying buzzing
}
