/*
  USE_GITHUB_USERNAME=pfriedel
*/

#include <avr/pgmspace.h>  //AVR library for writing to ROM
#include "Charliplexing.h"
#include <Wire.h>
#include "RealTimeClockDS1307.h"
#include <stdio.h>

//#include <String.h>;

#define button0 A0;
#define button1 A1;

//--------------------------------------------------------------------------------
// define some bitmapped letters!  put them in program memory so you aren't using up valuable running memory.
prog_uchar letters_48[] PROGMEM = /* 0 */ { 0,1, 0,2, 0,3, 1,0, 1,4, 2,1, 2,2, 2,3, 9,9 }; 
prog_uchar letters_49[] PROGMEM = /* 1 */ { 0,0, 0,1, 0,2, 0,3, 0,4, 9,9 }; 
prog_uchar letters_50[] PROGMEM = /* 2 */ { 0,0, 0,3, 0,4, 1,0, 1,2, 1,4, 2,1, 2,4, 9,9 }; 
prog_uchar letters_51[] PROGMEM = /* 3 */ { 0,0, 0,4, 1,0, 1,2, 1,4, 2,0, 2,1, 2,2, 2,3, 2,3, 9,9 }; 
prog_uchar letters_52[] PROGMEM = /* 4 */ { 0,0, 0,1, 0,2, 1,2, 2,1, 2,2, 2,3, 2,4, 9,9 }; 
prog_uchar letters_53[] PROGMEM = /* 5 */ { 0,0, 0,1, 0,2, 0,4, 1,0, 1,2, 1,4, 2,0, 2,2, 2,3, 9,9 }; 
prog_uchar letters_54[] PROGMEM = /* 6 */ { 0,1, 0,2, 0,3, 1,0, 1,2, 1,4, 2,0, 2,2, 2,3, 9,9 }; 
prog_uchar letters_55[] PROGMEM = /* 7 */ { 0,0, 0,3, 0,4, 1,0, 1,2, 2,0, 2,1, 9,9 }; 
prog_uchar letters_56[] PROGMEM = /* 8 */ { 0,0, 0,1, 0,3, 0,4, 1,0, 1,2, 1,4, 2,0, 2,1, 2,3, 2,4, 9,9 }; 
prog_uchar letters_57[] PROGMEM = /* 9 */ { 0,1, 0,2, 0,4, 1,0, 1,2, 1,4, 2,1, 2,2, 2,3, 9,9 }; 
prog_uchar letters_58[] PROGMEM = /* : */ { 0,1, 0,3, 9,9 }; 
prog_uchar letters_65[] PROGMEM = /* A */ { 0,1, 0,2, 0,3, 0,4, 1,0, 1,2, 2,0, 2,2, 3,1, 3,2, 3,3, 3,4, 9,9 }; 
prog_uchar letters_66[] PROGMEM = /* B */ { 0,0, 0,1, 0,2, 0,3, 0,4, 1,0, 1,2, 1,4, 2,0, 2,2, 2,4, 3,1, 3,3, 9,9 }; 
prog_uchar letters_67[] PROGMEM = /* C */ { 0,1, 0,2, 0,3, 1,0, 1,4, 2,0, 2,4, 3,1, 3,3, 9,9 }; 
prog_uchar letters_68[] PROGMEM = /* D */ { 0,0, 0,1, 0,2, 0,3, 0,4, 1,0, 1,4, 2,0, 2,4, 3,1, 3,2, 3,3, 9,9 }; 
prog_uchar letters_69[] PROGMEM = /* E */ { 0,0, 0,1, 0,2, 0,3, 0,4, 1,0, 1,2, 1,4, 2,0, 2,2, 2,4, 3,0, 3,4, 9,9 }; 
prog_uchar letters_70[] PROGMEM = /* F */ { 0,0, 0,1, 0,2, 0,3, 0,4, 1,0, 1,2, 2,0, 2,2, 3,0, 9,9 }; 
prog_uchar letters_71[] PROGMEM = /* G */ { 0,1, 0,2, 0,3, 1,0, 1,4, 2,0, 2,2, 2,4, 3,0, 3,2, 3,3, 9,9 }; 
prog_uchar letters_72[] PROGMEM = /* H */ { 0,0, 0,1, 0,2, 0,3, 0,4, 1,2, 2,2, 3,0, 3,1, 3,2, 3,3, 3,4, 9,9 }; 
prog_uchar letters_73[] PROGMEM = /* I */ { 0,0, 0,4, 1,0, 1,1, 1,2, 1,3, 1,4, 2,0, 2,4, 9,9 }; 
prog_uchar letters_74[] PROGMEM = /* J */ { 0,3, 1,4, 2,4, 3,0, 3,1, 3,2, 3,3, 9,9 }; 
prog_uchar letters_75[] PROGMEM = /* K */ { 0,0, 0,1, 0,2, 0,3, 0,4, 1,2, 2,1, 2,3, 3,0, 3,4, 9,9 }; 
prog_uchar letters_76[] PROGMEM = /* L */ { 0,0, 0,1, 0,2, 0,3, 0,4, 1,4, 2,4, 9,9 }; 
prog_uchar letters_77[] PROGMEM = /* M */ { 0,0, 0,1, 0,2, 0,3, 0,4, 1,1, 2,2, 3,1, 4,0, 4,1, 4,2, 4,3, 4,4, 9,9  }; 
prog_uchar letters_78[] PROGMEM = /* N */ { 0,0, 0,1, 0,2, 0,3, 0,4, 1,1, 2,2, 3,3, 4,0, 4,1, 4,2, 4,3, 4,4, 9,9 }; 
prog_uchar letters_79[] PROGMEM = /* O */ { 0,1, 0,2, 0,3, 1,0, 1,4, 2,0, 2,4, 3,1, 3,2, 3,3, 9,9 }; 
prog_uchar letters_80[] PROGMEM = /* P */ { 0,0, 0,1, 0,2, 0,3, 0,4, 1,0, 1,2, 2,0, 2,2, 3,1, 9,9 }; 
prog_uchar letters_81[] PROGMEM = /* Q */ { 0,1, 0,2, 0,3, 1,0, 1,4, 2,0, 2,3, 2,4, 3,1, 3,2, 3,3, 3,4, 9,9 }; 
prog_uchar letters_82[] PROGMEM = /* R */ { 0,0, 0,1, 0,2, 0,3, 0,4, 1,0, 1,2, 2,0, 2,2, 2,3, 3,1, 3,4, 9,9 }; 
prog_uchar letters_83[] PROGMEM = /* S */ { 0,1, 0,4, 1,0, 1,2, 1,4, 2,0, 2,2, 2,4, 3,0, 3,3, 9,9 }; 
prog_uchar letters_84[] PROGMEM = /* T */ { 0,0, 1,0, 1,1, 1,2, 1,3, 1,4, 2,0, 9,9 }; 
prog_uchar letters_85[] PROGMEM = /* U */ { 0,0, 0,1, 0,2, 0,3, 1,4, 2,4, 3,0, 3,1, 3,2, 3,3, 9,9 }; 
prog_uchar letters_86[] PROGMEM = /* V */ { 0,0, 0,1, 1,2, 1,3, 2,4, 3,2, 3,3, 4,0, 4,1, 9,9 }; 
prog_uchar letters_87[] PROGMEM = /* W */ { 0,0, 0,1, 0,2, 0,3, 1,4, 2,0, 2,1, 2,2, 2,3, 3,4, 4,0, 4,1, 4,2, 4,3, 9,9 }; 
prog_uchar letters_88[] PROGMEM = /* X */ { 0,0, 0,4, 1,1, 1,3, 2,2, 3,1, 3,3, 4,0, 4,4, 9,9 }; 
prog_uchar letters_89[] PROGMEM = /* Y */ { 0,0, 1,1, 2,2, 2,3, 2,4, 3,1, 4,0, 9,9 }; 
prog_uchar letters_90[] PROGMEM = /* Z */ { 0,0, 0,3, 0,4, 1,0, 1,2, 1,4, 2,0, 2,1, 2,4, 9,9 }; 

