// Adafruit_NeoMatrix example for single NeoPixel Shield.
// By Marc MERLIN <marc_soft@merlins.org>
// Contains code (c) Adafruit, license BSD

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#include "mchp.h"
#include "mchp_upside_down.h"
#include "mchp_rotations.h"

#define WHITE_BACKDROP

#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define PIN PIN_PE1

// Max is 255, 32 is a conservative value to not overload
// a USB power supply (500mA) for 12x12 pixels.
#define BRIGHTNESS 32

// MATRIX DECLARATION:
// Parameter 1 = width of EACH NEOPIXEL MATRIX (not total display)
// Parameter 2 = height of each matrix
// Parameter 3 = number of matrices arranged horizontally
// Parameter 4 = number of matrices arranged vertically
// Parameter 5 = pin number (most are valid)
// Parameter 6 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the FIRST MATRIX; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs WITHIN EACH MATRIX are
//     arranged in horizontal rows or in vertical columns, respectively;
//     pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns WITHIN
//     EACH MATRIX proceed in the same order, or alternate lines reverse
//     direction; pick one.
//   NEO_TILE_TOP, NEO_TILE_BOTTOM, NEO_TILE_LEFT, NEO_TILE_RIGHT:
//     Position of the FIRST MATRIX (tile) in the OVERALL DISPLAY; pick
//     two, e.g. NEO_TILE_TOP + NEO_TILE_LEFT for the top-left corner.
//   NEO_TILE_ROWS, NEO_TILE_COLUMNS: the matrices in the OVERALL DISPLAY
//     are arranged in horizontal rows or in vertical columns, respectively;
//     pick one or the other.
//   NEO_TILE_PROGRESSIVE, NEO_TILE_ZIGZAG: the ROWS/COLUMS OF MATRICES
//     (tiles) in the OVERALL DISPLAY proceed in the same order for every
//     line, or alternate lines reverse direction; pick one.  When using
//     zig-zag order, the orientation of the matrices in alternate rows
//     will be rotated 180 degrees (this is normal -- simplifies wiring).
//   See example below for these values in action.
// Parameter 7 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 pixels)
//   NEO_GRB     Pixels are wired for GRB bitstream (v2 pixels)
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA v1 pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)

// Define full matrix width and height.
#define mw 32
#define mh 32
//Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(8, mh, 
//  mw/8, 1, 
//  PIN,
//  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
//    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + 
//// progressive vs zigzag makes no difference for a 4 arrays next to one another
//    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE,
//  NEO_GRB            + NEO_KHZ800 );

Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(32, 8, 1, 3, PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
    NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG +
// progressive vs zigzag makes no difference for a 4 arrays next to one another
    NEO_TILE_COLUMNS+ NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE,
  NEO_GRB + NEO_KHZ800 );

// This could also be defined as matrix->color(255,0,0) but those defines
// are meant to work for adafruit_gfx backends that are lacking color()
#define LED_BLACK		0

#define LED_RED_VERYLOW 	(3 <<  11)
#define LED_RED_LOW 		(7 <<  11)
#define LED_RED_MEDIUM 		(15 << 11)
#define LED_RED_HIGH 		(31 << 11)

#define LED_GREEN_VERYLOW	(1 <<  5)   
#define LED_GREEN_LOW 		(15 << 5)  
#define LED_GREEN_MEDIUM 	(31 << 5)  
#define LED_GREEN_HIGH 		(63 << 5)  

#define LED_BLUE_VERYLOW	3
#define LED_BLUE_LOW 		7
#define LED_BLUE_MEDIUM 	15
#define LED_BLUE_HIGH 		31

#define LED_ORANGE_VERYLOW	(LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW		(LED_RED_LOW     + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM	(LED_RED_MEDIUM  + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH		(LED_RED_HIGH    + LED_GREEN_HIGH)

#define LED_PURPLE_VERYLOW	(LED_RED_VERYLOW + LED_BLUE_VERYLOW)
#define LED_PURPLE_LOW		(LED_RED_LOW     + LED_BLUE_LOW)
#define LED_PURPLE_MEDIUM	(LED_RED_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_PURPLE_HIGH		(LED_RED_HIGH    + LED_BLUE_HIGH)

#define LED_CYAN_VERYLOW	(LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_CYAN_LOW		(LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_CYAN_MEDIUM		(LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_CYAN_HIGH		(LED_GREEN_HIGH    + LED_BLUE_HIGH)

#define LED_WHITE_VERYLOW	(LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_WHITE_LOW		(LED_RED_LOW     + LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_WHITE_MEDIUM	(LED_RED_MEDIUM  + LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_WHITE_HIGH		(LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH)

