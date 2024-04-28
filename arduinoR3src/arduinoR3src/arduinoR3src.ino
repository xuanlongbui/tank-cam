#include <Arduino_JSON.h>
#include<Servo.h>           //Include The servo library for the functions used

Servo esc1;                  //Declare the ESC as a Servo Object
Servo esc2;                  //Declare the ESC as a Servo Object

#define STOP 0
#define FORWARD 1
#define BACKWARD 2
#define RIGHT 3
#define LEFT 4

#define MIN_VAL  30
#define START_VAL 150
#define START_DELAY 300 //ms

int direct = 0;
//motor A
const int PWMA = 3;
int speed1 = 0;
int val1;    // variable to read the value from the analog pin
int val2;    // variable to read the value from the analog pin
//Motor B
const int PWMB = 5;
int speed2 = 0;
//This will run only one time.
String content = "";
char character;

void forward() {
  esc1.write(START_VAL);                  // sets the servo position according to the scaled value
  esc2.write(START_VAL);                  // sets the servo position according to the scaled value

  delay(START_DELAY);          
  val1 = map(speed1, 0, 255, MIN_VAL, 180);     // scale it for use with the servo (value between 0 and 180)
  esc1.write(val1);                  // sets the servo position according to the scaled value
  
  val2 = map(speed2, 0, 255, MIN_VAL, 180);     // scale it for use with the servo (value between 0 and 180)
  esc2.write(val2);                  // sets the servo position according to the scaled value
  delay(15);                           // waits for the servo to get there
}
void backward() {
  esc1.write(0);                  // sets the servo position according to the scaled value

  esc2.write(0);                  // sets the servo position according to the scaled value
  delay(15);                           // waits for the servo to get there
}

void left() {
  esc1.write(START_VAL);                  // sets the servo position according to the scaled value
  esc2.write(0);                  // sets the servo position according to the scaled value   
  delay(START_DELAY);          

  val1 = map(speed1, 0, 255, MIN_VAL, 180);     // scale it for use with the servo (value between 0 and 180)
  esc1.write(val1);                  // sets the servo position according to the scaled value 

  delay(15);    
}
void right() {
  esc1.write(0);                  // sets the servo position according to the scaled value
  esc2.write(START_VAL);                  // sets the servo position according to the scaled value
  delay(START_DELAY);          
  val2 = map(speed2, 0, 255, MIN_VAL, 180);     // scale it for use with the servo (value between 0 and 180)
  esc2.write(val2);                  // sets the servo position according to the scaled value
  delay(15);    
}
void stop() {
  esc1.write(0);                  // sets the servo position according to the scaled value
  esc2.write(0);                  // sets the servo position according to the scaled value
  delay(500);                           // waits for the servo to get there
  esc1.write(MIN_VAL);                  // sets the servo position according to the scaled value
  esc2.write(MIN_VAL);                  // sets the servo position according to the scaled value
}
void setup() {
  Serial.begin(9600);
  while (!Serial);
  esc1.attach(PWMA);  // attaches the servo 
  esc2.attach(PWMB);  // attaches the servo
  esc1.write(MIN_VAL);                  // sets the servo position according to the scaled value
  esc2.write(MIN_VAL);                  // sets the servo position according to the scaled value
  delay(15);
}
void motorHandle() {
  switch (direct) {
    case STOP:
      stop();
      break;
    case FORWARD:
      forward();
       break;
    case BACKWARD:
      backward();
       break;
    case RIGHT:
      right();
      break;
    case LEFT:
      left();
      break;
    default:
      Serial.println("invalid direct");
  }

analogWrite(PWMA,speed1);
analogWrite(PWMB,speed2);
}
void dataHandle(String input) {
  JSONVar myObject = JSON.parse(input);

  // JSON.typeof(jsonVar) can be used to get the type of the variable
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  Serial.print("JSON.typeof(myObject) = ");
  Serial.println(JSON.typeof(myObject));  // prints: object

  if (myObject.hasOwnProperty("direct")) {
    Serial.print("myObject[\"direct\"] = ");
    Serial.println((int)myObject["direct"]);
    direct = (int)myObject["direct"];
  }
  if (myObject.hasOwnProperty("speed1")) {
    Serial.print("myObject[\"speed1\"] = ");
    Serial.println((int)myObject["speed1"]);
    speed1 = (int)myObject["speed1"];
  }
  if (myObject.hasOwnProperty("speed2")) {
    Serial.print("myObject[\"speed2\"] = ");
    Serial.println((int)myObject["speed2"]);
    speed2 = (int)myObject["speed2"];
  }

}
bool readingProcess = false;
void loop() {
  while (Serial.available()) {
    character = Serial.read();
    if (character == '{') {
      readingProcess = true;
    }
    if(readingProcess == true){
      content.concat(character);
    }
  }
    if (content != "" && character == '\n') {
      Serial.println(content);
      dataHandle(content);
      Serial.flush();
      readingProcess =false;
      content = "";
      motorHandle();
    }
}
