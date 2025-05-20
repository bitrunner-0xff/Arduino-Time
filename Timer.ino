// include the library code:
#include <LiquidCrystal.h>

// ----- Pins -----

const int16_t buzzer = 6;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


// ========== Input ===========================================================

enum Input : int8_t {
  Idle,
  Right,
  Left, 
  Up,
  Down,
};

Input inputX = Input::Idle;;
Input inputY = Input::Idle;;

void readInput(int16_t adcX, int16_t adcY) {
  const int16_t threshold = 100;

  static int16_t adcX_last = adcX;
  static int16_t adcY_last = adcY;

  if (adcX > adcX_last + threshold) {
    inputX = Input::Right;

  } else if (adcX < adcX_last - threshold) {
    inputX = Input::Left;

  } else {
    inputX = Input::Idle;
  }

  if (adcY > adcY_last + threshold) {
    inputY = Input::Up;
  
  } else if (adcY < adcY_last - threshold) {
    inputY = Input::Down;

  } else {
    inputY = Input::Idle;
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
static char timer_string[9] = "00:00:00\0";
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
    case 0: timer.h = timer.h >= 99 ? timer.h = 0 : timer.h += 1; break;
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

static Time stopwatch = {0,0,0};
static char stopwatch_string[9] = "00:00:00\0";
static bool isRunningStopwatch = false;

const char* getStopwatch() {
  return stopwatch_string;
}

void updateStopwatch() {
  snprintf(stopwatch_string, sizeof(stopwatch_string), "%02d:%02d:%02d", stopwatch.h, stopwatch.m, stopwatch.s);
}

void startStowatch() {
  isRunningStopwatch = true;
}

void runStopwatch() {
  if (isRunningStopwatch) {

    if( stopwatch.s < 59 ) {
      stopwatch.s++;

    } else if( stopwatch.m < 59 ) {
      stopwatch.m++;
      stopwatch.s = 0;

    } else if( stopwatch.h < 99 ) {
      stopwatch.h++;
      stopwatch.m = 0;

    } else {
      isRunningStopwatch = false;
    }

    updateStopwatch();
    Serial.println(getStopwatch());
  }
}



// ========== Control =========================================================

enum Mode : int8_t {
  Clock,
  Timer,
  Stopwatch,
};

Mode mode;
bool isSettingMode = false;

int8_t getMode() {
  return mode;
}

void modeSwtch(int32_t millis_val) {
  if (isSettingMode) return;

  static int32_t prev_delay = 0;
  bool isDelayExceed = millis_val - prev_delay >= 2000;

  if (inputY == Input::Down && isDelayExceed) {
    switch (mode) {
      case Clock: mode = Timer; break;
      case Timer: mode = Stopwatch; break;
      case Stopwatch: mode = Clock; break;
    }

    prev_delay = millis_val;
  }

  if (inputY == Input::Up && isDelayExceed) {
    switch (mode) {
      case Clock: mode = Stopwatch; break;
      case Timer: mode = Clock; break;
      case Stopwatch: mode = Timer; break;
    }

    prev_delay = millis_val;
  }
}

void timerControl(int32_t millis_val) {
  static int32_t prev_delay = 0;
  static int32_t prev_long_delay = 0;
  static int32_t prev_return_delay = 0;
  static bool is_wait_to_trigger_run = false;
  static bool is_wait_to_trigger_quit = false;

  bool isDelayReady = millis_val - prev_delay >= 300;
  bool isLongActionReady = millis_val - prev_long_delay >= 2000;

  switch (inputX) {
    case Right: 
      if (isDelayReady) {
        switchModeRight();
        prev_delay = millis_val;
      } 

      if (!is_wait_to_trigger_run) {
        prev_long_delay = millis_val;
        is_wait_to_trigger_run = true;

      } else if (isLongActionReady) {
        is_wait_to_trigger_run = false;
        isSettingMode = false;
        startTimer();
      }
      break;
      
    case Left: 
      if (isDelayReady) {
        switchModeLeft();
        prev_delay = millis_val;
      }  

      if (!is_wait_to_trigger_quit) {
        prev_long_delay = millis_val;
        is_wait_to_trigger_quit = true;

      } else if (isLongActionReady) {
        isSettingMode = false;
        is_wait_to_trigger_quit = false;
      }
      break;
  }
 
  if (inputY != Idle && isDelayReady) {
    switch (inputY) {
      case Up: increaseTime(); setTimer(); break;
      case Down: decreaseTime(); setTimer(); break;
    }

    prev_delay = millis_val;
  }
}

void modeSettings(int32_t millis_val) {
  if (isSettingMode) {
    switch (mode) {
      case Clock: break;
      case Timer: timerControl(millis_val); break;
      case Stopwatch: break;
    }
  }
}

void setModeSetting(int32_t millis_val) {
  static int32_t prev_delay = 0;
  static bool waiting_to_trigger = false;
  
  if (!isSettingMode) {
    if (inputX == Input::Right) {
      if (!waiting_to_trigger) {
        prev_delay = millis_val;
        waiting_to_trigger = true;

      } else if (millis_val - prev_delay >= 2000) {
        isSettingMode = true;
        waiting_to_trigger = false;

        Serial.println("Setting mode enagled");

      }
    } else {
      waiting_to_trigger = false;
    }
  }
}


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
  switch (getMode()) {
    case Clock: 
      updateDisplay("Clock", "Unknown");
      break;
    case Timer:
      updateDisplay("Timer", getTimer());

      break;
    case Stopwatch:
      updateDisplay("Stopwatch", getStopwatch());
      break;
  }
}


// ========== PROGRAM =========================================================

void setup() {
  Serial.begin( 9600 );
  pinMode( buzzer, OUTPUT );

  lcd.begin( 16, 2 );
  lcd.noCursor();
  lcd.noAutoscroll();
  startStowatch();
}

void loop() {
  static int64_t millies_last = 0;
  static int64_t display_last = 0;
  static int64_t time_last = 0;

  int64_t curMillis = millis();

  int16_t adcX = analogRead( A0 );
  int16_t adcY = analogRead( A1 );

  readInput(adcX, adcY);

  modeSwtch(curMillis);
  setModeSetting(curMillis);
  modeSettings(curMillis);

  if (curMillis - time_last >= 1000) {
    runTimer(runBuzzer);
    runStopwatch();

    time_last = millis();
  }

  if ((curMillis - display_last) >= 100) {
    display();

    display_last = millis();
  }
}
