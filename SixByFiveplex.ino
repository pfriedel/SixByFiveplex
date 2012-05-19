/*
  USE_GITHUB_USERNAME=pfriedel
*/

#include <avr/pgmspace.h>  //AVR library for writing to ROM
#include "Charliplexing.h"
#include <Wire.h>
#include "RealTimeClockDS1307.h"
#include "tinyfont.h" // font is hiding in here.

//#include <String.h>;

#define SET_BUTTON_PIN 14
#define INC_BUTTON_PIN 15

#define COLS 6 // usually x
#define ROWS 5 // usually y

// are your LEDs a little obnoxious at full brightness? 2 seems to be the workable minimum, some animations break at 1.
int MAXBRIGHT=7;

byte world[COLS][ROWS][2]; // Create a double buffered world.
byte frame_log[COLS][ROWS];

int hours;
int minutes;

boolean isSettingHours   = true;
boolean isSettingMinutes = false;

#define BOUNCE_TIME_BUTTON  300   // bounce time in ms for the menu button - too short and every click is an adventure, too long and it's ponderous.

// last time a button was pushed; used for debouncing;
volatile unsigned long lastButtonTime = 0;

// indicates that one of the menu options (0..MAX_OPTION) is currently displayed;
boolean isSettingTime = false;

// when alternating between displaying quotes and time, start with the time;
boolean isDisplayingTime  = true;
boolean isDisplayingQuote = false;

// buffer for time display;
char timeBuffer[]   = "12:45";

//--------------------------------------------------------------------------------
void setup() {
  LedSign::Init(GRAYSCALE);  //Initializes the screen
  Wire.begin();

  //  Serial.begin(9600);

  pinMode(SET_BUTTON_PIN, INPUT); // A0
  pinMode(INC_BUTTON_PIN, INPUT); // A1
  digitalWrite(SET_BUTTON_PIN, HIGH); // enable the pullup on B0
  digitalWrite(INC_BUTTON_PIN, HIGH); // ditto
  
  // let the clock seed our randomizer - seems to work
  RTC.readClock();
  randomSeed(RTC.getSeconds());
  updateTimeBuffer();
}

void loop() {
  // check SET button;
  if (digitalRead(SET_BUTTON_PIN) == 0) {
    // "Set time" button was pressed;
    processSetButton();
  }
  
  // check INC button;
  if (isSettingTime) {
    if (digitalRead(INC_BUTTON_PIN) == 0) {
      // "Inc time" button was pressed;
      processIncButton();
      // save the new time;
      setTime();
    }
  }
  
  // display the menu option for 5 seconds after menu button was pressed;
  if ((lastButtonTime > 0) && (millis() - lastButtonTime < 5000)) {
    return; // avoids hitting the next conditional.
  }
  
  // return the main mode if no button was pressed for 5 seconds;
  if (isSettingTime) {
    // just finished setting up the time;
    isSettingTime = false;
    
    updateTimeBuffer();
    
    resetDisplay();
  }
  else {
    Banner(timeBuffer, 100);

    //clear the current world of whatever had been in it.
    for(int y = 0; y < ROWS; y++) { for(int x = 0; x < COLS; x++) { world[x][y][0] = 0; world[x][y][1] = 0; } }
    
    unsigned long now=millis();
    
    switch(random(2)) {
    case 0:
      Rain(now,5000);
      break;
    case 1:
      Life();
      break;
    }
  }
}

//--------------------------------------------------------------------------------
// functions

void initialize_frame_log() {
  for(int y=0; y < ROWS; y++) {
    for(int x=0; x < COLS; x++) {
      frame_log[x][y] = -1;
    }
  }
}
void log_current_frame() {
  for(int y=0; y < ROWS; y++) {
    for(int x=0; x < COLS; x++) {
      frame_log[x][y] = world[x][y][0];
    }
  }
}

void set_random_next_frame(void) {
  // blank out the world
  resetDisplay();
  
  int density = random(20,80);
  
  for(int y=0; y<ROWS; y++) {
    for(int x=0; x<COLS; x++) {
      if(random(100) > density) {
	world[x][y][1] = MAXBRIGHT;
      }
    }
  }
}

char current_equals_next() {
  char x, y;
  for(y=0; y<ROWS; y++) {
    for(x=0; x<COLS; x++) {
      if( world[x][y][0] != world[x][y][1] ) {
	return 0;
      }
    }
  }
  return 1;
}

