#include <EEPROM.h>

/******************************************** 
 Arduino MEGA 2560端的程序
 串口使用情况：
 Serial —————— computer
 Serial1 —————— UNO SoftwareSerial
 *******************************************/

uint8_t i = 0;

void setup() {
  // Serial.begin(115200);     // 初始化serial，该串口用于与计算机连接通信
  Serial1.begin(115200);  // 初始化serial1，该串口用于与设备B连接通信
  i = EEPROM.read(0);
}

// 两个字符串分别用于存储A、B两端传来的数据
String device_A_String = "123321";

void loop() {
  // 读取从计算机传入的数据，并通过Serial1发送给设备B
  // Serial1.println(device_A_String);
  while (i < 50) {
    i++;
    Serial1.println(i);
    EEPROM.write(0,i);
    delay(300);
  }
  i = 0;
}
