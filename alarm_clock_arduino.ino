#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5 , 6 , 7, 3, POSITIVE);
unsigned long programTime = millis();
unsigned int seconds = 0;
unsigned int minutes = 59;
unsigned int hours = 20;
unsigned int days = 24;
unsigned int month = 9;
unsigned int year = 2020;
unsigned long int programSeconds = 1;
unsigned long int secondInMilliseconds = 1000;

unsigned int totalDelay = 0;
const int delayAmmountBTN = 5;

unsigned int cursorRow = 0;
unsigned int cursorCol = 0;

unsigned int alarmMinutes = 0;
unsigned int alarmHours = 0;
bool alarmOn = false;
bool alarmIsUp = false;
bool alarmPrinted = false;

unsigned int stopwatchSeconds = 0;
unsigned int stopwatchMinutes = 0;
bool settingStopwatch = true;
bool settingMinutes = true;
bool stopwatchRunning = false;

unsigned int timerSeconds = 0;
unsigned int timerMinutes = 0;
bool timerRunning = false;


int settingClock = -1;
int setting_h = 2;
int setting_m = 1;
int setting_s = 0;
int setting_Y = 5;
int setting_M = 4;
int setting_D = 3;
int lastSecondSetClock = -1;




const int buttonPin1 = 2;
const int buttonPin2 = 3;
const int buttonPin3 = 4;
const int buttonPin4 = 5;// the number of the pushbutton pin

int menuButton = 0;         // variable for reading the pushbutton status
int rightButton = 0;         // variable for reading the pushbutton status
int leftButton = 0;         // variable for reading the pushbutton status
int resetButton = 0;         // variable for reading the pushbutton status

const int ledPin =  13;      // the number of the LED pin

unsigned int state = 0;
const unsigned int clockState = 0;
const unsigned int menuState = 4;
const unsigned int timerMenu = 1;
const unsigned int alarmMenu = 2;
const unsigned int stopwatchMenu = 3;
const unsigned int idleState = 8;
unsigned int previousMenuState = idleState;
unsigned int currentMenuState = 0;

unsigned int lastSecond = 0;
unsigned int lastMinute = 61;

bool menuButtonPressed = false;
bool rightButtonPressed = false;
bool leftButtonPressed = false;
bool resetButtonPressed = false;

bool menuPrinted = false;

void setup() {
  Serial.begin(9600);

  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
  pinMode(buttonPin4, INPUT);

  lcd.begin(16, 2);
  lcd.clear();
  printCenter(">Time Manager<", 0);
  printCenter("Mladen Tasevski", 1);
  delay(2000);
  lcd.clear();
  printCenter("DATE AND TIME", 0);
  delay(1000);
  lcd.clear();

}

void loop() {

  if ((millis() / secondInMilliseconds) > programSeconds) {
    programSeconds++;
    if (settingClock < 0)
    {
      setTime();
      if (alarmIsUp)
      {
        notifyLight();
      }
    }
  }

  if (minutes != lastMinute) {
    lastMinute = minutes;
    if (alarmOn && alarmHours == hours && alarmMinutes == minutes) {
      alarmIsUp = true;
    }
  }

  if (state == clockState)
    printTime();

  else if (state == menuState && currentMenuState != previousMenuState) {
    printMenu();
    menuPrinted = true;
  }

  else if (state == stopwatchMenu) {
    printStopwatch();
  }

  else if (state == timerMenu) {
    printTimer();
  }

  else if (state == alarmMenu) {
    if (!alarmPrinted)
    {
      printAlarmTime();
      printAlarmMenu();
      alarmPrinted = true;
    }
  }




  checkButtons();

}

