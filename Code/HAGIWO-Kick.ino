#include <avr/io.h>

//---------basic waveform make--------------------
unsigned int frq = 60000; //frequency
float duty = 0.5;//pin10
int wave = 1000;//output wave form , fand + harm
const static word fand[180] PROGMEM = {
 0,  17, 34, 52, 69, 86, 103,  120,  137,  154,  171,  187,  203,  219,  234,  250,  264,  279,  293,  307,  321,  334,  347,  359,  371,  383,  394,  404,  414,  424,  433,  441,  449,  456,  463,  469,  475,  480,  485,  489,  492,  495,  497,  498,  499,  500,  499,  498,  497,  495,  492,  489,  485,  480,  475,  469,  463,  456,  449,  441,  433,  424,  414,  404,  394,  383,  371,  359,  347,  334,  321,  307,  293,  279,  264,  250,  234,  219,  203,  187,  171,  154,  137,  120,  103,  86, 69, 52, 34, 17, 0,  -18,  -35,  -53,  -70,  -87,  -104, -121, -138, -155, -172, -188, -204, -220, -235, -250, -265, -280, -294, -308, -322, -335, -348, -360, -372, -384, -395, -405, -415, -425, -434, -442, -450, -457, -464, -470, -476, -481, -486, -490, -493, -496, -498, -499, -500, -500, -500, -499, -498, -496, -493, -490, -486, -481, -476, -470, -464, -457, -450, -442, -434, -425, -415, -405, -395, -384, -372, -360, -348, -335, -322, -308, -294, -280, -265, -250, -235, -220, -204, -188, -172, -155, -138, -121, -104, -87,  -70,  -53,  -35,  -18
};
const static word harm[180] PROGMEM = {
 0,  416,  365,  321,  334,  307,  279,  279,  254,  232,  227,  203,  185,  178,  155,  140,  131,  110,  97, 88, 68, 57, 48, 30, 21, 11, -4, -11,  -21,  -34,  -40,  -48,  -60,  -63,  -71,  -80,  -82,  -88,  -95,  -95,  -101, -104, -103, -107, -109, -106, -109, -107, -103, -104, -101, -95,  -95,  -88,  -82,  -80,  -71,  -63,  -60,  -48,  -40,  -34,  -21,  -11,  -4, 11, 21, 30, 48, 57, 68, 88, 97, 110,  131,  140,  155,  178,  185,  203,  227,  232,  254,  279,  279,  307,  334,  321,  365,  416,  0,  -417, -366, -322, -335, -308, -280, -280, -255, -233, -228, -204, -186, -179, -156, -141, -132, -111, -98,  -89,  -69,  -58,  -49,  -31,  -22,  -12,  3,  10, 20, 33, 39, 47, 59, 62, 70, 79, 81, 87, 94, 94, 100,  103,  102,  106,  108,  105,  108,  106,  102,  103,  100,  94, 94, 87, 81, 79, 70, 62, 59, 47, 39, 33, 20, 10, 3,  -12,  -22,  -31,  -49,  -58,  -69,  -89,  -98,  -111, -132, -141, -156, -179, -186, -204, -228, -233, -255, -280, -280, -308, -335, -322, -366, -417
};

//-------------trig------------
bool trig = 0;//
bool old_trig = 0;//

//-----------control wave color------------
int h = 0; //table finish count
int i = 0; //waveform
byte j = 1; //decay frequency
float k = 1; //decay amp

int knob_tone = 512;
int knob_harm = 512;
int knob_attack = 512;
int knob_decay = 512;
int CV_harm = 512;

void setup() {
 pinMode(10, OUTPUT);//sound output
 pinMode(13, OUTPUT);//LED

 // PWM specification setting
 TCCR1A = 0b00100001;
 TCCR1B = 0b00010001;//分周比1

 // PWM setting
 OCR1A = (unsigned int)(8000000 / frq );

 //for development
 //  Serial.begin(9600);
}

void loop() {

 old_trig = trig;
 trig = digitalRead(3);

 digitalWrite(13, trig); //LED ON during trigger is high

 if (old_trig == 0 && trig == 1) {//trigger detect
   h = 0;
   i = 0;
   j = 0;
   k = 1;
 }

 i ++;
 if (i > 179) {

   knob_tone = analogRead(0) / 64;
   knob_harm = (1023 - analogRead(1) ) / 32;
   knob_decay = analogRead(3) / 16;
   knob_attack = analogRead(5) / 16;
   CV_harm = analogRead(6) / 32;

   knob_harm = knob_harm - CV_harm;

   if ( knob_harm >= 31) {
     knob_harm = 31;
   }

   //make decay
   h ++;
   i = 0;
   j = j + 5;//modulate wave frequency slower rate

   if (knob_decay <= 32) { //no body decay
     k = k * (98 - (32 - knob_decay)) / 100 - 10 / 100;
   }
   else if (knob_decay > 32) { //with body decay
     if (h < (16 - knob_tone) / 2) {
       k = 0.8 + h / 16 / 5;
     }
     else {
       k = k * (94 -  (knob_decay - 32)) / 100 - 10 / 100;
     }
   }
 }

 delayMicroseconds((knob_tone) * j);//modulate wave frequency slower

 //---------kick wave making------------------
 if (h == 0 && i < knob_attack  && knob_attack >= 5) { // make punch sound by noise
   wave = (pgm_read_word(&(fand[random(0, 179)]))) * 32 + (pgm_read_word(&(harm[i]))) * (32 - knob_harm);
 }

 else if (h <= 1 && i > 25 && i < 90) {
   wave = (pgm_read_word(&(fand[i]))) * (32 + knob_attack / 3) + (pgm_read_word(&(harm[i]))) * (32 - knob_harm);
 }

 else {
   wave = (pgm_read_word(&(fand[i]))) * 32 + (pgm_read_word(&(harm[i]))) * (32 - knob_harm);
 }

 wave = wave * k;
 wave = wave / 32 + 500;

 duty = (float)wave / 1000;

 //------output pwm wave -----------------
 // Duty rate setting
 OCR1B = (unsigned int)(8000000  / frq * duty);
}