#include <Arduino.h>
#include <Servo.h>
#include <FastLED.h>

using namespace std;

// FastLED definitions for the WS2812B LED strip
#define LED_PIN     3
#define NUM_LEDS    150
#define BRIGHTNESS  10
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

//-----------------------------------------------------
// digitalOutController controls a digital output pin.
class digitalOutController {
  public:
    int pin;
    bool status;

    digitalOutController(int pin_num) {
      pin = pin_num;
      status = false;
      pinMode(pin, OUTPUT);
    }

    int toggle() {
      if(status) {
        digitalWrite(pin, LOW);
        status = false;
      } else {
        digitalWrite(pin, HIGH);
        status = true;
      }
      return 0;
    }

    int turn_on() {
      digitalWrite(pin, HIGH);
      status = true;
      return 0;
    }

    int turn_off() {
      digitalWrite(pin, LOW);
      status = false;
      return 0;
    }
};

//-----------------------------------------------------
// pneumaticAxis uses two digital outputs for motion.
class pneumaticAxis {
  public:
    digitalOutController positiveMotion;
    digitalOutController negativeMotion;

    pneumaticAxis(int posMotPin, int negMotPin) 
      : positiveMotion(posMotPin), negativeMotion(negMotPin) {}

    void move_positive() {
      positiveMotion.turn_on();
      delay(2000);
      positiveMotion.turn_off();
    }

    void move_negative() {
      negativeMotion.turn_on();
      delay(2000);
      negativeMotion.turn_off();
    }
};

// Instantiate objects
pneumaticAxis lr(25, 27);  
digitalOutController pump(23);  
pneumaticAxis ladder(29, 31);  

//-----------------------------------------------------
// Servo objects for the owl wings.
Servo servoOwlWingLeft;
Servo servoOwlWingRight;

// Helper function to operate a servo by sweeping it.
void operateServo(Servo &s, int speed, int duration) {
  unsigned long startTime = millis();
  int pos = 0;
  int increment = 1;
  while (millis() - startTime < (unsigned long)duration) {
    s.write(pos);
    delay(speed);
    pos += increment;
    if (pos >= 180 || pos <= 0) {
      increment = -increment; // reverse direction at limits
    }
  }
}

//-----------------------------------------------------
// Update the WS2812B LED strip with a solid color.
// The 'color' parameter is case-insensitive and can be red, green,
// blue, yellow, white, or off (turns the strip black).
void setLightColor(String color) {
  color.toLowerCase();
  if(color == "red") {
    fill_solid(leds, NUM_LEDS, CRGB::Red);
  } else if(color == "green") {
    fill_solid(leds, NUM_LEDS, CRGB::Green);
  } else if(color == "blue") {
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
  } else if(color == "yellow") {
    fill_solid(leds, NUM_LEDS, CRGB::Yellow);
  } else if(color == "white") {
    fill_solid(leds, NUM_LEDS, CRGB::White);
  } else if(color == "off") {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  } else {
    // Unknown color: turn off the strip.
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  FastLED.show();
}

//-----------------------------------------------------
// Process a serial command in the format:
// <SERVO, SERVO_NAME, SPEED, TIME (ms)>
// <AXIS, AXIS_NAME, box>
// <LIGHT, COLOR>
void processCommand(String cmd) {
  cmd.trim();
  if(cmd.charAt(0) != '<' || cmd.charAt(cmd.length()-1) != '>') {
    Serial.println("Invalid command format");
    return;
  }
  // Remove the angle brackets.
  cmd = cmd.substring(1, cmd.length()-1);
  
  // Split the command by commas.
  const int maxTokens = 5;
  String tokens[maxTokens];
  int tokenIndex = 0;
  int startIndex = 0;
  int commaIndex = cmd.indexOf(',');
  while(commaIndex != -1 && tokenIndex < maxTokens) {
    tokens[tokenIndex++] = cmd.substring(startIndex, commaIndex);
    startIndex = commaIndex + 1;
    commaIndex = cmd.indexOf(',', startIndex);
  }
  if(tokenIndex < maxTokens) {
    tokens[tokenIndex++] = cmd.substring(startIndex);
  }
  
  // Trim whitespace from tokens.
  for (int i = 0; i < tokenIndex; i++){
    tokens[i].trim();
  }
  
  if(tokenIndex < 2) {
    Serial.println("Incomplete command");
    return;
  }
  
  String commandType = tokens[0];
  commandType.toUpperCase();
  
  if(commandType == "SERVO") {
    if(tokenIndex < 4) {
      Serial.println("Incomplete SERVO command");
      return;
    }
    String servoName = tokens[1];
    int speed = tokens[2].toInt();
    int duration = tokens[3].toInt();
    
    if(servoName == "owl_wing_left") {
      operateServo(servoOwlWingLeft, speed, duration);
      Serial.println("Operating owl_wing_left servo");
    } else if(servoName == "owl_wing_right") {
      operateServo(servoOwlWingRight, speed, duration);
      Serial.println("Operating owl_wing_right servo");
    } else {
      Serial.println("Unknown servo name");
    }
    
  } else if(commandType == "AXIS") {
    if(tokenIndex < 3) {
      Serial.println("Incomplete AXIS command");
      return;
    }
    String axisName = tokens[1];
    String box = tokens[2];
    // For "Lr": "left" moves negative; "right" moves positive.
    // For "ladder": "top" moves positive; "bottom" moves negative.
    if(axisName == "Lr") {
      if(box.equalsIgnoreCase("left")) {
        lr.move_negative();
        Serial.println("Moving Lr axis left");
      } else if(box.equalsIgnoreCase("right")) {
        lr.move_positive();
        Serial.println("Moving Lr axis right");
      } else {
        Serial.println("Unknown box option for Lr axis");
      }
    } else if(axisName == "ladder") {
      if(box.equalsIgnoreCase("top")) {
        ladder.move_positive();
        Serial.println("Moving ladder axis top");
      } else if(box.equalsIgnoreCase("bottom")) {
        ladder.move_negative();
        Serial.println("Moving ladder axis bottom");
      } else {
        Serial.println("Unknown box option for ladder axis");
      }
    } else {
      Serial.println("Unknown axis name");
    }
    
  } else if(commandType == "LIGHT") {
    if(tokenIndex < 2) {
      Serial.println("Incomplete LIGHT command");
      return;
    }
    String color = tokens[1];
    setLightColor(color);
    Serial.print("Setting light color to ");
    Serial.println(color);
    
  } else {
    Serial.println("Unknown command type");
  }
}

//-----------------------------------------------------
void setup() {
  // Start FastLED (WS2812B LED strip) after a safety delay.
  delay(3000);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  
  // Turn on the pump during initialization.
  pump.turn_on();
  
  Serial.begin(9600);
  pinMode(52, INPUT_PULLUP);  // Button input on pin 52
  
  // Attach servos to their pins.
  servoOwlWingLeft.attach(11);
  servoOwlWingRight.attach(12);
  
  // Wait for the button press.
  while(digitalRead(52) == HIGH) {
    delay(500);
  }
  Serial.println("Button pressed");
  pump.turn_off();
}

//-----------------------------------------------------
void loop() {
  if(Serial.available() > 0) {
    String inputCommand = Serial.readStringUntil('\n');
    processCommand(inputCommand);
  }
}
