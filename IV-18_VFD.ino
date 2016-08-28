//********************************************************************
//********************************************************************
//********* Arduino Uno Driver for IV-18 VFD & MAX6921 ***************
//*************** Steampunk Vintage GPS Clock ************************
//**************** Author: Andreas & Tommy ***************************
//********************************************************************
//// MAX6921
//// datasheet: http://datasheets.maximintegrated.com/en/ds/MAX6921-MAX6931.pdf
#include "Wire.h"
//#include "Time.h"
#include "TimeLib.h"
#include "DCF77.h"

#define DCF_PIN 2           // Connection pin to DCF 77 device
#define DCF_INTERRUPT 0    // Interrupt number associated with pin
#define SWITCH_0 0

time_t time;
DCF77 DCF = DCF77(DCF_PIN, DCF_INTERRUPT);

#define blank 7
#define lload 8
#define clk 12
#define din 11
#define led 13
#define BOOST 10 //PWM-Signal for boost power supply

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

//Time:
int hour_int = 0;
int minute_int = 0;
int second_int = 0;
int day_int = 1;
int month_int = 1;
int year_int = 1970;
const char *Month[12] = {"JANUAR", "FEBRUAR", "MAERZ", "APRIL", "MAI", "JUNI", "JULI", "AUGUST", "SEPTEMBER", "OKTOBER", "NOVEMBER", "DEZEMBER"};
boolean sync_indicator = false;
boolean sync_dcf77 = false;

