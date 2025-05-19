// include the library code:
#include <LiquidCrystal.h>

// ----- Pins -----

const int16_t buzzer = 6;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


// ========== Input ===========================================================

enum InputX : int8_t {
  IdleX,
  SwitchModeRight,
  SwitchModeLeft, 
};

enum InputY : int8_t {
  IdleY,
  IncreaseTime,
  DecreaseTime,
};

InputX inputX;
InputY inputY;

void readInput(int16_t adcX, int16_t adcY) {
  const int16_t threshold = 100;

  static int16_t adcX_last = adcX;
  static int16_t adcY_last = adcY;

  if (adcX > adcX_last + threshold) {
    inputX = InputX::SwitchModeRight;

  } else if (adcX < adcX_last - threshold) {
    inputX = InputX::SwitchModeLeft;

  } else {
    inputX = InputX::IdleX;
  }


  if (adcY > adcY_last + threshold) {
    inputY = InputY::IncreaseTime;
  
  } else if (adcY < adcY_last - threshold) {
    inputY = InputY::DecreaseTime;

  } else {
    inputY = InputY::IdleY;
  }
}


// ========== MODES ===========================================================

typedef struct {
  int8_t h;
  int8_t m;
  int8_t s;
} Time;

// ---------- Timer -----------------------------------------------------------

static Time timer = {0,0,0};
static char timer_string[9];
static int8_t timer_section = 0; // 0 - hours; 1 - minutes; 2 - seconds;
static bool isRunningTimer = false;

void setTimer() {
  snprintf(timer_string, sizeof(timer_string), "%02d:%02d:%02d", timer.h, timer.m, timer.s);
}

void startTimer() {
  isRunningTimer = true;
}

const char* getTimer() {
  return timer_string;
}

void switchModeRight() {
  ++timer_section;
  if (timer_section > 2) timer_section = 0;
}

void switchModeLeft() {
  --timer_section;
  if (timer_section < 0) timer_section = 2;
}

void increaseTime() {
  switch (timer_section) {
    case 0: timer.h = timer.h > 99 ? timer.h = 0 : timer.h += 1; break;
    case 1: timer.m = timer.m > 59 ? timer.m = 0 : timer.m += 1; break;
    case 2: timer.s = timer.s > 59 ? timer.s = 0 : timer.s += 1; break;
  }
}

void decreaseTime() {
  switch (timer_section) {
    case 0: timer.h = timer.h < 0 ? timer.h = 99 : timer.h -= 1; break;
    case 1: timer.m = timer.m < 0 ? timer.m = 59 : timer.m -= 1; break;
    case 2: timer.s = timer.s < 0 ? timer.s = 59 : timer.s -= 1; break;
  }
}

// typedef void (*termination_handler)();
void runTimer(void (*termination_handler)()) {
  if (isRunningTimer) {

    if( timer.s > 0 ) {
      timer.s--;

    } else if( timer.m > 0 ) {
      timer.m--;
      timer.s = 59;

    } else if( timer.h > 0 ) {
      timer.h--;
      timer.m = 59;

    } else {
      termination_handler();
      isRunningTimer = false;
    }

    setTimer();
  }
}

void runBuzzer() {
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer, LOW);
}

// ---------- Clock -----------------------------------------------------------

// TODO;

// ---------- Stopwatch -------------------------------------------------------

// TODO;



// ========== Control =========================================================

enum State : int8_t {
  Center,
  Top,
  Bottom,
  Left,
  Right,
};

int8_t mode = 0; // 0 - Clock; 1 - Timer; 2 - Stopwatch;

State state;

// void control() {
//   switch (mode) {
//     case 0: 
//     case 1: 
//     case 2:
//   }

// }

// void updateStateMachine() {
//   switch (state) {
//     case State::Center : 
//       switch (input) {
//         case Input::IncreaseTime: 
//           state = State::Top; 
//           increaseTime(); 
//           break;

//         case Input::DecreaseTime: 
//           state = State::Bottom; 
//           decreaseTime(); 
//           break;

//         case Input::SwitchModeRight: 
//           state = State::Right; 
//           switchModeRight(); 
//           break;

//         case Input::SwitchModeLeft: 
//           state = State::Left; 
//           switchModeLeft(); 
//           break;
//       } 
//       return;

//     case State::Bottom : 
//       if (input == Input::Idle) {
//         state = State::Center;
//       } 
//       return;

//     case State::Top : 
//       if (input == Input::Idle) {
//         state = State::Center;
//       } 
//       return;

//     case State::Left : 
//       if (input == Input::Idle) {
//         state = State::Center;
//       } else if (input == Input::RunTimer) {
//         state = State::Running;
//         isRunning = true;
//         runTimer(time, runBuzzer);
//       }
//       return;

//     case State::Right : 
//       if (input == Input::Idle) {
//         state = State::Center;
//       } 
//       return;

//     case State::Running : 
//       if (input == Input::Idle) {
//         state = State::Center;
//       } 
//       return;
//   }
// }


// ========== DISPLAY =========================================================


void updateDisplay(const char *row1 = nullptr, const char *row2 = nullptr) {
  lcd.clear();

  if (row1) {  
    lcd.setCursor(0, 0);
    lcd.print(row1);
  }

  if (!row1 && row2) {
    lcd.setCursor(0, 0);
    lcd.print(row2);

  } else if (row2) {
    lcd.setCursor(0, 1);
    lcd.print(row2);
  }
}

void display() {
  switch (mode) {
    case 0 : 
      updateDisplay("Clock", "Unknown");
      break;
    case 1:
      updateDisplay("Timer", getTimer());
      break;
    case 2:
      updateDisplay("Stopwatch", "Unknown");
      break;
    // default: 
  }
}


// ========== PROGRAM =========================================================

static int32_t millies_last = 0;
static int32_t display_last = 0;
static int32_t timer_last = 0;

void setup() {
  Serial.begin( 9600 );
  pinMode( buzzer, OUTPUT );

  lcd.begin( 16, 2 );
  lcd.noCursor();
  lcd.noAutoscroll();
  
  state = State::Center;
  inputX = InputX::IdleX;
  inputY = InputY::IdleY;
}

void loop() {
  int32_t curMillis = millis();

  int16_t adcX = analogRead( A0 );
  int16_t adcY = analogRead( A1 );

  readInput(adcX, adcY);

  // control();

  if (curMillis - timer_last >= 1000) {
    runTimer(runBuzzer);

    timer_last = millis();
  }

  if ((curMillis - display_last) >= 100) {
    display();

    display_last = millis();
  }
}
