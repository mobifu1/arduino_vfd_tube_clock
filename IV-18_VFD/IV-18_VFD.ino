//********************************************************************
//********************************************************************
//********* Arduino Uno Driver for IV-18 VFD & MAX6921 ***************
//*************** Steampunk Vintage GPS Clock ************************
//**************** Author: Andreas & Tommy ***************************
//********************************************************************
//// MAX6921
//// datasheet: http://datasheets.maximintegrated.com/en/ds/MAX6921-MAX6931.pdf
#include "Wire.h"
#include <Time.h>
#include <Timezone.h>
TimeChangeRule CEST = {"", Last, Sun, Mar, 2, 120};
TimeChangeRule CET = {"", Last, Sun, Oct, 3, 60};
Timezone CE(CEST, CET);
TimeChangeRule *tcr;

int is_vfd_on = 1; // day=0 / night=1

const char *Month[12] = {"JANUAR", "FEBRUAR", "MAERZ", "APRIL", "MAI", "JUNI", "JULI", "AUGUST", "SEPTEMBER", "OKTOBER", "NOVEMBER", "DEZEMBER"};
String date_string = "";
String time_string = "";

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
int hour_int = 0; //UTC
int minute_int = 0;
int second_int = 0;
int day_int = 0;
int month_int = 0;
int year_int = 0;

boolean ntp_sync_indicator = false;
boolean ntp_request_is_send = false;
String data = "";

//------------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(din, OUTPUT);
  pinMode(lload, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(blank, OUTPUT);
  pinMode(BOOST, OUTPUT);

  digitalWrite(blank, LOW);
  digitalWrite(clk, LOW);
  digitalWrite(lload, LOW);
  digitalWrite(din, LOW);
  digitalWrite(BOOST, LOW);

  brightness_control(2, 20); //divide factor 1-5, Pulse Width 0-40 / 10=22V, 20=41V, 30=58V, 40=75V
  set_vfd_blink_text("NTP WiFi V1.1", 250, 15);
  set_vfd_scroll_text("Wait NTP........", 250);
  setTime(0, 0, 0, 1, 1, 2020);//UTC
}
//------------------------------------------------------------------------------
void loop() {

  serial0_event(); // scan for ntp time of Serial.port:

  if (second() == 0) {
    if (ntp_request_is_send == false) {
      ntp_sync_indicator = true;
      Serial.println("ntp-sync: false");
      ntp_request_is_send = true;
    }
  }
  else {
    ntp_request_is_send = false;
  }

  hour_int = hour();
  minute_int = minute();
  second_int = second();
  day_int = day();
  month_int = month();
  year_int = year();

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

  date_string = day_string + " " + month_string + " " + year_string;
  time_string = hour_string + "-" + minute_string + "-" + second_string;

  if (is_vfd_on == 0) {
    set_vfd_text("        ", false);   //must be a string of length 8
  }
  if (is_vfd_on == 1) {
    set_vfd_text(time_string, ntp_sync_indicator);   //must be a string of length 8
  }
}
//----------------------------------------------------------------------------------------------------------------------
void serial0_event() {

  if (Serial.available() > 0) {
    data = "";
    data = Serial.readStringUntil('\n');
    data = data.substring(0, data.length() - 1); //LF,CR
    //Serial.println(data);
    //Serial.println(String(data.length()));
    split_data(data);
  }
}
//------------------------------------------------------------------------------
void split_data(String input) {
  //Serial.println("OK:" + ntp_time_date);
  String delimiter = ",";
  int delimiter_pos_1, delimiter_pos_2, delimiter_pos_3, delimiter_pos_4, delimiter_pos_5, delimiter_pos_6, delimiter_pos_7, delimiter_pos_8;

  delimiter_pos_1 = input.indexOf(delimiter);
  String value_0 = input.substring(0, delimiter_pos_1);

  if (value_0 == "ntp-time") {//ntp-time,16,29,31,19,10,2020,0,   (Time & Date & Night=0/1)  >>>UTC !!!

    delimiter_pos_2 = input.indexOf(delimiter, delimiter_pos_1 + 1);
    delimiter_pos_3 = input.indexOf(delimiter, delimiter_pos_2 + 1);
    delimiter_pos_4 = input.indexOf(delimiter, delimiter_pos_3 + 1);
    delimiter_pos_5 = input.indexOf(delimiter, delimiter_pos_4 + 1);
    delimiter_pos_6 = input.indexOf(delimiter, delimiter_pos_5 + 1);
    delimiter_pos_7 = input.indexOf(delimiter, delimiter_pos_6 + 1);
    delimiter_pos_8 = input.indexOf(delimiter, delimiter_pos_7 + 1);

    int value_1 = input.substring(delimiter_pos_1 + 1, delimiter_pos_2).toInt();
    int value_2 = input.substring(delimiter_pos_2 + 1, delimiter_pos_3).toInt();
    int value_3 = input.substring(delimiter_pos_3 + 1, delimiter_pos_4).toInt();
    int value_4 = input.substring(delimiter_pos_4 + 1, delimiter_pos_5).toInt();
    int value_5 = input.substring(delimiter_pos_5 + 1, delimiter_pos_6).toInt();
    int value_6 = input.substring(delimiter_pos_6 + 1, delimiter_pos_7).toInt();
    is_vfd_on = input.substring(delimiter_pos_7 + 1, delimiter_pos_8).toInt();

    ntp_sync_indicator = false;;
    Serial.println("ntp-sync: true");
    Serial.println(String(value_1) + ":" + String(value_2) + ":" + String(value_3) + "/" + String(value_4) + "." +  String(value_5) + "." + String(value_6) + "/" + String(is_vfd_on));

    setTime(value_1, value_2, value_3, value_4, value_5, value_6);
    time_t cet = CE.toLocal(now(), &tcr);
    setTime(cet);
  }

  if (value_0 == "get-time") {
    Serial.println(date_string + "/" + time_string + "/" + "Night:" + String(is_vfd_on));
  }

  if (value_0 == "set-text") {  //set-text,3,Hallo Welt,

    delimiter_pos_2 = input.indexOf(delimiter, delimiter_pos_1 + 1);
    delimiter_pos_3 = input.indexOf(delimiter, delimiter_pos_2 + 1);

    int value_1 = input.substring(delimiter_pos_1 + 1, delimiter_pos_2).toInt();
    String text = input.substring(delimiter_pos_2 + 1, delimiter_pos_3);

    if (value_1 == 1)set_vfd_blink_text(text, 250, 15); //frequency 250 , often 15
    if (value_1 == 2)set_vfd_scroll_text(text, 250); //delay time 250
    if (value_1 == 3) {
      for (int i = 0; i < 1000; i++) {
        set_vfd_text(text, false);   //must be a string of length 8
      }
    }
  }
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
