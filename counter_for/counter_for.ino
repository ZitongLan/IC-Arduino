#include <EEPROM.h>
bool ResFlag = 0;

void setup() {
  // put your setup code here, to run once:
  // Serial.begin(115200);
  // Serial.println("Program Restart");
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (uint8_t i = 0; i < 50; ++i) {
    // restore process
    if (ResFlag == 0) {
      if (EEPROM.read(0) == 0)
        i = EEPROM.read(2);
      else
        i = EEPROM.read(1);
      ResFlag = 1;
    }
    LightState_Port(i);
    delay(50);

    // checkpoint begin
    if (EEPROM.read(0) == 0) {
      EEPROM.write(1, i);
      EEPROM.write(0, 1);
    } else {
      EEPROM.write(2, i);
      EEPROM.write(0, 0);
    }
  }
}

void LightState_Port(uint8_t control) {
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  uint8_t local_control = control % 5;
  switch (local_control) {
    case 0:
      digitalWrite(2, HIGH);
      break;
    case 1:
      digitalWrite(3, HIGH);
      break;
    case 2:
      digitalWrite(4, HIGH);
      break;
    case 3:
      digitalWrite(5, HIGH);
      break;
    case 4:
      digitalWrite(6, HIGH);
      break;
  }
  return;
}
