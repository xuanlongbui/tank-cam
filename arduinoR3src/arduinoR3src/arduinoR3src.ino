#include <Arduino_JSON.h>

#define STOP 0
#define FORWARD 1
#define BACKWARD 2
#define RIGHT 3
#define LEFT 4
int direct = 0;
//motor A
const int APin1 = 2;
const int PWMA = 3;
const int APin2 = 4;
int speed1 = 0;
//Motor B
const int BPin1 = 5;
const int PWMB = 6;
const int BPin2 = 7;
int speed2 = 0;
//This will run only one time.
String content = "";
char character;

void forward() {
  digitalWrite(APin1, 0);
  digitalWrite(APin2, 1);
  digitalWrite(BPin1, 0);
  digitalWrite(BPin2, 1);
}
void backward() {
  digitalWrite(APin1, 1);
    digitalWrite(APin2, 0);
  digitalWrite(BPin1, 1);
  digitalWrite(BPin2, 0);
}
void left() {
  digitalWrite(APin1, 1);
    digitalWrite(APin2, 0);
  digitalWrite(BPin1, 0);
  digitalWrite(BPin2, 1);
}
void right() {
  digitalWrite(APin1, 0);
    digitalWrite(APin2, 1);
  digitalWrite(BPin1, 1);
  digitalWrite(BPin2, 0);
}
void stop() {
  digitalWrite(APin1, 0);
    digitalWrite(APin2, 0);
  digitalWrite(BPin1, 0);
  digitalWrite(BPin2, 0);
}
void setup() {
  Serial.begin(9600);
  while (!Serial);
  //Set pins as outputs
  pinMode(APin1, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(APin2, OUTPUT);
  pinMode(BPin1, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BPin2, OUTPUT);
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
