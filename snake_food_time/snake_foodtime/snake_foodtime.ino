#include <EEPROM.h>
// #include "TimerOne.h"  // timer

#include <LCDWIKI_GUI.h>                          //Core graphics library
#include <LCDWIKI_KBV.h>                          //Hardware-specific library
LCDWIKI_KBV my_lcd(ILI9341, A3, A2, A1, A0, A4);  //model,cs,cd,wr,rd,reset

uint16_t value = 60848;           //Preload timer value (57722 for 0.5 seconds)
uint16_t time_value_food = 3035;  //Preload timer value (3035 for 4 seconds)
uint8_t first_interrupt_1 = 0;
uint8_t first_interrupt_3 = 0;
uint16_t last_time_food = 0;

int lcd_height;
int lcd_width;
const uint8_t side = 13;

int adc_key_val[5] = { 50, 200, 400, 600, 800 };
int NUM_KEYS = 5;
int adc_key_in;
int key = -1;
int oldkey = -1;

bool interrupt_state = 1;

uint8_t game_state = 1;
const uint8_t height = 11;
const uint8_t width = 20;

uint8_t base_x;
uint8_t base_y;

struct pos {
  uint8_t x;
  uint8_t y;
};

uint8_t game_array[height * width] = { 0 };
pos snake_head = { 2, 2 };
pos snake_tail = { 2, 3 };
pos food = { 3, 4 };
uint8_t snake_length = 2;
uint8_t snake_dir = 4;  // 1 left; 2 up; 3 down; 4 right;

// intermittent variables here
const uint8_t food_value = 250;
uint8_t resotre_state;



void setup() {
  // Serial.begin(115200);  // 9600 bps
  // while (!Serial) { ; }
  // delay(2000);
  // Serial.println("-----Program Restart-----  Restart times: ");

  pinMode(13, OUTPUT);  //we'll use the debug LED to output a heartbeat
  pinMode(50, OUTPUT);
  digitalWrite(50, HIGH);

  my_lcd.Init_LCD();
  // Serial.println(my_lcd.Read_ID(), HEX);
  my_lcd.Fill_Screen(0x0);
  my_lcd.Set_Rotation(1);
  lcd_height = my_lcd.Get_Display_Height();
  lcd_width = my_lcd.Get_Display_Width();

  base_x = (lcd_width - (2 + width) * side) / 2 + 1 + side;
  base_y = (lcd_height - (2 + height) * side) / 2 + 1 + side;

  if (EEPROM.read(0) == 0) {  //
    snake_dir = EEPROM[1];
    EEPROM.get(2, last_time_food);
    EEPROM.get(6, game_array);
    snake_length = 0;
  } else {
    snake_dir = EEPROM[6 + width * height];
    EEPROM.get(7 + width * height, last_time_food);
    EEPROM.get(11 + width * height, game_array);
    snake_length = 0;
  }



  uint8_t max_head = 0;
  for (int i = 0; i < width; ++i) {  // not eat food
    for (int j = 0; j < height; ++j) {
      if (game_array[i + j * width] > 0 && game_array[i + j * width] < 220) {
        snake_length++;
        if (game_array[i + j * width] > max_head) {
          snake_head.x = i;
          snake_head.y = j;
          max_head = game_array[i + j * width];
        }
      } else if (game_array[i + j * width] > 220) {
        food.x = i;
        food.y = j;
        game_array[i + j * width] = food_value;
      }
    }
  }
  // Serial.println(max_head);
  // Serial.println(snake_length);
  // Serial.println(snake_dir);
  init_game();

  // Timer1.initialize(500000);  // move per second
  // Timer1.attachInterrupt(Flash);

  noInterrupts();  // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = value;
  TCCR1B |= (1 << CS10) | (1 << CS12);  // 1024 prescaler
  TIMSK1 |= (1 << TOIE1);               // enable timer overflow interrupt ISR

  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3 = time_value_food;
  TCCR3B |= (1 << CS30) | (1 << CS32);  // 1024 prescaler
  TIMSK3 |= (1 << TOIE3);               // enable timer overflow interrupt ISR
  interrupts();                         // enable all interrupts
}