void checkButtons() {
  // read the state of the pushbutton value:
  menuButton = digitalRead(buttonPin1);
  rightButton = digitalRead(buttonPin2);
  leftButton = digitalRead(buttonPin3);
  resetButton = digitalRead(buttonPin4);

  // check if the pushbuttons are pressed. If it is, the buttonState is HIGH:
  if (resetButton == HIGH && menuButton == HIGH) {
    // turn LED on:
    turnOffAlarm();
    delayForButton();

  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }

  // Go to menu
  if (isMenuBTNClicked()) {
    state = menuState;
    currentMenuState = 0;
    previousMenuState = 8;
    resetStopwatch();
    printMenu();
  }


  //menu state
  if (state == menuState) {
    if (isResetBTNClicked()) {
      state = currentMenuState;
      lcd.clear();
      printTitle();
      menuPrinted = false;
      alarmPrinted = false;
    }

    // move menu cursor right, select cursor state
    if (isRightBTNClicked()) {
      rightButtonPressed = true;


      currentMenuState++;
      currentMenuState = currentMenuState % 4;
    }

    // move menu cursor left, select cursor state
    if (isLeftBTNClicked()) {
      currentMenuState += 3;
      currentMenuState = currentMenuState % 4;
    }
  }

  //stopwatch menu
  if (state == stopwatchMenu) {
    if (isResetBTNClicked()) {
      if (settingStopwatch) {
        if (settingMinutes) {
          stopwatchMinutes++;
        }
        else {
          stopwatchSeconds++;
          if (stopwatchSeconds == 60)
            stopwatchSeconds = 0;
        }
      }
      else {
        stopwatchRunning = !stopwatchRunning;
      }
      printStopwatchTime();
      printStopwatchMenu();
    }
    if (isLeftBTNClicked()) {
      settingMinutes = !settingMinutes;
      settingStopwatch = true;
      printStopwatchMenu();
    }
    if (isRightBTNClicked()) {
      settingStopwatch = !settingStopwatch;
      printStopwatchMenu();
    }
  }

  //timer menu
  if (state == timerMenu) {
    if (isResetBTNClicked()) {
      resetTimer();
      printTimerTime();
    }
    if (isRightBTNClicked()) {
      timerRunning = !timerRunning;
      printTimerTime();
      printTimerMenu();
    }
  }

  //alarm menu
  if (state == alarmMenu) {
    if (isRightBTNClicked()) {
      alarmMinutes += 5;
      if (alarmMinutes == 60) alarmMinutes = 0;
      printAlarmTime();
    }
    if (isLeftBTNClicked()) {
      alarmHours += 1;
      if (alarmHours == 24) alarmHours = 0;
      printAlarmTime();
    }
    if (isResetBTNClicked()) {
      alarmOn = !alarmOn;
      printAlarmMenu();
    }
  }

  //settingTheClock
  if (state == clockState) {
    if (isResetBTNClicked()) {
      settingClock++;
      if (settingClock == 6) settingClock = -1;
    }
    // Clock is beeing set
    if (settingClock >= 0)
    {

      if (settingClock == setting_h) {
        blinkSetClock(5, 0);
        if (isLeftBTNClicked())
        {
          hours--;
          if (hours < 0) hours = 23;
        }
        if (isRightBTNClicked())
        {
          hours++;
          hours = hours % 24;
        }

      }
      else if (settingClock == setting_m) {
        blinkSetClock(9, 0);
        if (isLeftBTNClicked())
        {
          minutes--;
          if (minutes < 0) minutes = 59;
        }
        if (isRightBTNClicked())
        {
          minutes++;
          minutes = minutes % 60;
        }
      }
      else if (settingClock == setting_s) {
        blinkSetClock(13, 0);
        if (isLeftBTNClicked())
        {
          seconds--;
          if (seconds < 0) seconds = 59;
        }
        if (isRightBTNClicked())
        {
          seconds++;
          seconds = seconds % 60;
        }
      }
      else if (settingClock == setting_Y) {
        blinkSetClock(5, 1);
        if (isLeftBTNClicked())
        {
          year--;
          if (year < 20) year = 20;
        }
        if (isRightBTNClicked())
        {
          year++;
          year = year % 100;
          if (year < 20) year = 20;
        }
      }
      else if (settingClock == setting_M) {
        blinkSetClock(9, 1);
        if (isLeftBTNClicked())
        {
          month--;
          if (month <= 0) month = 12;
        }
        if (isRightBTNClicked())
        {
          month++;
          if (month > 12) month = 0;
        }
      }
      else if (settingClock == setting_D) {
        blinkSetClock(13, 1);
        bool isUp = false;
        if (isLeftBTNClicked())
        {
          days--;
          if (days <= 0) days = 32;
        }
        if (isRightBTNClicked())
        {
          days++;
          isUp = true;
          if (days > 12) days = 0;
        }
        if (days == 30 && month == 2 && (year % 100 == 0 && year % 400 != 0))
        {
            days = 29;
        }
        else if (days == 29 && month == 2 && year % 4 == 0)
        {
            days = 28;
        }
        else if (days == 31 && (month == 4 || month == 6 || month == 9 || month == 11))
        {
            days = 30;
        }
        else if (days == 32)
        {
            days = 29;
        }
      }

    }
  }
}


}


void printMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  previousMenuState = currentMenuState;
  String selectClock = "Clock";
  String selectAlarm = "Alarm";
  String selectTimer = "Stopwatch";
  String selectStopwatch = "Timer";
  if (currentMenuState == clockState) {
    selectClock = ">Clock";
  }
  else if (currentMenuState == timerMenu) {
    selectTimer = ">Stopwatch";
  }
  else if (currentMenuState == alarmMenu) {
    selectAlarm = ">Alarm";
  }
  else if (currentMenuState == stopwatchMenu) {
    selectStopwatch = ">Timer";
  }


  lcd.print(selectClock);
  lcd.print(" ");
  lcd.print(selectTimer);



  lcd.setCursor(0, 1);

  lcd.print(selectAlarm);
  lcd.print(" ");
  lcd.print(selectStopwatch);
}



void printTime() {
  lcd.setCursor(0, 1);
  lcd.print("Date:");
  lcd.print(year % 100);
  lcd.print("Y");
  lcd.print(":");
  if (month / 10 == 0) lcd.print("0");
  lcd.print(month);
  lcd.print("m");
  lcd.print(":");
  if (days / 10 == 0) lcd.print("0");
  lcd.print(days);
  lcd.print("d");


  lcd.setCursor(0, 0);

  lcd.print("Time:");
  lcd.print(hours);
  lcd.print("h");
  lcd.print(":");
  if (minutes / 10 == 0) lcd.print("0");
  lcd.print(minutes);
  lcd.print("m");
  lcd.print(":");
  if (seconds / 10 == 0) lcd.print("0");
  lcd.print(seconds);
  lcd.print("s");


}

void printCenter(String text, int cursorRow)
{

  unsigned int textLength = text.length();
  int paddingLeft = 0;
  if (textLength <= 16) {
    paddingLeft = (16 - textLength) / 2;
  }
  lcd.setCursor(paddingLeft, cursorRow);
  lcd.print(text);
}

void setTime() {
  seconds++;
  if (seconds >= 60) {
    minutes++;
    seconds = 0;
  }
  if (minutes >= 60) {
    hours++;
    minutes = 0;
  }
  if (hours > 24) {
    days++;
  }
  setMonth();
  setYear();
}

void setMonth() {
  if (days == 30 && month == 2 && (year % 100 == 0 && year % 400 != 0))
    nextMonth();
  else if (days == 29 && month == 2 && year % 4 == 0)
    nextMonth();
  else if (days == 31 && (month == 4 || month == 6 || month == 9 || month == 11))
    nextMonth();
  else if (days == 32) {
    nextMonth();
  }
}

void nextMonth() {
  days = 0;
  month++;
}

void setYear() {
  if (month == 12 && days == 31)
    year++;
}

bool isMenuBTNClicked() {
  if (menuButton == HIGH && !menuButtonPressed) {
    menuButtonPressed = true;
    delayForButton();
    return true;
  }
  if (menuButton == LOW)
    menuButtonPressed = false;
  delayForButton();
  return false;
}

bool isRightBTNClicked() {
  if (rightButton == HIGH && !rightButtonPressed) {
    rightButtonPressed = true;
    delayForButton();
    return true;
  }
  if (rightButton == LOW)
    rightButtonPressed = false;
  delayForButton();
  return false;
}

bool isLeftBTNClicked() {
  if (leftButton == HIGH && !leftButtonPressed) {
    leftButtonPressed = true;
    delayForButton();
    return true;
  }
  if (leftButton == LOW)
    leftButtonPressed = false;
  delayForButton();
  return false;
}

bool isResetBTNClicked() {
  if (resetButton == HIGH && !resetButtonPressed) {
    resetButtonPressed = true;
    delayForButton();
    return true;
  }
  if (resetButton == LOW)
    resetButtonPressed = false;
  delayForButton();
  return false;
}

bool anyBTNPressed() {
  return resetButton == HIGH || rightButton == HIGH || leftButton == HIGH || menuButton == HIGH;
}

void printStopwatch() {

  if (lastSecond != programSeconds) {
    printStopwatchTime();
    lastSecond = programSeconds;
  }

  if (!menuPrinted) {
    menuPrinted = true;
    printStopwatchMenu();
  }


}

void clearBottom() {
  printCenter("                ", 1);
}

void clearTop() {
  printCenter("                ", 0);
}

