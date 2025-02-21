using namespace std;

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

class pneumaticAxis {
  public:
    int location;
    digitalOutController positiveMotion;
    digitalOutController negativeMotion;

    // Proper initialization list
    pneumaticAxis(int posMotPin, int negMotPin) 
      : positiveMotion(posMotPin), negativeMotion(negMotPin) {}

    int move_positive() {
      positiveMotion.turn_on();
      delay(2000);
      positiveMotion.turn_off();
    }

    int move_negative() {
      negativeMotion.turn_on();
      delay(2000);
      negativeMotion.turn_off();
    }
};

// Correct object instantiation
pneumaticAxis lr(25, 27);  
digitalOutController pump(23);  // Assign a valid pin number
pneumaticAxis ladder(29, 31);  

void setup() {
  pump.turn_on();
  
  Serial.begin(9600); // Initialize serial communication
  pinMode(52, INPUT_PULLUP); // Set pin 7 as input with internal pull-up resistor
  // Wait for the button to be pressed

  while(digitalRead(52) == 1){
    delay(500);
  }
  Serial.println("Button pressed");
  pump.turn_off();
}

void loop() {
  
  ladder.move_positive();
  delay(5000);
  ladder.move_negative();
  delay(5000);
}
