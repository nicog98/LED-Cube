const int BUTTON = A0;
const int DEBOUNCE_DELAY = 75;
const byte ANODE_PINS[8] = {2, 3, 4, 5, A5, A4, A3, A2};
const byte CATHODE_PINS[8] = {6, 8, 10, 12, 7, 9, 11, 13};
const int ELAPSE = 100;

void setup() {
  // turns all LED pins to outputs, and all LED's off:
  for (byte i = 0; i < 8; i++) {
    pinMode(ANODE_PINS[i], OUTPUT);
    pinMode(CATHODE_PINS[i], OUTPUT);
    digitalWrite(ANODE_PINS[i], HIGH);
    digitalWrite(CATHODE_PINS[i], HIGH);
  }

  pinMode(BUTTON, INPUT_PULLUP);
  
  Serial.begin(115200);
  Serial.setTimeout(100);
}

/* Function: getLEDState
 * ---------------------
 * Returns the state of the LED in a 4x4x4 pattern array, each dimension
 * representing an axis of the LED cube, that corresponds to the given anode (+)
 * wire and cathode (-) wire number.
 *
 * This function is called by display(), in order to find whether an LED for a
 * particular anode (+) wire and cathode (-) wire should be switched on.
 */
inline byte getLEDState(byte pattern[4][4][4], byte aNum, byte cNum)
{
  // TODO: fill this in to return the value in the pattern array corresponding
  // to the anode (+) wire and cathode (-) wire number (aNum and cNum) provided.
  
  // the 'x' coordinate of the light will be the value of aNum if it is less than 3
  // if it is not, it will be the value of aNum % 4
  byte x;
  if (aNum > 3) {
    x = aNum % 4;
  } else {
    x = aNum;
  }

  // the 'y' coordinate will be the value of cNum if it is less than 3
  // if it is not, it will be cNum % 4
  byte y;
  if (cNum > 3) {
    y = cNum % 4;
  } else {
    y = cNum;
  }

  byte z;
  if (aNum > 3 && cNum < 4) {
    z = 3;
  } else {
    z = aNum/4 + cNum/4;
  }
  
  return pattern[z][y][x];
}

/* Function: display
 * -----------------
 * Runs through one multiplexing cycle of the LEDs, controlling which LEDs are
 * on.
 *
 * During this function, LEDs that should be on will be turned on momentarily,
 * one row at a time. When this function returns, all the LEDs will be off
 * again, so it needs to be called continuously for LEDs to be on.
 */
void display(byte pattern[4][4][4])
{
  for (byte aNum = 0; aNum < 8; aNum++) { // iterate through anode (+) wires

    // Set up all the cathode (-) wires first
    for (byte cNum = 0; cNum < 8; cNum++) { // iterate through cathode (-) wires
      byte value = getLEDState(pattern, aNum, cNum); // look up the value
      // Activate the cathode (-) wire if `value` is > 0, otherwise deactivate it
      if (value > 0) {
        digitalWrite(CATHODE_PINS[cNum], LOW);
      } else {
        digitalWrite(CATHODE_PINS[cNum], HIGH);
      }
    }

    // Activate the anode (+) wire (without condition)
    digitalWrite(ANODE_PINS[aNum], LOW);

    // Wait a short time
    delayMicroseconds(100);
    
    //We're now done with this row, so deactivate the anode (+) wire
    digitalWrite(ANODE_PINS[aNum], HIGH);


  }
}

bool button_pressed() {
  static byte button_state = HIGH;
  static byte last_reading = HIGH;
  static byte last_reading_change = 0;
  static char message[50];

  byte reading = digitalRead(BUTTON);
  unsigned long now = millis();

  // Ignore button_state changes within DEBOUNCE_DELAY milliseconds of the last
  // reading change, otherwise accept.
  if (now - last_reading_change > DEBOUNCE_DELAY) {
    if (reading == LOW && button_state == HIGH) { // button pressed down (HIGH to LOW)
      sprintf(message, "accepted, %ld ms since last\n", now - last_reading_change);
      Serial.print(message);
      button_state = reading;
      return true;
    }
    button_state = reading;
  }
  return false;
}

/* FUNCTION: loop
 * --------------
 * Handles the button response to toggle different 3D displays on the cube.
 * The first display turns all the lights on, the second expands the lights 
 * from one corner.
 */
void loop()
{
  static byte ledPattern[4][4][4];
  static int pattern = 0;
  static int layer = 0;

  // switching between patterns
  if (button_pressed()) {

    // map pattern to 0, 1, 2
    if (pattern >= 2) {
      pattern = 0;
    } else {
      ++pattern; 
    }

    for (byte x = 0; x < 4; ++x) {
      for (byte y = 0; y < 4; ++y) {
        for (byte z = 0; z < 4; ++z) {
          ledPattern[x][y][z] = 0;
        }
      }
    }
    layer = 0;
  }

  // different display options will toggle displays on the cube
  if (pattern == 0) {
    //ledPattern = display_one();
    for (byte x = 0; x < 4; x++) {
      for (byte y = 0; y < 4; y++) {
        for (byte z = 0; z < 4; z++) {
          ledPattern[x][y][z] = !ledPattern[x][y][z];
        }
      }
    }
    display(ledPattern);
  
  // Expand from Corner
  } else if (pattern == 1) {
    // set the current time/layer order
    static long last_change = 0;
    
    // illuminate this layer
    if (layer <= 4) {
      for (byte x = 0; x < layer; ++x) {
        for (byte y = 0; y < layer; ++y) {
          for (byte z = 0; z < layer; ++z) {
              ledPattern[x][y][z] = !ledPattern[x][y][z];
          }
        }
      }
    
    } else {
      for (byte x = 0; x < 4; ++x) {
        for (byte y = 0; y < 4; ++y) {
          for (byte z = 0; z < 4; ++z) {
            ledPattern[x][y][z] = 0;
          }
        }
      }
      layer = 0;
    }
    display(ledPattern);
    
    // once a time has elapsed, add another layer
    
    unsigned long now = millis();
    if (now - last_change > ELAPSE) {
      ++layer;
      last_change = now;
    }
  }

}