void printStopwatchMenu() {
  clearBottom();
  //Set minutes/seconds
  if (settingStopwatch) {
    if (settingMinutes) {
      printCenter(">minutes++|Start", 1);
    }
    else {
      printCenter(">seconds++|Start", 1);
    }
  }
  //Set start/pause
  else {
    if (!stopwatchRunning)
    {
      if (settingMinutes) {
        printCenter("minutes++|>Start", 1);
      }
      else {
        printCenter("seconds++|>Start", 1);
      }
    }
    else {
      if (settingMinutes) {
        printCenter("minutes++|>Pause", 1);
      }
      else {
        printCenter("seconds++|>Pause", 1);
      }
    }
  }
}

void delayForButton() {
  delay(delayAmmountBTN);
}

void resetStopwatch() {
  stopwatchSeconds = 0;
  stopwatchMinutes = 0;
  stopwatchRunning = false;
}

void printStopwatchTime() {
  clearTop();
  if (stopwatchRunning) {
    if (stopwatchSeconds == 0 && stopwatchMinutes > 0) {
      stopwatchMinutes--;
      stopwatchSeconds = 59;
    }
    else if (stopwatchSeconds > 0)
    {
      stopwatchSeconds--;
    }
    else {
      lcd.clear();
      notifyLight();
      printCenter("TIME IS OUT!", 0);
      state = idleState;
      resetStopwatch();
      return;
    }
  }

  String printedStopwatch = "";
  printedStopwatch.concat(stopwatchMinutes);
  printedStopwatch.concat(":");
  if (stopwatchSeconds / 10 == 0) printedStopwatch.concat("0");
  printedStopwatch.concat(stopwatchSeconds);
  printCenter(printedStopwatch, 0);
}

void printTimer() {
  if (lastSecond != programSeconds) {

    tickTimer();
    printTimerTime();
    lastSecond = programSeconds;

  }

  if (!menuPrinted) {
    menuPrinted = true;
    resetTimer();
  }
}

void tickTimer() {
  if (timerRunning) {
    if (timerSeconds == 60) {
      timerMinutes++;
      timerSeconds = 0;
    }
    timerSeconds++;
  }
}


void printTimerTime() {
  clearTop();
  String printedStopwatch = "";
  printedStopwatch.concat(timerMinutes);
  printedStopwatch.concat(":");
  if (timerSeconds / 10 == 0) printedStopwatch.concat("0");
  printedStopwatch.concat(timerSeconds);
  printCenter(printedStopwatch, 0);
}

void printTimerMenu() {
  clearBottom();
  if (timerRunning) {
    printCenter("STARTED!", 1);
  }
  else {
    printCenter("PAUSED!", 1);
  }
}

void resetTimer() {
  clearBottom();
  printCenter("STOPWATCH", 1);
  timerSeconds = 0;
  timerMinutes = 0;
  timerRunning = false;
}


void printTitle() {
  if (state == clockState) {
    printCenter("DATE AND TIME", 0);
  }
  else if (state == timerMenu) {
    printCenter("STOPWATCH", 0);
  }
  else if (state == alarmMenu) {
    printCenter("ALARM", 0);
  }
  else if (state == stopwatchMenu) {
    printCenter("TIMER", 0);
  }
  delay(1000);
}

void notifyLight() {
  int i;
  for (i = 0; i < 100; i++) {
    digitalWrite(ledPin, HIGH);
    delay(25);
    digitalWrite(ledPin, LOW);
    delay(25);
    if (anyBTNPressed())
    {
      turnOffAlarm();
      break;
    }
  }
}

void turnOffAlarm() {
  alarmIsUp = false;
}

void printAlarmTime() {
  clearTop();
  String printedStopwatch = "";
  if (alarmHours / 10 == 0) printedStopwatch.concat("0");
  printedStopwatch.concat(alarmHours);

  printedStopwatch.concat(":");

  if (alarmMinutes / 10 == 0) printedStopwatch.concat("0");
  printedStopwatch.concat(alarmMinutes);

  printCenter(printedStopwatch, 0);
}

void printAlarmMenu() {
  clearBottom();
  if (alarmOn) {
    printCenter("Alarm is running", 1);
    return;
  }
  printCenter("Set the alarm", 1);
}

void blinkSetClock(int col, int row) {
  lcd.setCursor(col, row);
  if (millis() % 1000 < 200)
    lcd.print("00");
  else
    printTime();
  lcd.setCursor(0, 0);
}