// Convert a BGR 4/4/4 bitmap to RGB 5/6/5 used by Adafruit_GFX
void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h) {
    // work around "a15 cannot be used in asm here" compiler bug when using an array on ESP8266
    // uint16_t RGB_bmp_fixed[w * h];
    static uint16_t *RGB_bmp_fixed = (uint16_t *) malloc( w*h*2);
    for (uint16_t pixel=0; pixel<w*h; pixel++) {
	uint8_t r,g,b;
	uint16_t color = pgm_read_word(bitmap + pixel);

	//Serial.print(color, HEX);
	b = (color & 0xF00) >> 8;
	g = (color & 0x0F0) >> 4;
	r = color & 0x00F;
	//Serial.print(" ");
	//Serial.print(b);
	//Serial.print("/");
	//Serial.print(g);
	//Serial.print("/");
	//Serial.print(r);
	//Serial.print(" -> ");
	// expand from 4/4/4 bits per color to 5/6/5
	b = map(b, 0, 15, 0, 31);
	g = map(g, 0, 15, 0, 63);
	r = map(r, 0, 15, 0, 31);
	//Serial.print(r);
	//Serial.print("/");
	//Serial.print(g);
	//Serial.print("/");
	//Serial.print(b);
	RGB_bmp_fixed[pixel] = (r << 11) + (g << 5) + b;
	//Serial.print(" -> ");
	//Serial.println(RGB_bmp_fixed[pixel], HEX);
    }
    matrix->drawRGBBitmap(x, y, RGB_bmp_fixed, w, h);
}

// 4 levels of crossing red lines.
//matrix->drawLine(0,mh/2-2, mw-1,2, LED_RED_VERYLOW);


void logo_shake(void){
  for(int i = 8; i >= 0; i--){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(i, 0, (const uint16_t *) mchp, 24, 24);
    matrix->show();
    delay(25);
  }
  for(int i = 1; i < 8; i++){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(i, 0, (const uint16_t *) mchp, 24, 24);
    matrix->show();
    delay(25);
  }

}
void logo_flip(void){
  #define FLIP_DELAY 0
  // flip
  for(int i = 0; i < 12; i++){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp, 24, 24);
    for(int j = -1; j < i; j++){
      matrix->drawLine(0, 0+j, 32, 0+j, LED_BLACK);
      matrix->drawLine(0, 24-j, 32, 24-j, LED_BLACK);
    }
    matrix->show();
    delay(FLIP_DELAY);
  }

  for(int i = 12; i >= 0; i--){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp_upside_down, 24, 24);
    for(int j = 0; j < i; j++){
      matrix->drawLine(0, 0+j, 32, 0+j, LED_BLACK);
      matrix->drawLine(0, 24-j, 32, 24-j, LED_BLACK);
    }
    matrix->show();
    delay(FLIP_DELAY);
  }

  delay(3000);

  // unfip
  for(int i = 0; i < 12; i++){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);    
    #endif
    matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp_upside_down, 24, 24);
    for(int j = -1; j < i; j++){
      matrix->drawLine(0, 0+j, 32, 0+j, LED_BLACK);
      matrix->drawLine(0, 24-j, 32, 24-j, LED_BLACK);
    }
    matrix->show();
    delay(FLIP_DELAY);
  }

  for(int i = 12; i >= 0; i--){
    matrix->clear();
    #ifdef WHITE_BACKDROP
      matrix->fillScreen(LED_WHITE_HIGH);
    #endif
    matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp, 24, 24);
    for(int j = 0; j < i; j++){
      matrix->drawLine(0, 0+j, 32, 0+j, LED_BLACK);
      matrix->drawLine(0, 24-j, 32, 24-j, LED_BLACK);
    }
    matrix->show();
    delay(FLIP_DELAY);
  }
  
  delay(3000);
}

void logo_rotate(void){
  #define ROTATION_DEGREES 15
  
  for(int i = 0; i < (int)360/ROTATION_DEGREES; i++){
   matrix->clear();
   #ifdef WHITE_BACKDROP
   matrix->fillScreen(LED_WHITE_HIGH);    
   #endif
   matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp_rotate[i], 24, 24);
   matrix->show();
   delay(100);
  }

}

void loop() {
  matrix->clear();
   matrix->drawRGBBitmap(4, 0, (const uint16_t *) mchp, 24, 24);
   matrix->show();
   delay(1000);

}

void setup() {
    Serial.begin(115200);
    matrix->begin();
    matrix->setTextWrap(false);
    matrix->setBrightness(BRIGHTNESS);
    // Test full bright of all LEDs. If brightness is too high
    // for your current limit (i.e. USB), decrease it.
    //matrix->fillScreen(LED_WHITE_HIGH);
    //matrix->show();
    matrix->clear();
}

// vim:sts=4:sw=4
