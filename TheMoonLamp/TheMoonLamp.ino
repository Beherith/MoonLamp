#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 3

#define NUM_LEDS 4

#define BRIGHTNESS 255

const float temperature_offset = 7.0;

const int thermpin = A5;
const int btnpin = 2;
byte mode = 0;
int oldbtn = 1;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRBW + NEO_KHZ800);

float Thermistor(int thermpin) { //returns the celsius value in a float
  float RawADC = 0.0;
  byte multisample = 64;
  for (byte b =0; b < multisample; b++){
    RawADC += analogRead(thermpin);
  }
  RawADC = RawADC/multisample;
  Serial.print(F("RawADC:"));
  Serial.print(thermpin);
  Serial.print(' ');
  Serial.print(RawADC);
  #define PullupR 5100.0
  long Resistance;
  float Temp;  // Dual-Purpose variable to save space.
  Resistance = PullupR * (RawADC/1024.0) / (1-RawADC/1024.0); //changed cause i use thermistors the other way around
  Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius
  //Temp = Temp;
  Serial.print(" Temperature =");
  Serial.println(Temp);
  return Temp;                                      // Return the Temperature
}

byte gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };


void setup() {
  Serial.begin(115200);
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(btnpin,INPUT_PULLUP);
  delay(500);
  //Serial.print(Thermistor(thermpin));
  mode = 1;
}

int btn(){
	  int newbtn = digitalRead(btnpin);
	  if ( newbtn == 0 && oldbtn == 1){
	    mode++;
	    mode = mode % 6;
	    Serial.print("Button pressed, new mode =");
	    Serial.println(mode);
	    Thermistor(thermpin);
	    }
	  oldbtn = newbtn;
	  return mode;
}

uint32_t ra = 0;
uint32_t rb = 0;


void loop() {
  Serial.print("Mode: ");
  Serial.println(mode);
  Thermistor(thermpin);
  // Some example procedures showing how to display to the pixels:
  if (mode == 0){
	  for(uint16_t i=0; i<strip.numPixels(); i++) {
		  uint32_t nc =  interpolaterbgw( strip.Color(255,255,0,10), strip.Color(0,255,255,10),  (millis()/20)%255 ) ;
		  Serial.println(nc,HEX);
		  strip.setPixelColor(i,nc );
		  btn();
	  }
	  strip.show();

  }
  if (mode == 1){

	  //uint32_t ra = strip.Color(random(),random(),random(),random());
	  rb = strip.Color(random(),random(),random(),random());
	  Serial.print(rb,HEX);
	  Serial.print('#');
	  Serial.println(ra,HEX);
	  for (byte t= 0; t <255; t++){
		  for(uint16_t i=0; i<strip.numPixels(); i++) {
			  uint32_t nc =  interpolaterbgw( ra,rb,  t ) ;

			  strip.setPixelColor(i,nc );
			  btn();
		  }
		  //Serial.println(t);
		  strip.show();
		  delay(20);
	  }
	  Serial.println('boop');
	  ra = rb;

  }


  if (mode == 5)   whiteOverRainbow(200,75,5);
  if (mode == 6){

    colorWipe(strip.Color(255, 0, 0), 50); // Red
    colorWipe(strip.Color(0, 255, 0), 50); // Green
    colorWipe(strip.Color(0, 0, 255), 50); // Blue
    colorWipe(strip.Color(0, 0, 0, 255), 50); // White
  }
  if (mode == 2) pulseWhite(5);

  if (mode == 3) fullWhite();

  if (mode == 4) rainbowFade2White(30,3,1);


}


void rotatecolors(float a){



}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
	  btn();
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void pulseWhite(uint8_t wait) {
  for(int j = 0; j < 256 ; j++){
      for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(0,0,0, gamma[j] ) );
          btn();
        }
        delay(wait);
        strip.show();
      }

  for(int j = 255; j >= 0 ; j--){
      for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(0,0,0, gamma[j] ) );
          btn();
        }
        delay(wait);
        strip.show();
      }
}

