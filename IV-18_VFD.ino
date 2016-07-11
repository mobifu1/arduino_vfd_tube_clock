//********************************************************************
//********************************************************************
//Arduino Uno Driver for IV-18 VFD & MAX6921
//Steampunk vintage GPS Clock
//********************************************************************
//********************************************************************
#include "Wire.h"
#include "Time.h"

String version = "V0.1";
// Defined Pin
#define BLANK     7 //Driver: If this is HIGH, the driver sets all Outputs to LOW
#define LOAD      8 //Driver: Loads the data from shift register to output latch
#define CLK      13 //Driver: Shifts in a Bit on rising edge
#define DIN      11 //Driver: Data In (gets shiftet on CLK rising edge)
#define BOOST    10 //PWM-Signal for boost power supply
#define HIGH_VOLTAGE A1 //measure Pin for Anoden voltage

// Decimal numbers to bitmask for the 7-segments (+ decimal Point)
uint8_t number_bitmask[11] = {
  //                          a_
  0b11101110, // 0           f|_|b    g:_    0bxf,a,b,g,e,c,d,h
  0b00100100, // 1           e|_|c .h
  0b01111010, // 2             d
  0b01110110, // 3
  0b10110100, // 4
  0b11010110, // 5
  0b11011111, // 6
  0b01100100, // 7
  0b11111110, // 8
  0b11110111, // 9
  0b00000000  // 10 = off
};

// Alphabet to bitmask for the 7-segments
uint8_t alphabet_bitmask[27] = {
  //                          a_
  0b11101100, // A           f|_|b    g:_    0bxf,a,b,g,e,c,d,h
  0b10011110, // b           e|_|c .h
  0b00011010, // c             d
  0b00111110, // d
  0b11011010, // E
  0b11011000, // F
  0b11011110, // G
  0b10111100, // H
  0b00100100, // I
  0b00100110, // J
  0b10011000, // K
  0b10001010, // L
  0b11101100, // M
  0b00011100, // n
  0b00011110, // o
  0b11110000, // P
  0b11101110, // Q
  0b00011000, // R
  0b11011100, // S
  0b10011010, // t
  0b00001110, // u
  0b10101110, // V
  0b10101110, // W
  0b10111100, // x
  0b10110100, // Y
  0b01111010, // Z
  0b00000000  // space
};

// Bitmask for selecting the digits from left to right
uint8_t digit_bitmask[9] = {
  0b00000100, // left most 7-segment digit
  0b00100000,
  0b00010000,
  0b00001000,
  0b01000000,
  0b00000010,
  0b10000000,
  0b00000001, // right most 7-segment digit
  0b00000000  // signs (dot and minus)
};

// Bitmasks for the minus and the dot
uint8_t dot_bitmask   = 0b00000001;
uint8_t minus_bitmask = 0b00010000;

// global variables
byte brightness = 5; //1-5
boolean dot = true;
boolean minus = false;

byte current_digit = 0;
byte display_value[8] = {0, 0, 10, 0, 0, 10, 0, 0};
byte display_alpha[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//--------------------------------------------------------------------------
void setup() {

  Serial.begin(9600);

  pinMode(BLANK,   OUTPUT);
  pinMode(LOAD,    OUTPUT);
  pinMode(CLK,     OUTPUT);
  pinMode(DIN,     OUTPUT);

  digitalWrite(BLANK , LOW); // Disable blank

  //Test Display:
  //set values on display
  display_value[0] = 1;
  display_value[1] = 2;
  display_value[3] = 3;
  display_value[4] = 4;
  display_value[6] = 5;
  display_value[7] = 6;

}

//-------------------------------------------------------------------------------
void loop() {

  // Show Value on Tube
  multiplex();
  brightness_control(5);

}

//---------------display values on tube------------------------------------------
void multiplex() {

  uint8_t value = 0;
  boolean signs = LOW;

  if (current_digit == 9) current_digit = 0;

  if (current_digit == 8) {
    if (dot) value = (value | dot_bitmask);//bitwise or
    if (minus) value = (value | minus_bitmask);
    signs = HIGH;
  } else {
    value = number_bitmask[display_value[current_digit]];
  }

  digitalWrite(LOAD, LOW);

  my_shiftOut(DIN, CLK, digit_bitmask[current_digit]);

  digitalWrite(DIN, signs);
  digitalWrite(CLK, LOW);
  digitalWrite(CLK, HIGH);

  digitalWrite(DIN, LOW);
  digitalWrite(CLK, LOW);
  digitalWrite(CLK, HIGH);

  digitalWrite(DIN, LOW);
  digitalWrite(CLK, LOW);
  digitalWrite(CLK, HIGH);

  digitalWrite(DIN, LOW);
  digitalWrite(CLK, LOW);
  digitalWrite(CLK, HIGH);

  my_shiftOut(DIN, CLK, value);

  digitalWrite(LOAD, HIGH);

  current_digit++;

}

//-----Changed original shiftOut to use rising edge instead of falling edge------
void my_shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t val) {
  uint8_t i;

  for (i = 0; i < 8; i++) {
    digitalWrite(dataPin, !!(val & (1 << (7 - i))));
    digitalWrite(clockPin, LOW);
    digitalWrite(clockPin, HIGH);
  }
}

//---------------Brightness Control----------------------------------------------
void brightness_control(byte Brightness_Value) {//1-5

  int voltage = analogRead(HIGH_VOLTAGE);
  Serial.print("Voltage:");
  Serial.println(voltage);

  //Divide PWM frequency to prevent inductor from singing
  int value;
  if (Brightness_Value == 5) value = 1;
  if (Brightness_Value == 4) value = 8;
  if (Brightness_Value == 3) value = 64;
  if (Brightness_Value == 2) value = 256;
  if (Brightness_Value == 1) value = 1024;

  setPwmFrequency(BOOST, value); //1,8,64,256,1024
  analogWrite(BOOST, 40);

}

//--------------------- Divide PWM Frequency-------------------------------------
// http://playground.arduino.cc/Code/PwmFrequency#.UySCqdt1uVc
//Base frequencies:
//The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
//The base frequency for pins 5 and 6 is 62500 Hz.
//The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64, 256, 1024.
//The divisors available on pins 3 and 11 are: 1, 8, 32, 64, 128, 256, 1024.

void setPwmFrequency(int pin, int divisor) {

  byte mode;
  if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if (pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if (pin == 3 || pin == 11) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
