// Arduino C++ - TicTacToe style NeoPixel + buttons
#include <Adafruit_NeoPixel.h>
#include <Adafruit_LiquidCrystal.h>

#define PIXEL_PIN 11      // data pin for NeoPixels 
#define NUMPIXELS 9
#define WINROW 8 

// --- delayed construction (pointers) ---
Adafruit_NeoPixel *pixels = nullptr;
Adafruit_LiquidCrystal *lcd_1 = nullptr;

static uint8_t has_started = 0; // checking if the game has already started
uint8_t red_counter = 0; 
uint8_t blue_counter = 0;

uint32_t COLOR_RED;
uint32_t COLOR_BLUE;

uint8_t cur_color = 0; // 0 -> red, 1 -> blue

// Map (0..8) to a physical Arduino input pin
const uint8_t buttonPin[NUMPIXELS] = {
  12, 2, 3,
  4, 5, 6,
  7, 8, 9
};

// Win conditions using 0-based pixel indices
const uint8_t winCondition[WINROW][3] = {
  {0, 1, 2},
  {3, 4, 5},
  {6, 7, 8},
  {0, 3, 6},
  {1, 4, 7},
  {2, 5, 8},
  {0, 4, 8},
  {2, 4, 6}
};

bool ledsDirty = false;      
bool lcdDirty = false;       

bool lcdReady = false;
bool ledsReady = false;

unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_MIN_INTERVAL = 30; // ms, avoid too-frequent updates

char lcdLine0[17] = ""; // 16 chars + null
char lcdLine1[17] = "";

// Change pixels (setPixelColor / clear)
inline void markLedsDirty() { ledsDirty = true; }

// Set buffer content for LCD (does not touch hardware)
void setLCDText(const char *l0, const char *l1) {
  strncpy(lcdLine0, l0, 16); lcdLine0[16] = '\0';
  strncpy(lcdLine1, l1, 16); lcdLine1[16] = '\0';
  lcdDirty = true;
}

// Actual hardware flush for LCD (only when lcdReady)
void flushLCD() {
  if (!lcdReady) return;
  lcd_1->clear();
  lcd_1->setCursor(0, 0);
  lcd_1->print(lcdLine0);
  lcd_1->setCursor(0, 1);
  lcd_1->print(lcdLine1);
  lcdDirty = false;
}

void safePrintLCD(const char *l0, const char *l1) {
  setLCDText(l0, l1);
  flushLCD();
}

// Single central updater — call once per loop()
void updateDisplay() {
  unsigned long now = millis();
  // throttle 
  if (now - lastDisplayUpdate < DISPLAY_MIN_INTERVAL) return;

  // 1) update LED strip if needed
  if (ledsDirty && ledsReady) {
    pixels->show();
    ledsDirty = false;
  }

  // 2) update LCD if needed
  if (lcdDirty && lcdReady) {
    flushLCD();
  }

  lastDisplayUpdate = now;
}

// return pixel index (0..8) if pressed, else -1
int readButtonIndex() {
  for (uint8_t i = 0; i < NUMPIXELS; i++) {
    if (digitalRead(buttonPin[i]) == LOW) {
      Serial.println("Button press detected");
      delay(10); 
      if (digitalRead(buttonPin[i]) == LOW) {
        return (int)i; // return the pixel index corresponding to this button
      }
    }
  }
  return -1;
}

int checkComplete() {
  // return 1 if all pixels non-zero, else -1
  if (!ledsReady) return -1;
  for (uint8_t i = 0; i < NUMPIXELS; i++) {
    if (pixels->getPixelColor(i) == 0) return -1;
  }
  Serial.println("Game complete");
  return 1;
}

