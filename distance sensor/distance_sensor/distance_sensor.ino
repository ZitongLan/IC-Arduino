
/*
 模块：          HC-SR04（2020款）系列超声波测距模块   
 主控芯片：      RCWL-9300系列
 功能：          GPIO模式下HC-SR04（2020款）系列超声波测距模块距离数据并串口显示
 注意：          HC-SR04（2020款）系列超声波测距模块默认GPIO模式，兼容HC-SR04。
 编写：          无锡日晨物联科技有限公司
 方案定制：      18851501095
 连线：
   -VCC          = 5V /3.3V/3-40V   //HC-SR04（2020款）可工作到40V，测试时需外接电源，注意12V以上要应用要加限流电阻。
   -TRIG/RX/SCL  = A5
   -ECHO/TX/SDA  = A4
   -GND          = GND
*/

// set up the sensor
// constant receive the sensor value
#include <EEPROM.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

float envir_temp;

float distance;
const int echo = A4;  //echO接A4脚
const int trig = A5;  //echO接A5脚

float ave_dis = 0;
uint8_t i;

bool skip_temp = 0;
// 0 is flag
// 1 is i
// 2-5 is ave_dis
// 6-9 is temperature

void setup() {
  Serial.begin(115200);  // baud rate 115200
  sensors.begin();
  pinMode(echo, INPUT);   // echo input
  pinMode(trig, OUTPUT);  // trig output
  Serial.println("Distance Measureing Begins");




  if (EEPROM.read(0) == 0)  // set recover code here.
  {
    i = EEPROM.read(1);
    EEPROM.get(2, ave_dis);
    if (EEPROM.read(19) == 1) {
      EEPROM.get(6, envir_temp);
      skip_temp = 1;
    }
  } else {
    i = EEPROM.read(10);
    EEPROM.get(11, ave_dis);
    if (EEPROM.read(19) == 1) {
      EEPROM.get(15, envir_temp);
      skip_temp = 1;
    }
  }
}


void loop() {

  if (skip_temp == 0) {
    float tic= millis();
    sensors.requestTemperatures();
    tic = millis() - tic;
    Serial.println(tic);
    tic= millis();
    envir_temp = sensors.getTempCByIndex(0);
    tic = millis() - tic;
    Serial.println(tic);
    
    Serial.print("Celsius temperature: ");
    Serial.println(envir_temp);

    if (EEPROM.read(0) == 0)  // back space 0 is ready, write to space 1
    {
      // EEPROM.write()
      EEPROM.put(9 + 6, envir_temp);
      EEPROM.write(0, 1);  // indicate that temp sensor is ready
    } else                 // back space 1 is ready, write to space 0
    {
      EEPROM.put(6, envir_temp);
      EEPROM.write(0, 0);  // indicate that temp sensor is ready
    }
    EEPROM.write(19, 1);

  } else skip_temp = 0;

  // delay(1000);  //延时100mS再次测量，延时可不要

  while (i < 10) {
    i++;
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    distance = pulseIn(echo, HIGH);  //计数接收高电平时间
    distance = distance * (331.45 + 0.61 * envir_temp) / 2 / 10000;
    ave_dis += distance;
    delay(30);                //单次测离完成后加30mS的延时再进行下次测量。防止近距离测量时，测量到上次余波，导致测量不准确。
                              //计算距离 1：声速：340M/S  2：实际距离为1/2声速距离 3：计数时钟为1US//温补公式：c=(331.45+0.61t/℃)m•s-1 (其中331.45是在0度）
    if (EEPROM.read(0) == 0)  // back space 0 is ready, write to space 1
    {
      // EEPROM.write()
      EEPROM.write(10, i);
      EEPROM.put(11, ave_dis);  // float size = 4
      EEPROM.write(0, 1);
    } else  // back space 1 is ready, write to space 0
    {
      EEPROM.write(1, i);
      EEPROM.put(2, ave_dis);  // float size = 4
      EEPROM.write(0, 0);
    }
    EEPROM.write(19, 0);
  }
  // ave_dis = mean_dis(dis_array);
  Serial.print("Distance: ");
  Serial.print(ave_dis / 10);
  Serial.println("CM");
  i = 0;
  ave_dis = 0;
  delay(1000);  //延时100mS再次测量，延时可不要
}
