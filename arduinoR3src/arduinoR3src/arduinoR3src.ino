#include <Arduino_JSON.h>
#include<Servo.h>           //Include The servo library for the functions used

Servo esc1;                  //Declare the ESC as a Servo Object
Servo esc2;                  //Declare the ESC as a Servo Object

#define STOP 0
#define FORWARD 1
#define BACKWARD 2
#define RIGHT 3
#define LEFT 4

#define DEBUG 0
int stand_by_val1 = 20;
int stand_by_val2 = 20;

int init_val1 = 150;
int init_val2 = 150;

int init_time1 = 200;
int init_time2 = 200;

int direct = 0;
//motor A
const int PWMA = 3;
int speed1 = 0;
//Motor B
const int PWMB = 5;
int speed2 = 0;
//This will run only one time.
String content = "";
char character;

void forward() {
  esc1.write(init_val1);                  // sets the servo position according to the scaled value
  esc2.write(init_val2);                  // sets the servo position according to the scaled value
  delay(init_time1);          
  esc1.write(speed1);                  // sets the servo position according to the scaled value
  esc2.write(speed2);                  // sets the servo position according to the scaled value
}
void backward() {                       // waits for the servo to get there
}

void left() {
  esc1.write(init_val1);                  // sets the servo position according to the scaled value
  esc2.write(0);
  delay(init_time1);          
  esc1.write(speed1);                  // sets the servo position according to the scaled value  
}
void right() {
  esc1.write(0);                  // sets the servo position according to the scaled value
  esc2.write(init_val2);
  delay(init_time1);          
  esc2.write(speed2);                  // sets the servo position according to the scaled value    
}
void stop() {
  esc1.write(0);                  // sets the servo position according to the scaled value
  esc2.write(0);                  // sets the servo position according to the scaled value
  delay(500);                           // waits for the servo to get there
  esc1.write(stand_by_val1);                  // sets the servo position according to the scaled value
  esc2.write(stand_by_val2);                  // sets the servo position according to the scaled value
}
void setup() {
  Serial.begin(9600);
  while (!Serial);
  esc1.attach(PWMA);  // attaches the servo 
  esc2.attach(PWMB);  // attaches the servo
  esc1.write(stand_by_val1);                  // sets the servo position according to the scaled value
  esc2.write(stand_by_val2);                  // sets the servo position according to the scaled value
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

}
void dataHandle(String input) {
  JSONVar myObject = JSON.parse(input);

  // JSON.typeof(jsonVar) can be used to get the type of the variable
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    direct = STOP;
    stop();
    return;
  }
#if DEBUG
  Serial.print("JSON.typeof(myObject) = ");
  Serial.println(JSON.typeof(myObject));  // prints: object
#endif

  if (myObject.hasOwnProperty("type"))
  {
    if (strcmp(myObject["type"],"control")==0)
    {
        if (myObject.hasOwnProperty("contents"))
        {
          if (strcmp(myObject["contents"]["direct"],"forward")==0)
          {
            direct = FORWARD;
          }
          else if (strcmp(myObject["contents"]["direct"],"backward")==0)
          {
            direct = BACKWARD;
          }
          else if (strcmp(myObject["contents"]["direct"],"stop")==0)
          {
            direct = STOP;
          }
          else if (strcmp(myObject["contents"]["direct"],"right")==0)
          {
            direct = RIGHT;
          }
          else if (strcmp(myObject["contents"]["direct"],"left")==0)
          {
            direct = LEFT;
          }
          speed1 = (int)myObject["contents"]["speed1"];
          speed2 = (int)myObject["contents"]["speed2"];
#if DEBUG
          Serial.print("[\"direct\"] = ");
          Serial.println(myObject["contents"]["direct"]);
          Serial.print("[\"speed1\"] = ");
          Serial.println(myObject["contents"]["speed1"]);
          Serial.print("[\"speed2\"] = ");
          Serial.println(myObject["contents"]["speed2"]);
#endif
        }
    }
    else if (strcmp(myObject["type"],"config")==0)
    {
          if (myObject.hasOwnProperty("contents"))
        {
          stand_by_val1 = (int)myObject["contents"]["motor1"]["stand_by_value"];
          stand_by_val1 = (int)myObject["contents"]["motor2"]["stand_by_value"];
          init_val1 = (int) myObject["contents"]["motor1"]["init_val"];
          init_val2 = (int) myObject["contents"]["motor2"]["init_val"];
          init_time1 = (int)myObject["contents"]["motor1"]["init_time"];
          init_time2 = (int)myObject["contents"]["motor2"]["init_time"];
#if DEBUG

          Serial.print("[\"init value 1\"] = ");
          Serial.println(myObject["contents"]["motor1"]["init_val"]);
          Serial.print("[\"stand_by_value 1\"] = ");
          Serial.println(myObject["contents"]["motor1"]["stand_by_value"]);
          Serial.print("[\"init_time 1\"] = ");
          Serial.println(myObject["contents"]["motor1"]["init_time"]);
          Serial.print("[\"pin 1\"] = ");
          Serial.println(myObject["contents"]["motor1"]["pin"]);

          Serial.print("[\"init value 2\"] = ");
          Serial.println(myObject["contents"]["motor2"]["init_val"]);
          Serial.print("[\"stand_by_value 2\"] = ");
          Serial.println(myObject["contents"]["motor2"]["stand_by_value"]);
          Serial.print("[\"init_time 2\"] = ");
          Serial.println(myObject["contents"]["motor2"]["init_time"]);
          Serial.print("[\"pin 2\"] = ");
          Serial.println(myObject["contents"]["motor2"]["pin"]);
#endif
        }
    }
    
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
#if DEBUG
      Serial.println(content);
#endif
      dataHandle(content);
      Serial.flush();
      readingProcess =false;
      content = "";
      motorHandle();
    }
}
