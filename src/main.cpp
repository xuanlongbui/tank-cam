#include "Arduino.h"

void setup() {

  Serial.begin(96a);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("{\"direct\":1, \"speed1\":100, \"speed2\":100}");

}

void loop() {
  // Do nothing. Everything is done in another task by the web server
  delay(1000);
  Serial.println("{\"direct\":1, \"speed1\":100, \"speed2\":100}");
  delay(1000);
  Serial.println("{\"direct\":0, \"speed1\":200, \"speed2\":200}");
}
