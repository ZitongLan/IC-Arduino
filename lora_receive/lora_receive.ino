#include <EEPROM.h>

/******************************************** 
 Arduino MEGA 2560端的程序
 串口使用情况：
 Serial —————— computer
 Serial1 —————— UNO SoftwareSerial
 *******************************************/

const uint16_t value = 34285;
uint8_t i = 0;


void setup() {
  Serial.begin(115200);   // 初始化serial，该串口用于与计算机连接通信
  Serial1.begin(115200);  // 初始化serial1，该串口用于与设备B连接通信

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = value;
  TCCR1B |= (1 << CS12);   // 64 prescaler
  TIMSK1 |= (1 << TOIE1);  // enable timer overflow interrupt ISR

  i = EEPROM.read(0);
  Serial1.println("trigger-restart");
}


// 两个字符串分别用于存储A、B两端传来的数据
String device_A_String = "";
String device_B_String = "";

ISR(TIMER1_OVF_vect)  // interrupt service routine for overflow
{
  TCNT1 = value;
  i++;
  // Serial1.println(i);
  if (i > 49)
    i = 0;
  EEPROM.write(0, i);
}
uint8_t RxState = 0;
uint8_t FirRx = 0;
void loop() {
  // 读取从设备B传入的数据，并在串口监视器中显示
  if (FirRx == 0 && EEPROM.read(10) > 0) {
    if (EEPROM.read(10) == 1) {
      Serial1.println("Re-send");
    } else if (EEPROM.read(10) == 2) {
      Serial1.println("Received");
    }
    EEPROM.write(10, 0);
    FirRx = 1;
  }
  if (Serial1.available() > 0) {
    if (RxState == 0) {
      // Serial.println("Start Receiving Data");
      RxState++;
      EEPROM.write(10, RxState);
    }
    if (Serial1.peek() != '\n') {
      device_B_String += char(Serial1.read());
    } else {
      Serial1.read();
      RxState++;
      EEPROM.write(10, RxState);
      Serial1.println("Received");
      RxState = 0;

      EEPROM.write(10, RxState);
      Serial.print("Receive:");
      Serial.println(device_B_String);
      device_B_String = "";
    }
  }
}
