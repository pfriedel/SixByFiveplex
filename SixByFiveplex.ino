/*
  USE_GITHUB_USERNAME=pfriedel
*/

#include <avr/pgmspace.h>  //AVR library for writing to ROM
#include "Charliplexing.h"
#include <Wire.h>
#include "RealTimeClockDS1307.h"
#include <stdio.h>
#include "tinyfont.h" // font is hiding in here.

//#include <String.h>;

#define button0 A0;
#define button1 A1;

// are your LEDs a little obnoxious at full brightness?
int MAXBRIGHT=4;

byte world[6][5][2]; // Create a double buffered world.

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
  unsigned long now = millis();

  // buttons are only read at the start of every cycle.  Too much mucking around
  // in interrupt code otherwise.  I probably want to borrow a page from the
  // WiseClock for setting time

  //  ReadButtons(); 
  switch(random(6)) {
    // make it rain!
  case 0:
    while(1) {
      unsigned long later = millis();
      // move everything down one row
      for(int y=0; y<=4; y++) {
	for(int x=0; x<=5; x++) {
	  if(world[x][y][0] > 0) { // if there's a value in the current frame, copy it to the next frame, 1 row down
	    world[x][y+1][1] = world[x][y][0]; 
	  }
	  else { // otherwise blank the LED in the next frame.
	    world[x][y+1][1] = 0;
	  }
	}
      }
      // fill in the now vacant top row with random lights
      for(int x = 0; x<=5; x++) {
	if(random(100) > 50) { 
	  int brightness = random(MAXBRIGHT); // gives it some texture.
	  world[x][0][1] = brightness;
	}
	else {
	  world[x][0][1] = 0;
	}
      }
      // draw the changes - after this world[0] will be identical to world[1], so keep that in mind.
      fade_to_next_frame(20);
      if(later > (now+5000)) { // get out of rain eventually.
	break;
      }
    }
    break;
    // the random initial seed for Life
  case 1:
    // build the world
    for(int y = 0; y<=4; y++) {
      for(int x = 0; x<=5; x++) {
	if(random(100) > 50)
	  world[x][y][0] = 1;
	else 
	  world[x][y][0] = 0;
	world[x][y][1]=0;
      }
    }
    // display the world
    for(int y = 0; y<=4; y++) {
      for(int x = 0; x<=5; x++) {
	if(world[x][y][0] == 1)
	  LedSign::Set(x,y,MAXBRIGHT);
	else
	  LedSign::Set(x,y,0);
      }
    }
    delay(1000);
    break;
    // a nice screen filling pulse
  case 2:
    // 8*25 = 200 + 300 second at the limit = half a second per up and down stroke.
    for(int g=0; g<=MAXBRIGHT; g++) {
      LedSign::Clear(g);
      delay(25); // how long should it pause on each brightness?
    }
    delay(300); 
    for(int g=MAXBRIGHT; g>=0; g--) {
      LedSign::Clear(g);
      delay(25);
    }
    delay(300); 
    break;
    // vertical sweeps
  case 3:
    for(int repeat=0; repeat<=5; repeat++) {
      for(int x=0; x<=5; x++) {
	LedSign::Vertical(x,MAXBRIGHT);
	LedSign::Vertical(x-1,0);
	delay(30);
      }
      LedSign::Clear();
    }
    break;
    // horizontal sweeps
  case 4:
    for(int repeat=0; repeat<=5; repeat++) {
      for(int y=0; y<=4; y++) {
	LedSign::Horizontal(y,MAXBRIGHT);
	LedSign::Horizontal(y-1,0);
	delay(50);
      }
      LedSign::Clear();
    }
    break;
    // lights every LED in sequence.
  case 5:
    for(int y=0; y<=4; y++) {
      for(int x=0; x<=5; x++) {
	LedSign::Set(x,y,MAXBRIGHT);
	delay(30);
	LedSign::Set(x,y,0);
      }
    }
    break;
  }

  char clock_time[6] = "00:00";
  RTC.readClock();
  sprintf(clock_time, "%02d:%02d", RTC.getHours(), RTC.getMinutes());
  Serial.println(clock_time);
  Banner(clock_time, 100);

}

//--------------------------------------------------------------------------------
// functions


// Theory of operation: Bring world[x][y][0] towards world[x][y][1] and draw it.  When nothing changes, break out of the loop.
void fade_to_next_frame(int speed) {
  char x,y, changes;

  while(1) {
    changes = 0;
    for(y=0; y<=4; y++) {
      for(x=0; x<=5; x++) {
	if( world[x][y][0] < world[x][y][1] ) {
	  world[x][y][0]++;
	  changes++;
	}
	if( world[x][y][0] > world[x][y][1] ) {
	  world[x][y][0]--;
	  changes++;
	}
      }
    }
    draw_frame();
    delay(speed); // give those changes a second to apply - LedSign's interrupt driven nature means otherwise the whole update could happen between polling intervals and it would just jump from frame to the next frame.
    if( changes == 0 ) {
      break;
    }
  }
}

void draw_frame (void) {
  char x, y;
  for(y=0; y<=4; y++) {
    for(x=0; x<=5; x++) {
      LedSign::Set(x,y,world[x][y][0]);
    }
  }
}

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
      x2 = Font_Draw(str[i],x,0,MAXBRIGHT);
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