void Life() {
    int frame_number;
    initialize_frame_log();

    // flash the screen
    for(int y=0; y < ROWS; y++) { for(int x=0; x < COLS; x++) { world[x][y][1] = MAXBRIGHT; } }
    fade_to_next_frame(int(200/MAXBRIGHT));
    delay(300); 
    for(int y=0; y < ROWS; y++) { for(int x=0; x < COLS; x++) { world[x][y][1] = 0; } }
    fade_to_next_frame(int(200/MAXBRIGHT));
    delay(300); 
    
    set_random_next_frame();
    fade_to_next_frame(int(200/MAXBRIGHT));
    delay(500);

    while(1) {
      if( frame_number == 0 ) { log_current_frame(); }

      generate_next_generation();

      if( current_equals_next() == 1 ) {
	for(int f=0; f<1500; f++) {
	  draw_frame();
	}
	break;
      }

      // frame logging will prevent a blinker from going forever.
//      if( next_equals_logged_frame() == 1 ) {
//	fade_to_next_frame(int(200/MAXBRIGHT));
//	for(int f = 0; f<FRAME_DELAY; f++) {
//	  draw_frame();
//	}
//	break;
//      }

      fade_to_next_frame(50);
      delay(500);
      frame_number++;

      if(frame_number == 20 ) {
	frame_number = 0;
      }
      
    }
}


void generate_next_generation(void){  //looks at current generation, writes to next generation array                                                    
  char x,y, neighbors;
  for ( y=0; y<ROWS; y++ ) {
    for ( x=0; x<COLS; x++ ) {
      //count the number of current neighbors - currently planar.  I'd love to make it toroidal.                                                        
      neighbors = 0;
      if( get_led_xy((x-1),(y-1)) > 0 ) { neighbors++; } //NW                                                                                       
      if( get_led_xy(( x ),(y-1)) > 0 ) { neighbors++; } //N                                                                                        
      if( get_led_xy((x+1),(y-1)) > 0 ) { neighbors++; } //NE                                                                                       
      if( get_led_xy((x-1),( y )) > 0 ) { neighbors++; } //W                                                                                        
      if( get_led_xy((x+1),( y )) > 0 ) { neighbors++; } //E                                                                                        
      if( get_led_xy((x-1),(y+1)) > 0 ) { neighbors++; } //SW                                                                                       
      if( get_led_xy(( x ),(y+1)) > 0 ) { neighbors++; } //S                                                                                        
      if( get_led_xy((x+1),(y+1)) > 0 ) { neighbors++; } //SE                                                                                       

      if( world[x][y][0] > 0 ){
        //current cell is alive                                                                                                                         
        if( neighbors < 2 ){
          //Any live cell with fewer than two live neighbours dies, as if caused by under-population.                                                   
          world[x][y][1] = 0;
        }
        if( (neighbors == 2) || (neighbors == 3) ){
          //Any live cell with two or three live neighbours lives on to the next generation.                                                            

          world[x][y][1] = MAXBRIGHT;
        }
        if( neighbors > 3 ){
          //Any live cell with more than three live neighbours dies, as if by overcyding.                                                             
          world[x][y][1] = 0;
        }

      }
      else {
        //current cell is dead                                                                                                                          
        if( neighbors == 3 ){
          // Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.                                               
          world[x][y][1] = MAXBRIGHT;
	  
        }
        else {
          //stay dead for next generation                                                                                                               
          world[x][y][1] = 0;
        }
      }
    }
  }
}

char get_led_xy (char col, char row) {
  if(col < 0 | col > COLS-1) { return 0; }
  if(row < 0 | row > ROWS-1) { return 0;  }
  return world[col][row][0];
}

void setTime() {
  
  RTC.switchTo24h();
  RTC.stop();
  RTC.setSeconds(0);
  RTC.setMinutes(minutes);
  RTC.setHours(hours);
  // dummy values, since they are not displayed anyway;
  RTC.setDayOfWeek(1);
  RTC.setDate(1);
  RTC.setMonth(1);
  RTC.setYear(9);
  RTC.setClock(); // important!  Without this, all of the prior settings are lost by the next getter
  RTC.start();
}

void updateTimeBuffer() {
  RTC.readClock();
  minutes = RTC.getMinutes();
  hours   = RTC.getHours();

  // build the string containing formatted time;
  sprintf(timeBuffer, "%2d:%02d", hours, minutes);
}

void resetDisplay() {
  for(int y = 0; y < ROWS; y++) {
    for(int x = 0; x < COLS; x++) {
      world[x][y][0] = 0;
      world[x][y][1] = 0;
    }
  }
}

void processSetButton() {
  if((hours > 23) or (minutes > 59)) {
    updateTimeBuffer();
  }

  // debouncing;
  if (abs(millis() - lastButtonTime) < BOUNCE_TIME_BUTTON)
    return;

  lastButtonTime = millis();

  isSettingTime    = true;
  isSettingHours   = !isSettingHours;
  isSettingMinutes = !isSettingMinutes;

  resetDisplay();
  
  int x;
  char text[2];
  if(isSettingHours) {
    LedSign::Clear();
    itoa(hours,text,10);
    x = Font_Draw(text[0],0,0,MAXBRIGHT);
    if(x>=3)
      Font_Draw(text[1],x-1,0,MAXBRIGHT);
    else
      Font_Draw(text[1],x,0,MAXBRIGHT);
    delay(10);
  }
  else {
    LedSign::Clear();
    itoa(minutes,text,10);
    x = Font_Draw(text[0],0,0,MAXBRIGHT);
    if(x>=3)
      Font_Draw(text[1],x-1,0,MAXBRIGHT);
    else
      Font_Draw(text[1],x,0,MAXBRIGHT);
    delay(10);
  }
}