// Basically an ASCII lookup table
prog_uchar* font[] = { letters_48, letters_49, letters_50, letters_51, letters_52, letters_53, letters_54, letters_55, letters_56, letters_57, letters_58, 0, 0, 0, 0, 0, 0, letters_65, letters_66, letters_67, letters_68, letters_69, letters_70, letters_71, letters_72, letters_73, letters_74, letters_75, letters_76, letters_77, letters_78, letters_79, letters_80, letters_81, letters_82, letters_83, letters_84, letters_85, letters_86, letters_87, letters_88, letters_89, letters_90 };

// Define the beginning and end of your available glyphs
uint16_t fontMin=48;
uint16_t fontMax=90;

//--------------------------------------------------------------------------------
void setup() {
  LedSign::Init(GRAYSCALE);  //Initializes the screen
  Wire.begin();

  Serial.begin(9600);

  pinMode(A0, INPUT); // A0
  pinMode(A1, INPUT); // A1
  digitalWrite(A0, HIGH); // enable the pullup on B0
  digitalWrite(A1, HIGH); // ditto
  
  // let the clock seed our randomizer - seems to work
  RTC.readClock();
  randomSeed(RTC.getSeconds());
}

void loop() {
  // buttons are only read at the start of every cycle.  Too much mucking around
  // in interrupt code otherwise.

  ReadButtons(); 

  int mode = random(4);

  switch(mode) {
  case 0:
    // 8*25 = 200 + 300 second at the limit = half a second per up and down stroke.
    for(int g=0; g<=7; g++) {
      LedSign::Clear(g);
      delay(25); // how long should it pause on each brightness?
    }
    delay(300); 
    for(int g=7; g>=0; g--) {
      LedSign::Clear(g);
      delay(25);
    }
    delay(300); 
    break;
  case 1:
    for(int repeat=0; repeat<=5; repeat++) {
      for(int x=0; x<=5; x++) {
	LedSign::Vertical(x,7);
	LedSign::Vertical(x-1,0);
	delay(30);
      }
      LedSign::Clear();
    }
    break;
  case 2:
    for(int repeat=0; repeat<=5; repeat++) {
      for(int y=0; y<=4; y++) {
	LedSign::Horizontal(y,7);
	LedSign::Horizontal(y-1,0);
	delay(30);
      }
      LedSign::Clear();
    }
    break;
  case 3:
    // lights every LED in sequence.
    for(int y=0; y<=4; y++) {
      for(int x=0; x<=5; x++) {
	LedSign::Set(x,y,7);
	delay(30);
	LedSign::Set(x,y,0);
      }
    }
    break;
  }

  char clock_time[9] = "00:00:00";
  RTC.readClock();
  sprintf(clock_time, "%02d:%02d:%02d", RTC.getHours(), RTC.getMinutes(), RTC.getSeconds());
  Serial.println(clock_time);
  Banner(clock_time, 100);

}

