// include the library code:
#include <LiquidCrystal.h>

typedef void (*termination_handler)();

typedef struct {
  int8_t h;
  int8_t m;
  int8_t s;
} Time;

enum State : int8_t {
  Center,
  Top,
  Bottom,
  Left,
  Right,
};

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


int8_t mode = 0; // 0 - Menu; 1 - Clock; 2 - Timer; 3 - Stopwatch;


State state;

InputX inputX;
InputY inputY;

// ----- Pins -----

const int16_t buzzer = 6;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


// ========== Input ===========================================================

static bool isChangedX = false;
static bool isChangedY = false;

void readInput(int16_t adcX, int16_t adcY) {
  static int16_t adcX_last = adcX;
  static int16_t adcY_last = adcY;

  if (adcX > adcX_last + 50 || adcX < adcX_last - 50) {
    isChangedX = true;
  }

  if (adcY > adcY_last + 50 || adcY < adcY_last - 50) {
    isChangedY = true;
  }
}

void registerEvent(int16_t adcX, int16_t adcY) {
  const int16_t center = 510;
  const int16_t threshold = 150;

  if (isChangedX) {
    if (adcX > center + threshold) {
      inputX = InputX::SwitchModeRight;

    } else if (adcX < center - threshold) {
      inputX = InputX::SwitchModeLeft;

    } else {
      inputX = InputX::IdleX;
    }
  }

  if (isChangedY) {
    if (adcY > center + threshold) {
      inputY = InputY::IncreaseTime;
    
    } else if (adcY < center - threshold) {
      inputY = InputY::DecreaseTime;

    } else {
      inputY = InputY::IdleY;
    }
  }
}


// ========== MODES ===========================================================

// ---------- Timer -----------------------------------------------------------

static Timer time;
static char timer_string[9] = "00:00:00\0";
static int8_t timer_section = 0; // 0 - hours; 1 - minutes; 2 - seconds;
static bool isRunningTimer = false;

void setTime(Timer &timer) {
  snprintf(timer_string, sizeof(time_string), "%02d:%02d:%02d", timer.h, timer.m, timer.s);
}

void startTimer() {
  static int8_t start_timer_last = 0;

  if (inputX == InputX::SwitchModeRight && millis() - start_timer_last >= 3000) {
    start_timer_last = millis();

    isRunningTimer = true;
  }
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

void runTimer(termination_handler handler ) {
  while (isRunningTimer) {

    if( timer.s > 0 ) {
      timer.s--;

    } else if( timer.m > 0 ) {
      timer.m--;
      timer.s = 59;

    } else if( timer.h > 0 ) {
      timer.h--;
      timer.m = 59;

    } else {
      handler();
      isRunningTimer = false;
    }
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


// ========== DISPLAY =========================================================


void display() {
  switch (mode) {
    case 0 : 
      showMenu(0);
      break;

    // case Mode::Timer : 
    // case Mode::Clock : 
    // case Mode::Stopwatch : 
  }
}

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

void showMenu(int8_t menu_mode) {
  switch (menu_mode) {
    case 1 : updateDisplay("Menu", "Clock"); break;
    case 2 : updateDisplay("Menu", "Timer"); break;
    case 3 : updateDisplay("Menu", "Stopwatch"); break;
    default: updateDisplay("Menu", "Up Down"); break;
  }
}




// ========== Control =========================================================

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


// ========== PROGRAM =========================================================

static int16_t millies_last = 0;
static int64_t delay_last = 0;

void setup() {
  Serial.begin( 9600 );
  pinMode( buzzer, OUTPUT );

  lcd.begin( 16, 2 );
  // lcd.setCursor( 0, 0 );
  lcd.noCursor();
  lcd.noAutoscroll();
  
  time = { 0,0,0 };
  state = State::Center;
  inputX = InputX::IdleX;
  inputY = InputY::IdleY;
}

void loop() {
  int64_t curMillis = millis();

  // int16_t adcX = analogRead( A0 );
  // int16_t adcY = analogRead( A1 );

  // readInput(adcX, adcY);
  // registerEvent(adcX, adcY);

  // control();
  if ((curMillis - delay_last) >= 100) {
    display();


    delay_last = millis();
  }
}
