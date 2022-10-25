#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <WS2812FX.h>

#define PIN PIN_PE1

WS2812FX ws2812fx = WS2812FX(768, PIN, NEO_GRB + NEO_KHZ800);

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, 1, 3, PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
    NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG +
// progressive vs zigzag makes no difference for a 4 arrays next to one another
    NEO_TILE_COLUMNS+ NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE,
  NEO_GRB + NEO_KHZ800 );

void setup() {
  // put your setup code here, to run once:
//  matrix.begin();
//  matrix.setBrightness(40);
//  matrix.fillScreen(0);
//  matrix.drawPixel(0,16, 0xFF);
  ws2812fx.init();
  ws2812fx.setBrightness(100);
  ws2812fx.setSpeed(200);
  ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
  ws2812fx.start();
}

void loop() {
  // put your main code here, to run repeatedly:
//  matrix.show();
//  delay(100);
  ws2812fx.service();
}