//--------------------------------------------------------------------------------
// functions

void ReadButtons() {
  bool button0State = digitalRead(A0);  
  bool button1State = digitalRead(A1);

  Serial.println(button0State, DEC);
  Serial.println(button1State, DEC);

  if(button0State == 0) { // setting mode!
    int setting_xpos;
    Banner("SET", 100);
    
    RTC.readClock();
    
    char hour[2];
    itoa(RTC.getHours(), hour, 10);
    
    while(1) {
      setting_xpos = Font_Draw(hour[0],0,0,7);
      setting_xpos = Font_Draw(hour[1],(setting_xpos),0,7);
      delay(100);
      LedSign::Clear();
      delay(100);
      bool button0State = digitalRead(A0);
      bool button1State = digitalRead(A1);
      if(button0State == 0) {
	break;
      }
      else if (button1State == 0) {
	hour[1]++;
      }
    }
    
  }

}
		 

/** Writes an array to the display
 * @param str is the array to display
 * @param speed is the inter-frame speed
 */
//void Banner ( String str, int speed ) { // this works, hogs memory.
void Banner ( char* str, int speed ) {
  // length is length(array)
  // width is the width of the array in pixels.
  // these can be unsigned - int8 might be too small
  uint8_t width=0, length=0;
  
  // figure out pixel width of str
  while(str[length]!='\0') { //read to the end of the string
    int charwidth=0;
    length++;
    // get the character's width by calling Font_Draw in the "Don't draw this, just tell me how wide it is" mode.
    // It would probably be faster to have a lookup table.
    charwidth = Font_Draw(str[length],0,0,0);
    // adds the width of ths current character to the pixel width.
    width=width+charwidth; 
  }

  // these need to be signed, otherwise I can't draw off the left side of the screen.
  int8_t x=0, x2=0; // x position buckets
  
  // j is the virtual display width from the actual rightmost column to a 
  // virtual column off the left hand side.  Decrements, so the scroll goes
  // to the left.
  for(int8_t j=5; j>=-(width+5); j--) { 
    // x starts out at j (which is always moving to the left, remember)
    x=j; 
    // clear the sign
    LedSign::Clear(); 
    // walk through the array, drawing letters where they belong.
    for(int i=0; i<length; i++) { 
      x2 = Font_Draw(str[i],x,0,7);
      // sets the new xpos based off of the old xpos + the width of the 
      // current character.
      x+=x2;
    }
    delay(speed);
  }
}


/** Draws a figure (0-Z). You should call it with set=1, 
 * wait a little them call it again with set=0
 * @param figure is the figure [0-9]
 * @param x,y coordinates, 
 * @param set is 1 or 0 to draw or clear it
 */
uint8_t Font_Draw(unsigned char letter,int x,int y,int set) {
  uint16_t maxx=0;
  
  uint8_t charCol;
  uint8_t charRow;

  prog_uchar* character;
  if (letter==' ') return 2+2; // if I'm sent a space, just tell the called that the end column is 4 spaces away.
  if (letter<fontMin || letter>fontMax) { // return a blank if the sender tries to call something outside of my font table
    return 0;
  }
  character = font[letter-fontMin];

  int i=0;

  charCol = pgm_read_byte_near(character);
  charRow = pgm_read_byte_near(character + 1);

  while (charRow != 9) { // the terminal 9 ends the font element.
    if (charCol>maxx) maxx=charCol; // increment the maximum column count
    if ( // if the resulting pixel would be onscreen, send it to LedSign::Set.
	charCol + x < 6 && 
	charCol + x >= 0 && 
	charRow + y < 5 && 
	charRow + y >= 0
	) {
      LedSign::Set(charCol + x, charRow + y, set);
    } 
    i+=2; // only get the even elements of the array.

    charCol = pgm_read_byte_near(character + i);
    charRow = pgm_read_byte_near(character + i + 1);
  }
  return maxx+2; // return the rightmost column + 2 for spacing.
}