long system_clock = 0;
//------------------------------------------------------------------------------
void setup() {

  DCF.Start();

  Serial.begin(9600);
  pinMode(led, OUTPUT);
  pinMode(din, OUTPUT);
  pinMode(lload, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(blank, OUTPUT);

  digitalWrite(blank, LOW);
  digitalWrite(clk, LOW);
  digitalWrite(lload, LOW);
  digitalWrite(din, LOW);

  pinMode(SWITCH_0, INPUT);
  brightness_control(2, 10);//divide factor 1-5, Pulse Width 0-40 / 10=22V, 20=41V, 30=58V, 40=75V

  set_vfd_blink_text("DCF V1.0", 250, 15);
  set_vfd_scroll_text("SEARCHING RADIO SIGNAL........", 250);

}
//------------------------------------------------------------------------------
void loop() {

  time_t DCFtime = DCF.getTime(); // Check if new DCF77 time is available

  if (second_int == 59 && minute_int == 59) sync_dcf77 = false; // force to resync > left point on vfd tube

  if (DCFtime == 0) {

    if (sync_dcf77 == false) {
      boolean val = digitalRead(DCF_PIN);
      if (val == LOW) sync_indicator = false;
      if (val == HIGH) sync_indicator = true;
    }

    if (system_clock + 1000 < millis()) {
      system_clock = millis();
      second_int++;
    }

    if (second_int == 60) { //time counter
      second_int = 0;
      minute_int ++;

      if (minute_int == 60) {
        minute_int = 0;
        hour_int ++;

        if (hour_int == 24) {
          hour_int = 0;
        }
      }
    }
  }

  if (DCFtime != 0) {

    setTime(DCFtime);
    hour_int = hour();
    minute_int = minute();
    second_int = second();
    day_int = day();
    month_int = month();
    year_int = year();
    sync_dcf77 = true;
    sync_indicator = false;
  }

  //----------------------------------
  String hour_string = String(hour_int);
  String minute_string = String(minute_int);
  String second_string = String(second_int);
  String day_string = String(day_int);
  String month_string = Month[month_int - 1];
  String year_string = String(year_int);

  if (day_string.length() == 1)  day_string = "0" + day_string;              //adding a 0 if day is 0-9
  if (hour_string.length() == 1)  hour_string = "0" + hour_string;           //adding a 0 if hour is 0-9
  if (minute_string.length() == 1)  minute_string = "0" + minute_string;     //adding a 0 if minute is 0-9
  if (second_string.length() == 1)  second_string = "0" + second_string;     //adding a 0 if second is 0-9

  String date_string = day_string + " " + month_string + " " + year_string;
  String time_string = hour_string + "-" + minute_string + "-" + second_string;

  if (digitalRead(SWITCH_0) == LOW) {
    set_vfd_scroll_date(date_string, 200);
  } else set_vfd_text(time_string, sync_indicator);   //must be a string of length 8
}

//------------------------------------------------------------------------------
void set_vfd_scroll_text(String text, int delay_time) {

  long system_time = 0;
  int i = 0; // counter
  String scroll_text = "        " + text + "        "; // 8x space chart
  int len = scroll_text.length();

  while (i < len - 8) {

    if (system_time + delay_time < millis()) {
      system_time = millis();
      i++;
    }
    String value = scroll_text.substring(i, i + 8);
    set_vfd_text(value, false);    //must be a string of length 8
  }
}

//------------------------------------------------------------------------------
void set_vfd_blink_text(String text, int blink_frequency, int often) {

  long system_time = 0;
  int i = 1; // counter

  while (i < often) { // blinking how often

    if (system_time + blink_frequency < millis()) {
      system_time = millis();
      i++;
    }
    if (0 == i % 2) set_vfd_text(text + "        ", false); //must be a string of length 8
    if (1 == i % 2) set_vfd_text("        ", false);    //must be a string of length 8
  }
}

//------------------------------------------------------------------------------
void set_vfd_text(String string, boolean left_point) {

  set_vfd_values(string.substring(0, 1), false, 8);
  set_vfd_values(string.substring(1, 2), false, 7);
  set_vfd_values(string.substring(2, 3), false, 6);
  set_vfd_values(string.substring(3, 4), false, 5);
  set_vfd_values(string.substring(4, 5), false, 4);
  set_vfd_values(string.substring(5, 6), false, 3);
  set_vfd_values(string.substring(6, 7), false, 2);
  set_vfd_values(string.substring(7, 8), false, 1);
  set_vfd_values(" ", left_point, 9);
}

//------------------------------------------------------------------------------
void set_vfd_scroll_date(String string, int delay_time) {  //function for floating point for date

  long system_time = 0;
  int i = 0; // counter
  String scroll_date = "        " + string + "        "; // 8x space chart
  int len = scroll_date.length();

  while (i < len - 8) {

    if (system_time + delay_time < millis()) {
      system_time = millis();
      i++;
    }
    String date = scroll_date.substring(i, i + 8);

    if (i == 9) set_vfd_values(date.substring(0, 1), true, 8);
    else set_vfd_values(date.substring(0, 1), false, 8);
    if (i == 8) set_vfd_values(date.substring(1, 2), true, 7);
    else set_vfd_values(date.substring(1, 2), false, 7);
    if (i == 7) set_vfd_values(date.substring(2, 3), true, 6);
    else set_vfd_values(date.substring(2, 3), false, 6);
    if (i == 6) set_vfd_values(date.substring(3, 4), true, 5);
    else set_vfd_values(date.substring(3, 4), false, 5);
    if (i == 5) set_vfd_values(date.substring(4, 5), true, 4);
    else set_vfd_values(date.substring(4, 5), false, 4);
    if (i == 4) set_vfd_values(date.substring(5, 6), true, 3);
    else set_vfd_values(date.substring(5, 6), false, 3);
    if (i == 3) set_vfd_values(date.substring(6, 7), true, 2);
    else set_vfd_values(date.substring(6, 7), false, 2);
    if (i == 2) set_vfd_values(date.substring(7, 8), true, 1);
    else set_vfd_values(date.substring(7, 8), false, 1);
  }
}

//-------------------------------------------------------------------------------
void set_vfd_values(String vfd_value, boolean decimal_point, byte vfd_position) {

  byte bit_muster;
  if (vfd_value == "." || vfd_value == ",") {
    bit_muster = 0b00000000; //empty digit
    decimal_point = true;
  }
  if (vfd_value == "0")  bit_muster = 0b11101110; //0b(d,c,e,g,b,f,a,0)
  if (vfd_value == "1")  bit_muster = 0b01001000;
  if (vfd_value == "2")  bit_muster = 0b10111010;
  if (vfd_value == "3")  bit_muster = 0b11011010;
  if (vfd_value == "4")  bit_muster = 0b01011100;
  if (vfd_value == "5")  bit_muster = 0b11010110;
  if (vfd_value == "6")  bit_muster = 0b11110110;
  if (vfd_value == "7")  bit_muster = 0b01001010;
  if (vfd_value == "8")  bit_muster = 0b11111110;
  if (vfd_value == "9")  bit_muster = 0b11011110;
  if (vfd_value == " ")  bit_muster = 0b00000000; //empty digit
  if (vfd_value == "-")  bit_muster = 0b00010000;
  if (vfd_value == "A" || vfd_value == "a")  bit_muster = 0b01111110;
  if (vfd_value == "B" || vfd_value == "b")  bit_muster = 0b11110100;
  if (vfd_value == "C" || vfd_value == "c")  bit_muster = 0b10100110;
  if (vfd_value == "D" || vfd_value == "d")  bit_muster = 0b11111000;
  if (vfd_value == "E" || vfd_value == "e")  bit_muster = 0b10110110;
  if (vfd_value == "F" || vfd_value == "f")  bit_muster = 0b00110110;
  if (vfd_value == "G" || vfd_value == "g")  bit_muster = 0b11100110;
  if (vfd_value == "H" || vfd_value == "h")  bit_muster = 0b01111100;
  if (vfd_value == "I" || vfd_value == "i")  bit_muster = 0b01001000;
  if (vfd_value == "J" || vfd_value == "j")  bit_muster = 0b11101000;
  if (vfd_value == "K" || vfd_value == "k")  bit_muster = 0b00110100;
  if (vfd_value == "L" || vfd_value == "l")  bit_muster = 0b10100100;
  if (vfd_value == "M" || vfd_value == "m")  bit_muster = 0b01100010;
  if (vfd_value == "N" || vfd_value == "n")  bit_muster = 0b01101110;
  if (vfd_value == "O" || vfd_value == "o")  bit_muster = 0b11101110;
  if (vfd_value == "P" || vfd_value == "p")  bit_muster = 0b00111110;
  if (vfd_value == "Q" || vfd_value == "q")  bit_muster = 0b01011110;
  if (vfd_value == "R" || vfd_value == "r")  bit_muster = 0b00110000;
  if (vfd_value == "S" || vfd_value == "s")  bit_muster = 0b11010110;
  if (vfd_value == "T" || vfd_value == "t")  bit_muster = 0b10110100;
  if (vfd_value == "U" || vfd_value == "u")  bit_muster = 0b11101100;
  if (vfd_value == "V" || vfd_value == "v")  bit_muster = 0b11100000;
  if (vfd_value == "W" || vfd_value == "w")  bit_muster = 0b10001100;
  if (vfd_value == "X" || vfd_value == "x")  bit_muster = 0b01111100;
  if (vfd_value == "Y" || vfd_value == "y")  bit_muster = 0b11011100;
  if (vfd_value == "Z" || vfd_value == "z")  bit_muster = 0b10111010; //0b(d,c,e,g,b,f,a,0)

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
  digitalWrite(lload, LOW);
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
  digitalWrite(lload, HIGH);
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
