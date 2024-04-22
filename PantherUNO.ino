#include <Servo.h>

Servo clamp;
Servo wrist;
Servo C;
Servo D;
Servo E;
Servo rot;

long t1;

void setup() {
  Serial.begin(9600);

  clamp.attach(11);
  wrist.attach(10);
  C.attach(9);
  D.attach(6);
  E.attach(5);
  rot.attach(3);

  clamp.write(90);
  wrist.write(0);
  C.write(0);
  D.write(45);
  E.write(90);
  rot.write(90);
}

void loop() {
  if(Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    if(data.startsWith("A")) {
      clamp.write(data.substring(1).toInt());
    }

    if(data.startsWith("B")) {
      wrist.write(data.substring(1).toInt());
    }

    if(data.startsWith("C")) {
      C.write(data.substring(1).toInt());
    }
    
    if(data.startsWith("D")) {
      D.write(data.substring(1).toInt());
    }
    
    if(data.startsWith("E")) {
      E.write(data.substring(1).toInt());
    }
    if(data.startsWith("F")) {
      rot.write(data.substring(1).toInt());
    }
  }

  if(millis()-t1>2000) {
    Serial.println("PS_FRONT="+String(random(0,200)));
    Serial.println("PS_RIGHT="+String(random(0,200)));
    Serial.println("PS_LEFT="+String(random(0,200)));

    /*ws.send("PS_FRONT="+String(random(0,200)));
    ws.send("PS_RIGHT="+String(random(0,200)));
    ws.send("PS_LEFT="+String(random(0,200)));*/
    t1 = millis();
  }
}
