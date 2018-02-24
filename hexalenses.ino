#include <Bounce2.h>
#include "FastLED.h"

// Tell me about these LEDs you brought me.
#define NUM_LEDS 19
#define DATA_PIN 3
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB

// How much power in your supply, we'll keep it under that.
#define SOURCE_VOLTAGE 5
#define SOURCE_CURRENT 800 //mA

// define our buttons, all use the internal pullup resistor
// BUTT_PATTERN and BUTT_SOLID are debounced using the Bounce2 library.
#define BUTT_PATTERN 2
#define BUTT_BRIGHTNESS 4
#define BUTT_SOLID 5
#define BUTT_HUE 6

//deal with the debouncers
#define BUTTON_DEBOUNCE_DELAY   5   // [ms]
Bounce debounced_pattern = Bounce();
Bounce debounced_solid = Bounce();

//globals set by buttons
byte current_brightness = 255;
byte current_hue = 0;
bool solid_or_pattern = 1; // 0 = solid, 1 = pattern

//used by patterns
int hue = 0;
int divisor = 30;

void flashlight();
void flashlight100();
void flashlight75();
void flashlight50();
void flashlight25();
void flashlight10();
void breath();
void randy();
void red_rand();
void green_rand();
void blue_rand();
void earth_rand();
void water_rand();
void fire_rand();
void air_rand();
void bw_rand();
void off();
void sectionals();
void unicornPoo();
void unicornSpit();
void rainbow_cylon();
void palette_fader();
void FillLEDsFromPaletteColors( uint8_t colorIndex);
void ChangePalettePeriodically();
void pollAndProcess();
void all_h(byte h);



//initialize palettes
CRGBPalette16 currentPalette( CRGB::Black);
CRGBPalette16 targetPalette( OceanColors_p );

//super helpful FastLED array size function.  WHY IS THIS NOT VANILLA ARDUINO?!
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// Define the array of leds
CRGB leds[NUM_LEDS];

//our list of patterns
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { sectionals, unicornSpit, unicornPoo, green_rand, breath, flashlight100, flashlight50, flashlight10, rainbow_cylon, red_rand, off, 
                                    green_rand, off, blue_rand, off, earth_rand, off, air_rand, off, fire_rand, off, water_rand, off, randy, off, palette_fader, off };
volatile uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

void setup() { 
  //set up our initial pins and serial comm
  Serial.begin(9600);

  pinMode(BUTT_PATTERN,INPUT_PULLUP);
  debounced_pattern.attach(BUTT_PATTERN);
  debounced_pattern.interval(BUTTON_DEBOUNCE_DELAY);

  pinMode(BUTT_SOLID,INPUT_PULLUP);
  debounced_solid.attach(BUTT_SOLID);
  debounced_solid.interval(BUTTON_DEBOUNCE_DELAY);

  pinMode(BUTT_BRIGHTNESS,INPUT_PULLUP);
  pinMode(BUTT_HUE,INPUT_PULLUP);

	FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds,NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setMaxPowerInVoltsAndMilliamps(SOURCE_VOLTAGE,SOURCE_CURRENT); 

 
  leds[0] = CRGB::Green;
  FastLED.setBrightness(current_brightness);
  delay(2000); //give the thing time to boot.
  FastLED.show();
  
  delay(500);  //give the human time to see the debug
}

void loop() { 
  pollAndProcess();

  if(solid_or_pattern)
  {
    gPatterns[gCurrentPatternNumber]();
  }else{
    all_h(current_hue);
  }
}

void pollAndProcess(){
  // poll button states, return continuous on-time [ms] if pressed (debounced)
  debounced_pattern.update();
  debounced_solid.update();
  
  if(debounced_pattern.fell())
  {
    nextPattern();
    Serial.print("I heard the pattern button.  Pattern has updated to: ");
    Serial.println(gCurrentPatternNumber);
  }

  if(digitalRead(BUTT_BRIGHTNESS) == LOW)
  {
    //increment by 3 to go quickly through brightnesses
    current_brightness = current_brightness+3; //auto-loops, because byte

    //handle some edge cases and allow for a true "off"
    if(current_brightness < 3)
    {
      current_brightness = 0;
      FastLED.setBrightness(current_brightness);
      //won't see it unless we show it (still at 255 from last show)
      FastLED.show();
      //and wait a second so they can leave it there.
      delay(1000);      
    }

    if(current_brightness > 250)
    {
      //close enough to full, give them full brightness
      current_brightness = 255;
      FastLED.setBrightness(current_brightness);
      //and wait a second so they can leave it there.
      delay(1000);
    }
    Serial.print("I heard the brightness button.  Brightness is now: ");
    Serial.println(current_brightness);
  }

  //some patterns use the built-in brightness functions, so we set it every loop and they override if they need it.
  FastLED.setBrightness(current_brightness);    

  
  if(debounced_solid.fell())
  {
    solid_or_pattern = ! solid_or_pattern; 
    Serial.print("I heard the solid button and have switched solid_or_pattern to: ");
    Serial.println(solid_or_pattern);
  }
  
  if(digitalRead(BUTT_HUE) == LOW)
  {
    current_hue++;
    Serial.print("I heard the hue button.  Hue is now: ");
    Serial.println(current_hue);
  }
  
}