//-----------------------------------------------------------
void processIncButton() {
  if((hours > 23) or (minutes > 59)) {
    updateTimeBuffer();
  }

  // debouncing;
  if (abs(millis() - lastButtonTime) < BOUNCE_TIME_BUTTON)
    return;

  lastButtonTime = millis();

  if (isSettingHours) {
    hours++;
    if (hours > 23) hours = 0;
  }
  else {
    minutes++;
    if (minutes > 59) minutes = 0;
  }

  int x;
  char text[2];
  if(isSettingHours) {
    LedSign::Clear();
    itoa(hours,text,10);
    x = Font_Draw(text[0],0,0,MAXBRIGHT);
    if(x>=3)
      Font_Draw(text[1],x-1,0,MAXBRIGHT);
    else
      Font_Draw(text[1],x,0,MAXBRIGHT);
    delay(10);
  }
  else {
    LedSign::Clear();
    itoa(minutes,text,10);
    x = Font_Draw(text[0],0,0,MAXBRIGHT);
    if(x>=3)
      Font_Draw(text[1],x-1,0,MAXBRIGHT);
    else
      Font_Draw(text[1],x,0,MAXBRIGHT);
    delay(10);
  }
}

void Rain(unsigned long now, unsigned long runtime) {
  int density = random(20,80);
  int stopraining = 0;
  while(1) {
    if(millis() > (now+runtime)) { // get out of rain eventually.
      stopraining++;
    }
    // move everything down one row
    for(int y = 0; y < ROWS; y++) {
      for(int x = 0; x < COLS; x++) {
	if(world[x][y][0] > 0) { // if there's a value in the current frame, copy it to the next frame, 1 row down
	  world[x][y+1][1] = world[x][y][0]; 
	}
	else { // otherwise blank the LED in the next frame.
	  world[x][y+1][1] = 0;
	}
      }
    }
    // fill in the now vacant top row with random lights
    for(int x = 0; x < COLS; x++) {
      if(stopraining < ROWS+1) {
	if(random(100) > density) { 
	  world[x][0][1] = random(MAXBRIGHT);
	}
	else {
	  world[x][0][1] = 0;
	}
      }
      else if(stopraining >= ROWS*COLS) {
	return;
      }
      else {
	world[x][0][1] = 0;
      }
    }
    // draw the changes - after this world[0] will be identical to world[1], so keep that in mind.
    fade_to_next_frame(80/MAXBRIGHT);
  }
}

void Breathe(unsigned long now, unsigned long runtime) {
  while(1) {
    if(millis() > (now+runtime)) {
      return;
    }
    for(int y=0; y < ROWS; y++) { for(int x=0; x < COLS; x++) { world[x][y][1] = MAXBRIGHT; } }
    if(millis() > (now+runtime)) { return; }
    
    fade_to_next_frame(int(200/MAXBRIGHT));
    delay(300); 
    if(millis() > (now+runtime)) { return;  }
    
    for(int y=0; y < ROWS; y++) { for(int x=0; x < COLS; x++) { world[x][y][1] = 0; } }
    
    fade_to_next_frame(int(200/MAXBRIGHT));
    if(millis() > (now+runtime)) { return; }
    delay(300); 
  }
}

// Theory of operation: Bring world[x][y][0] towards world[x][y][1] and draw it.  When nothing changes, break out of the loop.
void fade_to_next_frame(int speed) {
  char x,y, changes;

  while(1) {
    changes = 0;
    for(y = 0; y < ROWS; y++) {
      for(x = 0; x < COLS; x++) {
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

    // give those changes a second to apply - LedSign's interrupt driven nature
    // means otherwise the whole update could happen between polling intervals
    // and it would just jump from frame to the next frame.
    delay(speed); 
    if( changes == 0 ) {
      break;
    }
  }
}

// Draws the current data in world[0].
void draw_frame (void) {
  char x, y;
  for(y=0; y < ROWS; y++) {
    for(x=0; x < COLS; x++) {
      LedSign::Set(x,y,world[x][y][0]);
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
  for(int8_t j=5; j>=-(width+5); j--) {  // FIXME: this might need fixing for wider arrays 
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
	charCol + x < COLS && 
	charCol + x >= 0 && 
	charRow + y < ROWS && 
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
