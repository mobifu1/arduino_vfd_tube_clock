//********************************************************************
//********************************************************************
//********* Arduino Uno Driver for IV-18 VFD & MAX6921 ***************
//*************** Steampunk Vintage GPS Clock ************************
//**************** Author: Andreas & Tommy ***************************
//********************************************************************
//// MAX6921
//// datasheet: http://datasheets.maximintegrated.com/en/ds/MAX6921-MAX6931.pdf
#include "Wire.h"
#include "Time.h"

#define blank 7
#define load 8
#define clk 12
#define din 11
#define led 13
#define BOOST 10 //PWM-Signal for boost power supply
#define HIGH_VOLTAGE A1 //measure Pin for Anoden voltage, resistor divider 100k and 1k

//value:
boolean a;
boolean b;
boolean c;
boolean d;
boolean e;
boolean f;
boolean g;

//decimal point:
boolean dp;

//digits:
boolean digit_1;
boolean digit_2;
boolean digit_3;
boolean digit_4;
boolean digit_5;
boolean digit_6;
boolean digit_7;
boolean digit_8;
boolean digit_9;

//test with timer:
int my_hour = 0;
int my_minute = 0;
int my_second = 0;
//------------------------------------------------------------------------------
void setup() {

  Serial.begin(9600);
  pinMode(led, OUTPUT);
  pinMode(din, OUTPUT);
  pinMode(load, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(blank, OUTPUT);

  digitalWrite(blank, LOW);
  digitalWrite(clk, LOW);
  digitalWrite(load, LOW);
  digitalWrite(din, LOW);
}
//------------------------------------------------------------------------------
void loop() {

  brightness_control(2, 10);//divide factor 1-5, Pulse Width 0-40 / 10=22V, 20=41V, 30=58V, 40=75V
//  set_vfd_values("1", false, 8);
//  set_vfd_values("2", false, 7);
//  set_vfd_values("-", false, 6);
//  set_vfd_values("3", false, 5);
//  set_vfd_values("4", false, 4);
//  set_vfd_values("-", false, 3);
//  set_vfd_values("5", false, 2);
//  set_vfd_values("5", false, 1);
  
  if (my_second == 59){  //timer
    if (my_minute == 59){
      if (my_hour == 23){
        my_hour = 0;
      } else my_hour ++;
      my_minute = 0;
    }else my_minute ++;
  my_second = 0;
  } else my_second ++;  //timer end



  if (my_hour < 10){                           //writing on the tube: hour
    set_vfd_values(0, false, 8);
  }
  else if (my_hour < 20 && my_hour >= 10){
    set_vfd_values(1, false, 8);
  }
  else if (my_hour < 25 && my_hour >=20){
    set_vfd_values(2, false, 8);
  }
  int hour_2 = my_hour % 10;
  set_vfd_values(hour_2, false, 7);            // writing on the tube: hour end


  set_vfd_values(10, false, 6);                 //10 for minus 


  if (my_minute < 10){                          //writing on the tube: minute
    set_vfd_values(0, false, 5);
  }
  else if (my_minute < 20 && my_minute >= 10){
    set_vfd_values(1, false, 5);
  }
  else if (my_minute < 30 && my_minute >= 20){
    set_vfd_values(2, false, 5);
  }
  else if (my_minute < 40 && my_minute >= 30){
    set_vfd_values(3, false, 5);
  }
  else if (my_minute < 50 && my_minute >= 40){
    set_vfd_values(4, false, 5);
  }
  else if (my_minute < 60 && my_minute >= 50){
    set_vfd_values(5, false, 5);
  }
  int minute_2 = my_minute % 10;
  set_vfd_values(minute_2, false, 4);          //writing on the tube: minute end

  
  set_vfd_values(10, false, 3);                //10 for minus 


  if (my_second < 10){                         //writing on the tube: second
    set_vfd_values(0, false, 2);
  }
  else if (my_second < 20 && my_second >= 10){
    set_vfd_values(1, false, 2);
  }
  else if (my_second < 30 && my_second >= 20){
    set_vfd_values(2, false, 2);
  }
  else if (my_second < 40 && my_second >= 30){
    set_vfd_values(3, false, 2);
  }
  else if (my_second < 50 && my_second >= 40){
    set_vfd_values(4, false, 2);
  }
  else if (my_second < 60 && my_second >= 50){
    set_vfd_values(5, false, 2);
  }
  int second_2 = my_second % 10;
  set_vfd_values(second_2, false, 1);          //writing on the tube: second end
 
}

//------------------------------------------------------------------------------
void set_vfd_values(int vfd_value, boolean decimal_point, byte vfd_position) {
  byte bit_muster;
  if (vfd_value == 0)  bit_muster = 0b11101110;//0b(d,c,e,g,b,f,a,0)
  if (vfd_value == 1)  bit_muster = 0b01001000;
  if (vfd_value == 2)  bit_muster = 0b10111010;
  if (vfd_value == 3)  bit_muster = 0b11011010;
  if (vfd_value == 4)  bit_muster = 0b01011100;
  if (vfd_value == 5)  bit_muster = 0b11010110;
  if (vfd_value == 6)  bit_muster = 0b11110110;
  if (vfd_value == 7)  bit_muster = 0b01001010;
  if (vfd_value == 8)  bit_muster = 0b11111110;
  if (vfd_value == 9)  bit_muster = 0b11011110;
  if (vfd_value == 10) bit_muster = 0b00010000;  //minus

  //                               _ a
  //                             f|_|b    g:_
  //                             e|_|c .h
  //                               d
  g = false;
  if (bit_muster > 127) g = true;
  bit_muster = bit_muster << 1;
  f = false;
  if (bit_muster > 127) f = true;
  bit_muster = bit_muster << 1;
  e = false;
  if (bit_muster > 127) e = true;
  bit_muster = bit_muster << 1;
  d = false;
  if (bit_muster > 127) d = true;
  bit_muster = bit_muster << 1;
  c = false;
  if (bit_muster > 127) c = true;
  bit_muster = bit_muster << 1;
  b = false;
  if (bit_muster > 127) b = true;
  bit_muster = bit_muster << 1;
  a = false;
  if (bit_muster > 127) a = true;
  //-------------------------------------------------
  dp = false;
  if (decimal_point == true) dp = true;
  //-------------------------------------------------
  digit_1 = false;
  if (vfd_position == 1) digit_1 = true;//right position
  digit_2 = false;
  if (vfd_position == 2) digit_2 = true;
  digit_3 = false;
  if (vfd_position == 3) digit_3 = true;
  digit_4 = false;
  if (vfd_position == 4) digit_4 = true;
  digit_5 = false;
  if (vfd_position == 5) digit_5 = true;
  digit_6 = false;
  if (vfd_position == 6) digit_6 = true;
  digit_7 = false;
  if (vfd_position == 7) digit_7 = true;
  digit_8 = false;
  if (vfd_position == 8) digit_8 = true;
  digit_9 = false;
  if (vfd_position == 9) digit_9 = true;

  write_vfd();
}
//------------------------------------------------------------------------------
void write_vfd() {

  //write OUT19 with first clock signal
  digitalWrite(load, LOW);
  delay(0);
  digitalWrite(din, a); //bit 19
  serial_clock();
  digitalWrite(din, b); //bit 18
  serial_clock();
  digitalWrite(din, c); //bit 17
  serial_clock();
  digitalWrite(din, d); //bit 16
  serial_clock();
  digitalWrite(din, e); //bit 15
  serial_clock();
  digitalWrite(din, f); //bit 14
  serial_clock();
  digitalWrite(din, g); //bit 13
  serial_clock();
  digitalWrite(din, digit_2); //bit 12
  serial_clock();
  digitalWrite(din, dp); //bit 11
  serial_clock();
  digitalWrite(din, digit_4); //bit 10
  serial_clock();
  digitalWrite(din, digit_6); //bit 9
  serial_clock();
  digitalWrite(din, digit_7); //bit 8
  serial_clock();
  digitalWrite(din, digit_8); //bit 7
  serial_clock();
  digitalWrite(din, digit_5); //bit 6
  serial_clock();
  digitalWrite(din, digit_3); //bit 5
  serial_clock();
  digitalWrite(din, digit_1); //bit 4
  serial_clock();
  digitalWrite(din, digit_9); //bit 3
  serial_clock();
  digitalWrite(din, LOW); //bit 2
  serial_clock();
  digitalWrite(din, LOW); //bit 1
  serial_clock();
  digitalWrite(din, LOW); //bit 0
  serial_clock();
  digitalWrite(load, HIGH);
  delay(0);
}
//------------------------------------------------------------------------------
void serial_clock() {

  delay(0);
  digitalWrite(clk, LOW);
  delay(0);
  digitalWrite(clk, HIGH);
  delay(0);
}
//---------------Brightness Control----------------------------------------------
void brightness_control(byte divide_value, byte brightness_value) {//1-5, 0-40

  int voltage = analogRead(HIGH_VOLTAGE);
  voltage = voltage * 4.8828 / 10;
  // if (voltage < 20 || voltage > 80) {
  //Serial.print("Voltage out of Range: ");
  //  Serial.print(String(voltage));
  //  Serial.println(" V");
  //  delay (1000);
  // }

  //Divide PWM frequency to prevent inductor from singing
  int value;
  if (divide_value == 5) value = 1;
  if (divide_value == 4) value = 8;
  if (divide_value == 3) value = 64;
  if (divide_value == 2) value = 256;
  if (divide_value == 1) value = 1024;

  setPwmFrequency(BOOST, value); //1,8,64,256,1024
  analogWrite(BOOST, brightness_value); //Pulse Width 0-40 / 10=22V, 20=41V, 30=58V, 40=75V

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