uint32_t interpolaterbgw(uint32_t a, uint32_t b, byte level){
  //Linear Interpolation bewteen two RGBW colors A and B, a level of 0 is A, a level of 255 is B
  //WRGB #define NEO_GRBW ((3 << 6) | (1 << 4) | (0 << 2) | (2))
	// R = 1 , G = 0, B = 2, W = 3
	uint32_t result = 0;
	uint8_t *result_pointer = (uint8_t *) & result;
	uint8_t *apointer = (uint8_t *) & a; 
	uint8_t *bpointer = (uint8_t *) & b;

	for (byte i = 0; i < 4; i ++){
		result_pointer[i] = (uint8_t) (((255 - level) * apointer[i] + level * bpointer[i] )>>8) ;
	}
	return result;
}


void showtemperature(float intemp){



}

#define VIOLET strip.Color(128,0,255,0);
#define BLUE strip.Color(0,0,255,0);
#define CYAN strip.Color(0,255,255,0);
#define GREEN strip.Color(0,255,0,0);
#define YELLOW strip.Color(255,255,0,0);
#define RED strip.Color(255,0,0,0);
#define WARM_WHITE strip.Color(255,50,10,255);
#define COOL_WHITE strip.Color(255,0,0,255);

uint32_t colormap(float p){ //p goes from 0 to 1
	if (p < 0.1){ //Deep violet
   


	}else if (p < 0.3){

	}else if (p < 0.3){

	}else if (p < 0.3){

	}else if (p < 0.3){

	}else if (p < 0.3){

	}else if (p < 0.3){

	}else if (p < 0.3){

	}else if (p < 0.3){

	}

}

void rainbowFade2White(uint8_t wait, int rainbowLoops, int whiteLoops) {
  float fadeMax = 100.0;
  int fadeVal = 0;
  uint32_t wheelVal;
  int redVal, greenVal, blueVal;

  for(int k = 0 ; k < rainbowLoops ; k ++){

    for(int j=0; j<256; j++) { // 5 cycles of all colors on wheel

      for(int i=0; i< strip.numPixels(); i++) {

        wheelVal = Wheel(((i * 256 / strip.numPixels()) + j) & 255);

        redVal = red(wheelVal) * float(fadeVal/fadeMax);
        greenVal = green(wheelVal) * float(fadeVal/fadeMax);
        blueVal = blue(wheelVal) * float(fadeVal/fadeMax);

        strip.setPixelColor( i, strip.Color( redVal, greenVal, blueVal ) );

      }

      //First loop, fade in!
      if(k == 0 && fadeVal < fadeMax-1) {
          fadeVal++;
      }

      //Last loop, fade out!
      else if(k == rainbowLoops - 1 && j > 255 - fadeMax ){
          fadeVal--;
      }

        strip.show();
        delay(wait);
    }

  }



  delay(500);


  for(int k = 0 ; k < whiteLoops ; k ++){

    for(int j = 0; j < 256 ; j++){

        for(uint16_t i=0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(0,0,0, gamma[j] ) );
          }
          strip.show();
        }

        delay(2000);
    for(int j = 255; j >= 0 ; j--){

        for(uint16_t i=0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(0,0,0, gamma[j] ) );
          }
          strip.show();
        }
  }

  delay(500);


}

void whiteOverRainbow(uint8_t wait, uint8_t whiteSpeed, uint8_t whiteLength ) {

  if(whiteLength >= strip.numPixels()) whiteLength = strip.numPixels() - 1;

  int head = whiteLength - 1;
  int tail = 0;

  int loops = 3;
  int loopNum = 0;

  static unsigned long lastTime = 0;


  while(true){
    for(int j=0; j<256; j++) {
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        if((i >= tail && i <= head) || (tail > head && i >= tail) || (tail > head && i <= head) ){
          strip.setPixelColor(i, strip.Color(0,0,0, 255 ) );
        }
        else{
          strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }

      }

      if(millis() - lastTime > whiteSpeed) {
        head++;
        tail++;
        if(head == strip.numPixels()){
          loopNum++;
        }
        lastTime = millis();
      }

      if(loopNum == loops) return;

      head%=strip.numPixels();
      tail%=strip.numPixels();
        strip.show();
        delay(wait);
    }
  }

}
void fullWhite() {

    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(255,255,255, 255 ) );
    }
      strip.show();
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256 * 5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3,0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3,0);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0,0);
}

uint8_t red(uint32_t c) {
  return (c >> 8);
}
uint8_t green(uint32_t c) {
  return (c >> 16);
}
uint8_t blue(uint32_t c) {
  return (c);
}