void nextPattern()
{
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns); 
}

void breath()
{
  Serial.println("Starting breath");
  float breath = (exp(sin(millis()/5000.0*PI)) - 0.36787944)*108.0;
  breath = map(breath, 0, 255, 20, 255);
  FastLED.setBrightness(breath);
  fill_rainbow(leds, NUM_LEDS, (hue++/divisor));
  Serial.println(hue);
  if(hue == (255 * divisor)) {
    hue = 0;
  }
  FastLED.show();  
}

//BE - take an array of pixels and set them all to given color.
//you probably want to do a colorWipe or all to a background color first.
int setPixelGroup(byte pixels[],byte pixelCount, CRGB c)
{
  byte i;
  for(i=0;i<pixelCount;i++)
  {
    leds[pixels[i]] = c;
  }
  FastLED.show();
  return 1;
}

//BE - take an array of pixels and set them all to given color.
//you probably want to do a colorWipe or all to a background color first.
int setPixelGroupHSV(byte pixels[],byte pixelCount, CHSV h)
{
  byte i;
  for(i=0;i<pixelCount;i++)
  {
    leds[pixels[i]] = h;
  }
  FastLED.show();
  return 1;
}

void sectionals()
{
  byte north[] = {7,8,18,1,2,6,0};
  byte south[] = {0,3,5,4,12,13,14};
  byte southwest[] = {2,10,0,3,11,4,12};
  byte northeast[] = {18,1,17,6,0,16,5};
  byte northwest[] = {8,9,1,2,10,0,3};
  byte southeast[] = {6,0,16,5,4,15,14};
  byte center[] = {1,2,6,0,3,4,5};

  switch(random(1000))
  {
    case 0:
      setPixelGroup(north, 7, CHSV(hue,255,255));
      delay(250);
      break;
    case 1:
      setPixelGroup(northwest, 7, CHSV(hue,255,255));
      delay(250);      
      break;
    case 2:
      setPixelGroup(northeast, 7, CHSV(hue,255,255));
      delay(250);      
      break;
    case 3:
      setPixelGroup(south, 7, CHSV(hue,255,255));
      delay(250);      
      break;
    case 4:
      setPixelGroup(southwest, 7, CHSV(hue,255,255));
      delay(250);      
      break;
    case 5:
      setPixelGroup(southeast, 7, CHSV(hue,255,255));
      delay(250);
      break;
    case 6:
      setPixelGroup(center, 7, CHSV(hue,255,255));
      delay(250);
      break;
    case 7:
      fill_rainbow(leds, NUM_LEDS, (hue++/divisor));
    default:
      fadeall();
      FastLED.show();
      delay(10);
      break;
  }

  hue = hue+23;
}

void unicornSpit()
{
  byte smallRing[] = {0};
  byte medRing[] = {1,2,3,4,5,6};
  byte bigRing[] = {7,8,9,10,11,12,13,14,15,16,17,18};

  CRGB colors[] = {CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Blue, CRGB::Indigo, CRGB::Violet};
  
  setPixelGroup(smallRing, 1,   colors[(hue+2)%(sizeof(colors)/sizeof(CRGB))]);
  setPixelGroup(medRing,   6,   colors[(hue+1)%(sizeof(colors)/sizeof(CRGB))]);
  setPixelGroup(bigRing,   12,  colors[(hue+0)%(sizeof(colors)/sizeof(CRGB))]);
  delay(200);
  hue++;
}

void unicornPoo()
{
  byte smallRing[] = {0};
  byte medRing[] = {1,2,3,4,5,6};
  byte bigRing[] = {7,8,9,10,11,12,13,14,15,16,17,18};
  
  setPixelGroupHSV(smallRing, 1,   CHSV((hue*10)%255,255,255));
  setPixelGroupHSV(medRing,   6,   CHSV(((hue+10)*10)%255,255,255));
  setPixelGroupHSV(bigRing,   12,  CHSV(((hue+20)*10)%255,255,255));
  delay(50);
  hue++;
}

void randy()
{
  Serial.println("Starting randy");
  CRGB r;
  leds[random(0,NUM_LEDS)] = r.setRGB(random(0,256),random(0,256),random(0,256));
  FastLED.show();
  FastLED.delay(random(0,255));
}


void flashlight100(){
  FastLED.setBrightness(255);
  flashlight();
}

void flashlight75(){
  FastLED.setBrightness(190);
  flashlight();
}

void flashlight50(){
  FastLED.setBrightness(128);
  flashlight();
}

void flashlight25(){
  FastLED.setBrightness(64);
  flashlight();
}