void gameOver() {
  if (!ledsReady) return;

  int winning_row = -1;

  // find winning row: all three pixels same non-zero color
  for (uint8_t r = 0; r < WINROW; r++) {
    uint32_t c0 = pixels->getPixelColor(winCondition[r][0]);
    uint32_t c1 = pixels->getPixelColor(winCondition[r][1]);
    uint32_t c2 = pixels->getPixelColor(winCondition[r][2]);

    if (c0 != 0 && c0 == c1 && c1 == c2) {
      winning_row = r;
      break;
    }
  }

  if (winning_row == -1) {
    // Check if all LEDs are on
    int is_complete = checkComplete(); 
    if (is_complete == 1 && has_started == 1) {
      setLCDText("It's a tie!", "");
      has_started = 0;
      return; 
    }
    // don't do anything 
    return;
  }

  uint8_t pix1 = winCondition[winning_row][0];
  uint8_t pix2 = winCondition[winning_row][1];
  uint8_t pix3 = winCondition[winning_row][2];

  // turn off other pixels
  for (uint8_t i = 0; i < NUMPIXELS; i++) {
    if (i != pix1 && i != pix2 && i != pix3) {
      pixels->setPixelColor(i, 0);
    }
  }
  markLedsDirty(); 
  
  uint32_t winnerColor = pixels->getPixelColor(pix1);
  
  if (winnerColor == COLOR_RED) {
    red_counter ++;
    char line_buffer[20]; 
    sprintf(line_buffer, "Red %d vs Blue %d", red_counter, blue_counter);
    safePrintLCD(line_buffer, "RED wins!");
    memset(line_buffer, 0, sizeof(line_buffer));
  } else if (winnerColor == COLOR_BLUE) {
    blue_counter ++; 
    char line_buffer[20]; 
    sprintf(line_buffer, "Red %d vs Blue %d", red_counter, blue_counter);
    safePrintLCD(line_buffer, "BLUE wins!");
    memset(line_buffer, 0, sizeof(line_buffer));
  } else {
    setLCDText("ERROR: Unknown Color", "");
  }

  has_started = 0; 
  return;
}

void setColorForPixel(uint8_t idx) {
  if (!ledsReady) return;
  Serial.println("Setting pixel colors");
  Serial.print("Pixel position: "); Serial.println(idx);
  Serial.print(" The current color is: "); Serial.println(cur_color);

  if (idx >= NUMPIXELS) return;

  // if already colored, do nothing
  if (pixels->getPixelColor(idx) != 0) return;

  if (cur_color == 0) {
    pixels->setPixelColor(idx, COLOR_RED);
    cur_color = 1;
  } else {
    pixels->setPixelColor(idx, COLOR_BLUE);
    cur_color = 0;
  }
  
  markLedsDirty();
}

void showTurn() {
  // show current turn.
  if (!lcdReady) return; 
  switch (cur_color) {
    case 0:
      setLCDText("RED's turn", "");
      break;
    case 1:
      setLCDText("BLUE's turn", "");
      break;
    default:
      setLCDText("ERROR", "");
      break;
  }
}

void reset() {
  // handles game reset 
  for (uint8_t i = 0; i < NUMPIXELS; i++) {
    if (digitalRead(buttonPin[i]) == LOW) {
      has_started = 1; 
      delay(30); // debouncing
      if (digitalRead(buttonPin[i]) != LOW) return;
      Serial.println("Resetting");
      if (ledsReady) {
        pixels->clear();
        markLedsDirty();
      }
      
      setLCDText("NEXT ROUND:", "Red first");
      
      if (red_counter + blue_counter <= 3) {
        if (lcdReady) lcd_1->clear();
        cur_color = 0; 
        break; 
      } else {
        if (ledsReady) {
          pixels->clear();
        }
        red_counter = 0;
        blue_counter = 0;
        if (lcdReady) lcd_1->clear(); 
        cur_color = 0; 
        break;
      }
    }
  }
}

void setup() {
  Serial.begin(9600);

  // create pixels object in setup 
  pixels = new Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
  pixels->begin();
  pixels->clear();
  pixels->show();
  ledsReady = true;

  // create lcd object in setup
  lcd_1 = new Adafruit_LiquidCrystal(0);
  lcd_1->begin(16, 2);
  lcd_1->clear();
  lcdReady = true;

  // define named colors 
  COLOR_RED = pixels->Color(255, 0, 0);
  COLOR_BLUE = pixels->Color(0, 0, 255);

  // init button pins
  for (uint8_t i = 0; i < NUMPIXELS; i++) {
    Serial.println("Set up buttons");
    pinMode(buttonPin[i], INPUT_PULLUP);
  }
  Serial.println("Set up complete");

  safePrintLCD("Tic Tac Toe", "RED first");
  has_started = 1; 
  delay(200);
}

void loop() {
  Serial.print("has_started value: ");
  Serial.println(has_started);

  if (!has_started) {
    safePrintLCD("Press to reset", "");
    reset();
    updateDisplay();
  } else {
    gameOver();
    showTurn();
    int idx = readButtonIndex();
    if (idx != -1) {
      setColorForPixel(idx);
    }
    updateDisplay();
    delay(50); // small delay
  }
}
