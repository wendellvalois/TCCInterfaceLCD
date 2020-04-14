#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
// Sketch settings
const char LASER_SYMBOL = '^';

// include the library code:
// #include <LiquidCrystal.h>

// Invader string
String invaders;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Current laser position
int laserColumn;

// Timer variables
unsigned long timerStart;
unsigned long timerInterval;
unsigned long currentTime;

// Button debounce variables
int currentReading;
int newReading;

// Edge detection variable
int previousReading;

void setup() {
  // set up the LCD's number of columns and rows:
//   lcd.begin(16, 2);
  lcd.begin();

  // Initilize laser position
  laserColumn = 0;

  // Initialize invaders
  invaders = "** * ** * **  **";

  // Display invaders
  lcd.setCursor(0, 0);
  lcd.print(invaders);

  // Setup laser button
  pinMode(7, INPUT_PULLUP);

  // Initialize timer
  timerStart = millis();
  timerInterval = 250;

  // Initialize debouncing
  currentReading = digitalRead(7);
  
}

void loop() {

  currentTime = millis();
  if((currentTime-timerStart) > timerInterval) {

    // Reset timer
    timerStart = currentTime;
  
    // Erase the old frame
    lcd.setCursor(laserColumn, 1);
    lcd.print(' ');
    
    // Calculate new frame
    laserColumn = laserColumn + 1;
  
    // Limit the laser column
    if (laserColumn > 15) {
      laserColumn = 0;
    }
    
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(laserColumn, 1);
    lcd.print(LASER_SYMBOL);
  }

  // Save the current button reading
  // previousReading = currentReading;

  // Debounce the button
  // debounce(7, 4);

  // Check the button and zap invaders
  if (debounce(7, 4)) {
    lcd.setCursor(laserColumn, 0);
    lcd.print(' ');
  }
}

bool debounce(int pin, int interval) {
  int newValue;
  previousReading = currentReading;
  newValue = digitalRead(pin);
  if (newValue != currentReading) {
    delay(interval);
    newValue = digitalRead(pin);
    if (newValue != currentReading) {
      currentReading = newValue;
    }
  }
  // return currentReading;
   if ((previousReading == HIGH) && (currentReading == LOW)) {
     return true;
   }
   return false;
}