void flashlight10(){
  FastLED.setBrightness(25);
  flashlight();
}

void flashlight()
{
  Serial.println("Starting flashlight");
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    // Show the leds
    FastLED.show(); 
  }
}

void all(CRGB c)
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = c;
  }
  FastLED.show();
}

void all_h(byte h)
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CHSV(h,255,255);
  }
  FastLED.show();
}

void off()
{
  Serial.println("Turning Off");
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    // Show the leds
    FastLED.show(); 
  }
}


void green_rand()
{
  CRGB colors[24];
  for(int i=0;i<23;i++)
  {
    colors[i].setRGB(0,i*8,0); 
  }
  palette_rand(colors, 24, 200);
}

void red_rand()
{
  CRGB colors[24];
  for(int i=0;i<23;i++)
  {
    colors[i].setRGB(i*8,0,0); 
  }
  palette_rand(colors, 24, 200);
}

void blue_rand()
{
  CRGB colors[24];
  for(int i=0;i<23;i++)
  {
    colors[i].setRGB(0,0,i*8); 
  }
  palette_rand(colors, 24, 200);
}

void fire_rand()
{
  CRGB colors[] = {CRGB::Tomato, CRGB::Red, CRGB::OrangeRed, CRGB::Brown, CRGB::FireBrick, CRGB::Maroon};
  palette_rand(colors, 6, 200);
}

void water_rand()
{
  CRGB colors[] = {CRGB::Blue, CRGB::DarkBlue, CRGB::DarkTurquoise, CRGB::Aqua, CRGB::SeaGreen, CRGB::Aquamarine};
  palette_rand(colors, 6, 200);
}

void earth_rand()
{
  CRGB colors[] = {CRGB::Chartreuse, CRGB::Green, CRGB::DarkOliveGreen, CRGB::DarkGreen, CRGB::LimeGreen, CRGB::ForestGreen};
  palette_rand(colors, 6, 200);
}

void air_rand()
{
  CRGB colors[] = {CRGB::Azure, CRGB::Cyan, CRGB::DeepSkyBlue, CRGB::LightSkyBlue, CRGB::DodgerBlue, CRGB::Turquoise};
  palette_rand(colors, 6, 200);
}

void bw_rand()
{
  CRGB colors[] = {CRGB::Black, CRGB::White};
  palette_rand(colors, 2, 200);
}

void palette_rand(CRGB colors[], int colorCount,int wait)
{
    leds[random(0,NUM_LEDS)] = colors[random(0,colorCount-1)];
    FastLED.show();
    fadeall();
    FastLED.delay(wait); 
}


void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

void rainbow_cylon() { 
  static uint8_t hue = 0;
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show(); 
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }

  // Now go in the other direction.  
  for(int i = (NUM_LEDS)-1; i >= 0; i--) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
}

static uint8_t startIndex = 0;
void palette_fader(){
  ChangePalettePeriodically();

  // Crossfade current palette slowly toward the target palette
  //
  // Each time that nblendPaletteTowardPalette is called, small changes
  // are made to currentPalette to bring it closer to matching targetPalette.
  // You can control how many changes are made in each call:
  //   - the default of 24 is a good balance
  //   - meaningful values are 1-48.  1=veeeeeeeery slow, 48=quickest
  //   - "0" means do not change the currentPalette at all; freeze
  
  uint8_t maxChanges = 1; 
  nblendPaletteTowardPalette( currentPalette, targetPalette, maxChanges);


  
  startIndex = startIndex + 1; /* motion speed */
  FillLEDsFromPaletteColors( startIndex);
  FastLED.show();
}


void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  uint8_t brightness = 255;
  
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex + sin8(i*16), brightness);
    colorIndex += 3;
  }
}


void ChangePalettePeriodically()
{
  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    /*
    CRGB p = CHSV( HUE_PURPLE, 255, 255);
    CRGB g = CHSV( HUE_GREEN, 255, 255);
    CRGB b = CRGB::Black;
    CRGB w = CRGB::White;
    */
    if( secondHand == 0)   { targetPalette = OceanColors_p; }
    if( secondHand == 10)  { targetPalette = ForestColors_p; }
    if( secondHand == 20)  { targetPalette = OceanColors_p; }
    if( secondHand == 30)  { targetPalette = ForestColors_p; }    
    if( secondHand == 40)  { targetPalette = OceanColors_p; }
    if( secondHand == 50)  { targetPalette = ForestColors_p; }
    /*
    if( secondHand == 50)  { targetPalette = PartyColors_p; }
    if( secondHand == 10)  { targetPalette = CRGBPalette16( g,g,b,b, p,p,b,b, g,g,b,b, p,p,b,b); }
    if( secondHand == 20)  { targetPalette = CRGBPalette16( b,b,b,w, b,b,b,w, b,b,b,w, b,b,b,w); }
    if( secondHand == 30)  { targetPalette = LavaColors_p; }
    */

  }
}
