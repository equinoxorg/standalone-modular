/*
  The circuit:
 VSS (1)
 VDD (2)
 BKLIGHT Anode (15)
 BKLIGHT Cathode (16) to ground
 * LCD RS (4) pin to digital pin 12
 * LCD Enable (6) pin to digital pin 11
 * LCD D4 (11) pin to digital pin 5
 * LCD D5 (12) pin to digital pin 4
 * LCD D6 (13) pin to digital pin 3
 * LCD D7 (14) pin to digital pin 2
 * LCD R/W (5) pin to ground
 * 10K potentiometer:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 * 10K poterntiometer on pin A0
 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 
 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

 //random comment

#include <LiquidCrystal.h>
#include <EEPROM.h>
//#include <PinChangeInt.h>
//#include <PinChangeIntConfig.h>
// use the pinChange Interrupt for the keypad
// http://playground.arduino.cc/Main/PinChangeInterrupt 

/*
Depending on the state of the UI, the button press will trigger different things.
 When system is off, a button press will turn the lcd on and turn the outputs on depending on the payment/battery state.
 When system is on, a button press will turn off the lcd and turn off the outputs.
 
 Depending on the state of the UI, the keypad press will trigger different things.
 If in await payment mode or setup mode, a key press on the keypad will reset the screen with the correct number input
 If in any other mode, a keypad press should momentary turn on the lcd.
 */

// setup the EEPROM to hold the following info:
// 1. Box ID
// 2. Unlock Count
// 3. Full Unlock

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 19);

int bklight = 10;  // Setup the backlight pin
int button = 2;
int keypad_1 = 9;
int keypad_2 = 18;
int keypad_3 = 7;
int keypad_4 = 6; 
int keypad_5 = 14;
int keypad_6 = 15;
int keypad_7 = 16;

// Keypad
#define CROSS  11
#define TICK   10
#define ZERO   0
#define ONE    1
#define TWO    2
#define THREE  3
#define FOUR   4
#define FIVE   5
#define SIX    6
#define SEVEN  7
#define EIGHT  8
#define NINE   9
#define NO_BUTTON 12

// UI States
#define STATE_NORM            0
#define STATE_AWAIT_PAYMENT   1
#define STATE_LVDC            2
#define STATE_SETUP           3
#define STATE_OFF             4

uint8_t ui_state;
uint8_t current_key_press = ZERO;

// Char1 to Char5 represent the battery symbols
//Char1 --> Left side of battery
byte char1[8] = {
  0b11111,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b11111
};

//Char2 --> Line on top and Bottom
byte char2[8] = {
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111
};

//Char3	--> empty end of battery
byte char3[8] = {
  0b11100,
  0b00100,
  0b00111,
  0b00001,
  0b00001,
  0b00111,
  0b00100,
  0b11100
};

