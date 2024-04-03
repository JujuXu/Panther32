#include <Servo.h>

Servo Michel;

long t1;

int a1 = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Michel.attach(9);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');

    /*Serial.print("Données reçues : ");
    Serial.println(data);*/

    if(data.startsWith("B")) {
      data = data.substring(1);

      a1 = data.toInt();

      Michel.write(a1);
    }
  }

  if(millis()-t1>2000) {
    Serial.println("capteur");
    t1 = millis();
  }
}