// constantly judge the dir of snake
void loop() {
  adc_key_in = analogRead(A15);  // read the value from the sensor
  digitalWrite(13, LOW);
  key = get_key(adc_key_in);  // convert into key press
  if (key != oldkey)          // if keypress is detected
  {
    delay(50);                     // wait for debounce time
    adc_key_in = analogRead(A15);  // read the value from the sensor
    key = get_key(adc_key_in);     // convert into key press
    if (key != oldkey) {
      oldkey = key;
      if (key >= 0) {
        digitalWrite(13, HIGH);
        switch (key) {
          case 0:  // s1
            if (snake_head.x - 1 + snake_head.y * width > -1)
              if (game_array[snake_head.x - 1 + snake_head.y * width] < 1)
                snake_dir = 1;
            break;
          case 1:  // s2
            if (snake_head.x + (snake_head.y - 1) * width > -1)
              if (game_array[snake_head.x + (snake_head.y - 1) * width] < 1)
                snake_dir = 3;
            break;
          case 2:  // s3
            if (snake_head.x + (snake_head.y + 1) * width > -1)
              if (game_array[snake_head.x + (snake_head.y + 1) * width] < 1)
                snake_dir = 2;
            break;
          case 3:  // s4
            if (snake_head.x + 1 + snake_head.y * width > -1)
              if (game_array[snake_head.x + 1 + snake_head.y * width] < 1)
                snake_dir = 4;
            break;
          case 4:  // s5
            if (interrupt_state) {
              interrupt_state = 0;
              noInterrupts();
            }
            else if(interrupt_state == 0)
            {
              interrupt_state = 1;
              interrupts();
            }
            break;
        }
      }
    }
  }
  delay(50);
}

// constantly update the snake state
ISR(TIMER1_OVF_vect) {
  if (first_interrupt_1 == 0) {
    first_interrupt_1++;
    return;
  }
  TCNT1 = value;  // preload timer
  // Serial.println("Begin flash");
  switch (snake_dir) {
    case 1:
      snake_head.x--;
      break;
    case 2:
      snake_head.y++;
      break;
    case 3:
      snake_head.y--;
      break;
    case 4:
      snake_head.x++;
      break;
  }
  game_state = judge_snake();
  if (game_state == 0) {
    snake_head = { 2, 2 };
    snake_tail = { 2, 3 };
    snake_length = 2;
    snake_dir = 4;
    food = { 3, 4 };
    for (uint8_t k = 0; k < height * width; ++k)
      game_array[k] = 0;

    game_array[food.x + food.y * width] = food_value;
    game_array[snake_tail.x + snake_tail.y * width] = 1;
    game_array[snake_head.x + snake_head.y * width] = 2;
    init_game();
    return;
  } else if (game_state == 2) {
    do {
      food.x = random(width);
      food.y = random(height);
    } while (game_array[food.x + food.y * width] > 0);
    game_array[food.x + food.y * width] = food_value;
    my_lcd.Set_Draw_color(0, 0, 255);  // food
    my_lcd.Fill_Rectangle(base_x + food.x * side, base_y + food.y * side, base_x + food.x * side + side - 1, base_y + food.y * side + side - 1);
  }

  // Serial.println("Begin save");
  save_game();
  // Serial.println("End flash");
}

ISR(TIMER3_OVF_vect) {
  if (first_interrupt_3 == 0) {
    first_interrupt_3++;
    TCNT3 = last_time_food;
    // Serial.print("Last time food: ");
    // Serial.println(last_time_food);
    // Serial.println("First into interrupt, TCNT3 = ");
    // Serial.println(TCNT3);
    return;
  }
  TCNT3 = time_value_food;
  game_array[food.x + food.y * width] = 0;
  my_lcd.Set_Draw_color(255, 255, 255);  // food
  my_lcd.Fill_Rectangle(base_x + food.x * side, base_y + food.y * side, base_x + food.x * side + side - 1, base_y + food.y * side + side - 1);
  do {
    food.x = random(width);
    food.y = random(height);
  } while (game_array[food.x + food.y * width] > 0);
  game_array[food.x + food.y * width] = food_value;
  my_lcd.Set_Draw_color(0, 0, 255);  // food
  my_lcd.Fill_Rectangle(base_x + food.x * side, base_y + food.y * side, base_x + food.x * side + side - 1, base_y + food.y * side + side - 1);
}


