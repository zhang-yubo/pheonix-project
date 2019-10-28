//arduino sending
#include <SoftwareSerial.h>

SoftwareSerial Mo(8,7);

void setup() {
  Serial.begin(9600);
  Mo.begin(9600);
}

char in;
void loop() {
  if (Serial.available()>0)
  {
    in = Serial.read();
    Serial.println(in);
    Mo.print(in);
  }
}
