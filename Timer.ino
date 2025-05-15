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

enum Mode : int8_t {
  Clock,
  Timer,
  Stopwatch,
  Settings,
};

State state;
InputX inputX;
InputY inputY;
Mode mode;

// ----- Pins -----

const int16_t buzzer = 6;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


// ========== DISPLAY =========================================================

static char time_string[9] = "00:00:00\0";

void setDisplay(Time &time) {
  snprintf(time_string, sizeof(time_string), "%02d:%02d:%02d", time.h, time.m, time.s);
}

void showDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(time_string);
  lcd.noCursor();
}

// void showDisplay() {
//   Serial.print(time_string);
//   Serial.println(" ");
// }

// ========== Input ===========================================================

static bool isChangedX = false;
static bool isChangedY = false;

int16_t adcX = 0; 
int16_t adcY = 0;

void readInput() {
  static int16_t adcX_last = adcX;
  static int16_t adcY_last = adcY;

  if (adcX > adcX_last + 50 || adcX < adcX_last - 50) {
    isChangedX = true;
  }

  if (adcY > adcY_last + 50 || adcY < adcY_last - 50) {
    isChangedY = true;
  }
}

void registerEvent() {
  const int16_t center = 510;
  const int16_t threshold = 150;

  int16_t deltaX = abs(adcX - center);
  int16_t deltaY = abs(adcY - center);

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

  Serial.print("inputX: ");
  Serial.print(inputX);
  Serial.print("; ");
  Serial.print("inputY: ");
  Serial.print(inputY);
  Serial.println(" ");
}


// ========== MODES ===========================================================

// ---------- Timer -----------------------------------------------------------

static Time time;
static int8_t timer_section = 0; // 0 - hours; 1 - minutes; 2 - seconds;
static bool isRunningTimer = false;

void startTimer() {
  static int8_t left_input_delay = 0;

  if (inputX == InputX::SwitchModeRight && millis() - left_input_delay >= 3000) {
    left_input_delay = millis();

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
    case 0: time.h = time.h > 99 ? time.h = 0 : time.h += 1; break;
    case 1: time.m = time.m > 59 ? time.m = 0 : time.m += 1; break;
    case 2: time.s = time.s > 59 ? time.s = 0 : time.s += 1; break;
  }
}

void decreaseTime() {
  switch (timer_section) {
    case 0: time.h = time.h < 0 ? time.h = 99 : time.h -= 1; break;
    case 1: time.m = time.m < 0 ? time.m = 59 : time.m -= 1; break;
    case 2: time.s = time.s < 0 ? time.s = 59 : time.s -= 1; break;
  }
}

void runTimer( Time& time, termination_handler handler ) {
  while (isRunningTimer) {

    if( time.s > 0 ) {
      time.s--;

    } else if( time.m > 0 ) {
      time.m--;
      time.s = 59;

    } else if( time.h > 0 ) {
      time.h--;
      time.m = 59;

    } else {
      handler();
      isRunningTimer = false;
      delay(3000);
    }

    delay(1000);
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


// ---------- State machine ----------

// static bool isRunning;


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

void setup() {
  Serial.begin( 9600 );
  pinMode( buzzer, OUTPUT );

  lcd.begin( 16, 1 );
  lcd.setCursor( 0, 0 );
  lcd.noCursor();
  lcd.noAutoscroll();
  
  time = { 0,0,0 };
  state = State::Center;
  inputX = InputX::IdleX;
  inputY = InputY::IdleY;
  mode = Mode::Settings;
}

void loop() {
  digitalWrite( buzzer, LOW );

  adcX = analogRead( A0 );
  adcY = analogRead( A1 );

  readInput();
  registerEvent();

  setDisplay(time);


  if (millis() - millies_last > 100) {
    millies_last = millis();

    showDisplay();
  }
}