uint8_t judge_snake() {
  uint8_t state = 1;
  if (snake_head.x < 0 || snake_head.x > width - 1 || snake_head.y < 0 || snake_head.y > height - 1) {
    state = 0;                                                    // game over;
  } else if (snake_head.x == food.x && snake_head.y == food.y) {  // eat the food
    snake_length++;
    game_array[snake_head.x + snake_head.y * width] = snake_length;
    state = 2;
    TCNT3 = time_value_food;                                         // re-initialize food timer
  } else if (game_array[snake_head.x + snake_head.y * width] > 1) {  // hit the body
    state = 0;
  } else {
    for (int i = 0; i < width; ++i) {  // not eat food
      for (int j = 0; j < height; ++j) {
        if (game_array[i + j * width] > 0) {
          if (game_array[i + j * width] == 1) {
            my_lcd.Set_Draw_color(255, 255, 255);  // white
            my_lcd.Fill_Rectangle(base_x + i * side, base_y + j * side, base_x + i * side + side - 1, base_y + j * side + side - 1);
          }
          game_array[i + j * width]--;
        }
      }
    }
    game_array[snake_head.x + snake_head.y * width] = snake_length;
  }
  my_lcd.Set_Draw_color(0, 255, 0);  // red
  my_lcd.Fill_Rectangle(base_x + snake_head.x * side, base_y + snake_head.y * side, base_x + snake_head.x * side + side - 1, base_y + snake_head.y * side + side - 1);
  return state;
}

void init_game() {

  my_lcd.Set_Draw_color(255, 0, 0);  // red
  my_lcd.Fill_Rectangle((lcd_width - (2 + width) * side) / 2 + 1, (lcd_height - (2 + height) * side) / 2 + 1, (lcd_width + (2 + width) * side) / 2, (lcd_height + (2 + height) * side) / 2);
  my_lcd.Set_Draw_color(255, 255, 255);  // black
  my_lcd.Fill_Rectangle((lcd_width - (width)*side) / 2 + 1, (lcd_height - (height)*side) / 2 + 1, (lcd_width + (width)*side) / 2, (lcd_height + (height)*side) / 2);

  for (int j = height - 1; j >= 0; --j) {
    for (int i = 0; i < width; ++i) {
      if (game_array[i + j * width] > 0 && game_array[i + j * width] < food_value) {  // body
        my_lcd.Set_Draw_color(0, 255, 0);
        my_lcd.Fill_Rectangle(base_x + i * side, base_y + j * side, base_x + i * side + side - 1, base_y + j * side + side - 1);
      } else if (game_array[i + j * width] == 0) {  // empty
      } else {                                      // food
        my_lcd.Set_Draw_color(0, 0, 255);
        my_lcd.Fill_Rectangle(base_x + i * side, base_y + j * side, base_x + i * side + side - 1, base_y + j * side + side - 1);
      }
    }
  }
}

void save_game() {
  if (EEPROM.read(0) == 1)  // back the first place
  {
    EEPROM.write(1, snake_dir);
    EEPROM.put(2, TCNT3);

    for (int i = 0; i < width; ++i)
      for (int j = 0; j < height; ++j)
        EEPROM.update(6 + i + j * width, game_array[i + j * width]);
    // EEPROM.put(2, game_array);
    EEPROM.write(0, 0);
    // Serial.print("Snake length:");
    // Serial.println(snake_length);
  } else  // back the second place
  {
    EEPROM.write(6 + width * height, snake_dir);
    EEPROM.put(7 + width * height, TCNT3);

    for (int i = 0; i < width; ++i)
      for (int j = 0; j < height; ++j)
        EEPROM.update(11 + width * height + i + j * width, game_array[i + j * width]);
    EEPROM.write(0, 1);
    // Serial.print("Snake length:");
    // Serial.println(snake_length);
  }
}

// Convert ADC value to key number
int get_key(unsigned int input) {
  int k;
  for (k = 0; k < NUM_KEYS; k++) {
    if (input < adc_key_val[k]) {
      return k;
    }
  }
  if (k >= NUM_KEYS) k = -1;  // No valid key pressed
  return k;
}