//Char4 --> Full Cell
byte char4[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

//Char5 --> full end of battery
byte char5[8] = {
  0b11100,
  0b11100,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11100,
  0b11100
};

void pciSetup(byte pin){
  // function to setup interrupt on specific pin using the pin change     
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

void key_scan(int keypad_line)
{
  digitalWrite(keypad_4, HIGH); 
  digitalWrite(keypad_5, HIGH); 
  digitalWrite(keypad_6, HIGH); 
  digitalWrite(keypad_7, HIGH);

  int button = NO_BUTTON;

  digitalWrite(keypad_4, LOW);
  if(digitalRead(keypad_line) == LOW){
    if(keypad_line == keypad_1)  button = CROSS;
    if(keypad_line == keypad_2)  button = ZERO;
    if(keypad_line == keypad_3)  button = TICK;
  }
  digitalWrite(keypad_4, HIGH);

  digitalWrite(keypad_5, LOW);
  if(digitalRead(keypad_line) == LOW){
    if(keypad_line == keypad_1)  button = SEVEN;
    if(keypad_line == keypad_2)  button = EIGHT;
    if(keypad_line == keypad_3)  button = NINE;
  }
  digitalWrite(keypad_5, HIGH);

  digitalWrite(keypad_6, LOW);
  if(digitalRead(keypad_line) == LOW){
    if(keypad_line == keypad_1)  button = FOUR;
    if(keypad_line == keypad_2)  button = FIVE;
    if(keypad_line == keypad_3)  button = SIX;
  }
  digitalWrite(keypad_6, HIGH);

  digitalWrite(keypad_7, LOW);
  if(digitalRead(keypad_line) == LOW){
    if(keypad_line == keypad_1)  button = ONE;
    if(keypad_line == keypad_2)  button = TWO;
    if(keypad_line == keypad_3)  button = THREE;
  }
  digitalWrite(keypad_7, HIGH);

  digitalWrite(keypad_4, LOW); 
  digitalWrite(keypad_5, LOW); 
  digitalWrite(keypad_6, LOW); 
  digitalWrite(keypad_7, LOW);

  current_key_press = button;  
}

ISR (PCINT0_vect){ 
  // handle key press on vertical line 1
  key_scan(keypad_1);
  lcd.clear();
  char buf[2];
  sprintf(buf, "%d", current_key_press);
  lcd.setCursor(0, 0);
  lcd.print(buf);
  delay(500);
}

ISR (PCINT1_vect){ 
  // handle key press on vertical line 2
  key_scan(keypad_2);
  lcd.clear();
  char buf[2];
  sprintf(buf, "%d", current_key_press);
  lcd.setCursor(0, 0);
  lcd.print(buf);
  delay(500);
}  

ISR (PCINT2_vect){
  // handle key press on vertical line 3
  key_scan(keypad_3);
  lcd.clear();
  char buf[2];
  sprintf(buf, "%d", current_key_press);
  lcd.setCursor(0, 0);
  lcd.print(buf);
  delay(500);
}  

void keypad_init()
{
  pinMode(keypad_1, INPUT); // set Pin as Input
  digitalWrite(keypad_1, HIGH);  // enable pullup resistor
  pciSetup(keypad_1);

  pinMode(keypad_2, INPUT);
  digitalWrite(keypad_2, HIGH); 
  pciSetup(keypad_2);

  pinMode(keypad_3, INPUT);
  digitalWrite(keypad_3, HIGH); 
  pciSetup(keypad_3);

  pinMode(keypad_4, OUTPUT);
  digitalWrite(keypad_4, LOW); 

  pinMode(keypad_5, OUTPUT);
  digitalWrite(keypad_5, LOW); 

  pinMode(keypad_6, OUTPUT);
  digitalWrite(keypad_6, LOW); 

  pinMode(keypad_7, OUTPUT);
  digitalWrite(keypad_7, LOW); 
}

void setup() {

  pinMode(bklight, OUTPUT);
  digitalWrite(bklight, HIGH);

  pinMode(button, OUTPUT);
  attachInterrupt(0, button_evt, HIGH);

  keypad_init();
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // create the battery symbols
  lcd.createChar(1, char1);
  lcd.createChar(2, char2);
  lcd.createChar(3, char3);
  lcd.createChar(4, char4);
  lcd.createChar(5, char5);
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  splash_screen(0.5);
  ui_state = 0;

  while(ui_state<5){
    reset_display();
    delay(500);
    ui_state++;
  }
}

void button_evt(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    e.quinox    ");
  lcd.setCursor(0,1);
  lcd.print("     Switch!!   ");
  delay(1000);
}

void splash_screen(uint8_t sec){
  //sec refers to the number of seconds to show the splash screen
  digitalWrite(bklight, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    e.quinox    ");
  lcd.setCursor(0,1);
  lcd.print("   izuba.box   ");
  delay(1000*sec);
  //digitalWrite(bklight, LOW);  
  lcd.clear();
}


void reset_display ()
{
  switch (ui_state)
  {
  case STATE_NORM:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    e.quinox    ");
    lcd.setCursor(0, 1);
    //write the amount of days left and update the battery
    lcd.write("          3 Days");
    update_battery(20);
    break;
  case STATE_AWAIT_PAYMENT:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write("Enter:__________");
    // Refresh screen by displaying the correct code
    //lcd_write_string_XY(6, 0, display_str);
    lcd.setCursor(0, 1);
    lcd.write("          Locked");
    //update battery status
    update_battery(20);
    break;
  case STATE_LVDC:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write("  Battery Low   ");
    //update battery status
    update_battery(30);
    break;
  case STATE_SETUP:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write("Box ID:         ");
    // display the box id by refreshing the screen
    //lcd_write_string_XY(8, 0, display_str);
    lcd.setCursor(0, 1);
    lcd.write("   Setup Mode   ");
    break;
  case STATE_OFF:
    lcd.clear();
    break;
  default:
    lcd.clear();
    break;
  }
}

void update_battery(uint8_t percent){  
  //print the correct battery symbol depending on the state of charge of the battery
  if(percent == 0){ 
    lcd.setCursor(0,1);  // set (col, row)
    lcd.write(1);
    lcd.setCursor(1,1);
    lcd.write(2);
    lcd.setCursor(2,1);
    lcd.write(2);
    lcd.setCursor(3,1);
    lcd.write(2);
    lcd.setCursor(4,1);
    lcd.write(3);
  }

  if(percent == 20){ 
    lcd.setCursor(0,1);  // set (col, row)
    lcd.write(4);
    lcd.setCursor(1,1);
    lcd.write(2);
    lcd.setCursor(2,1);
    lcd.write(2);
    lcd.setCursor(3,1);
    lcd.write(2);
    lcd.setCursor(4,1);
    lcd.write(3);
  }

  if(percent == 40){ 
    lcd.setCursor(0,1);  // set (col, row)
    lcd.write(4);
    lcd.setCursor(1,1);
    lcd.write(4);
    lcd.setCursor(2,1);
    lcd.write(2);
    lcd.setCursor(3,1);
    lcd.write(2);
    lcd.setCursor(4,1);
    lcd.write(3);
  }

  if(percent == 60){ 
    lcd.setCursor(0,1);  // set (col, row)
    lcd.write(4);
    lcd.setCursor(1,1);
    lcd.write(4);
    lcd.setCursor(2,1);
    lcd.write(4);
    lcd.setCursor(3,1);
    lcd.write(2);
    lcd.setCursor(4,1);
    lcd.write(3);
  }

  if(percent == 80){ 
    lcd.setCursor(0,1);  // set (col, row)
    lcd.write(4);
    lcd.setCursor(1,1);
    lcd.write(4);
    lcd.setCursor(2,1);
    lcd.write(4);
    lcd.setCursor(3,1);
    lcd.write(4);
    lcd.setCursor(4,1);
    lcd.write(3);
  }

  if(percent == 100){ 
    lcd.setCursor(0,1);  // set (col, row)
    lcd.write(4);
    lcd.setCursor(1,1);
    lcd.write(4);
    lcd.setCursor(2,1);
    lcd.write(4);
    lcd.setCursor(3,1);
    lcd.write(4);
    lcd.setCursor(4,1);
    lcd.write(5);
  }

}

