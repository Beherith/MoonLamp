#include <Adafruit_NeoPixel.h>
#include <Adafruit_SleepyDog.h>
#ifdef __AVR__
  //#include <avr/power.h>
#endif

#define VERSION "1.0a"

#define LEDS_PIN 3
#define NUM_LEDS 4
#define BRIGHTNESS 255

#define DBG


const int temperature_offset = 700;

const int thermpin = A5;
const int thermpullup = A2;
const int btnpin = 2;
byte mode = 0;
int oldbtn = 1;
long timeslept = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LEDS_PIN, NEO_GRBW + NEO_KHZ800);
/*
float Thermistor(int thermpin) { //returns the celsius value in a float
  float RawADC = 0.0;
  byte multisample = 64;
  for (byte b =0; b < multisample; b++){
    RawADC += analogRead(thermpin);
  }
  RawADC = RawADC/multisample;
  //Serial.print(F("RawADC pin= "));
  //Serial.print(thermpin);
  //Serial.print(' value = ');
  //Serial.print(RawADC);
  #define PullupR 10000.0
  long Resistance;
  float Temp;  // Dual-Purpose variable to save space.
  Resistance = PullupR * (RawADC/1024.0) / (1-RawADC/1024.0); //changed cause i use thermistors the other way around
  Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius
  //Temp = Temp;
  Serial.print("Temperature=\t");
  Serial.print(Temp);
  Serial.print("\t Time=\t");
  Serial.println(millis()/1000);
  return Temp;                                      // Return the Temperature
}*/

int Thermistor(int thermpin){
  digitalWrite(thermpullup,HIGH);
  delay(1);
  unsigned long RawADC = 0;
  byte multisample = 5;
  for (byte b = 0; b<(1 << multisample); b++) {
    RawADC += analogRead(thermpin);
    //Serial.println(RawADC);
  }
  digitalWrite(thermpullup,LOW);
  #define PullupR 10000
  long Resistance;
  RawADC = RawADC>>multisample;
  //float Temp;  // Dual-Purpose variable to save space.
  //Resistance = PullupR * (RawADC/1023.0) / (1-RawADC/1023.0); //changed cause i use thermistors the other way around
  Resistance = (PullupR * RawADC) / ((1023)-RawADC); //changed cause i use thermistors the other way around
  //resistance to
  Serial.print("ADC = \t"); 
  Serial.print(RawADC);
  Serial.print(" R=\t");
  Serial.print(Resistance);
  Serial.print(" T=\t");
  Serial.println(r2t(Resistance));
  return r2t(Resistance);
} 

int r2t(int r){ //returns the temperature given the resistance, for a 10k thermistor with B = 3950K
  if (r>19690) return map(r,19690,32000,1000,   0); //<10C
  if (r>15618) return map(r,15618,19690,1500,1000); //<15C
  if (r>12474) return map(r,12474,15618,2000,1500); //<20C
  if (r>10000) return map(r,10000,12474,2500,2000); //<25C
  if (r>8080 ) return map(r, 8080,10000,3000,2500); //<30C
  if (r>6569 ) return map(r, 6569, 8080,3500,3000); //<35C
  if (r>5370 ) return map(r, 5370, 6569,4000,3500); //<40C
  return 0;
}

int temperature_runtime_corrector(int temperature_in){
	//if (millis() < 1000) // first 5
		return 1;
}
byte gamma(int k){
	return k*k >> 8;
}



void setup() {
  Serial.begin(115200);
  Serial.print(F("The Moon, Version "));
  Serial.println(VERSION);
  Serial.println(F("Source: https://github.com/Beherith/MoonLamp"));
  //strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(btnpin,INPUT_PULLUP);
  delay(500);
  Thermistor(thermpin);
  mode = 5;
  pinMode(13,OUTPUT);
  pinMode(thermpullup,OUTPUT);
  digitalWrite(thermpullup,LOW);
}

#define BTN (if(btn())break;)
bool btn(){
	  int newbtn = digitalRead(btnpin);
	  bool pressed = false;
	  if ( newbtn == 0 && oldbtn == 1){
	    mode++;
	    mode = mode % 6;
	    Serial.print("Button pressed, new mode =");
	    Serial.println(mode);
	    Thermistor(thermpin);
	    pressed = true;
	    }
	  oldbtn = newbtn;
	  return pressed;
}

uint32_t last_random_color = 0;
uint32_t random4[NUM_LEDS];

uint32_t interpolatergbw(uint32_t a, uint32_t b, int level){// Byte order is 0XWWBBRRGG
  //Serial.print(level);

  level = min(level,255);
  level = max(0,level);
	//Linear Interpolation between two RGBW colors A and B, a level of 0 is A, a level of 255 is B
	uint32_t result = 0;
	uint8_t *result_pointer = (uint8_t *) & result;
	uint8_t *apointer = (uint8_t *) & a;
	uint8_t *bpointer = (uint8_t *) & b;
	for (byte i = 0; i < 4; i ++) result_pointer[i] = (uint8_t) (((255 - level) * apointer[i] + level * bpointer[i] )>>8) ;
  //Serial.print(' ');
  //Serial.println(result,HEX);
	return result;
}

void loop() {
  Serial.print("Mode: ");
  Serial.print(mode);
  Serial.print(" Time Slept = ");
  Serial.println(timeslept);
  int temperature = Thermistor(thermpin);
  btn();

  //The part that decides what we show on the pixels:
  if (mode == 0){
	  randomColor(20);
  }
  if (mode == 1){
	  //pulseWhite(20);
	  smoothWhitePulse(20);
  }
  if (mode == 2) pulsemap(Thermistor(thermpin));

  if (mode == 3) randomColor4(20);

  if (mode == 4) smoothWhitePulse(50);

  if (mode ==5) showtemperature(temperature -600);
}

void printColor(uint32_t color){
	uint8_t *colorpointer = (uint8_t *) & color;
	Serial.print('R');
	Serial.print(colorpointer[1]);
	Serial.print('G');
	Serial.print(colorpointer[0]);
	Serial.print('B');
	Serial.print(colorpointer[2]);
	Serial.print('W');
	Serial.println(colorpointer[3]);
}

uint32_t expRandomColor(){
  byte r1 = random();
  byte r2 = random();
  byte r3 = random();
  byte r4 = random();
  return strip.Color(r1*r1 >> 8, r2*r2 >>8, r3*r3 >> 8, r4*r4 >> 10);
}

void delayled(int t){ //we can pretty much choose between 30ms and 15ms. 
  Serial.flush();
  digitalWrite(13,LOW);
  //delay(t);
  int sleepms = Watchdog.sleep(t);
  timeslept = timeslept + sleepms;
  digitalWrite(13,HIGH); 
  //Serial.print(sleepms);
  //Serial.println("ms");
  }

void randomColor(int wait){
	  uint32_t new_random_color = expRandomColor();
	  for (byte level = 0; level <255; level++){
		  uint32_t mixed_color =  interpolatergbw(last_random_color, new_random_color, level) ;
		  for(uint16_t i = 0; i < NUM_LEDS; i++) {
			  strip.setPixelColor(i, mixed_color);
		  }
		  if (btn()) return;
		  strip.show();
		  delayled(wait);
	  }
	  last_random_color = new_random_color;
}


void randomColor4(int wait){
	  uint32_t new_random4[NUM_LEDS];
	  for (byte i = 0; i < NUM_LEDS; i++) new_random4[i] = expRandomColor();
	  for (byte level = 0; level <255; level++){
		  for(uint16_t i = 0; i < NUM_LEDS; i++) {
			  uint32_t mixed_color =  interpolatergbw(random4[i], new_random4[i], level) ;
			  strip.setPixelColor(i, mixed_color);
		  }
		  if (btn()) return;
		  strip.show();
		  delayled(wait);
	  }

	  for (byte i = 0; i < NUM_LEDS; i++) random4[i] = new_random4[i];
}


void rotatecolors(int a){
	return;
}


void smoothWhitePulse(int wait){
	uint16_t minbrightness = 16;
	for(uint16_t j = minbrightness; j <= 255; j++){

		for(uint16_t i=0; i<NUM_LEDS; i++) {
		  int brightness = (j*j+i*64) >> 8;
		  strip.setPixelColor(i, interpolatergbw(0, strip.Color(128,32,16, 255), brightness ));
		}
		if (btn()) return;
		delayled(wait);
		strip.show();
	}
	for(uint16_t j = 255; j > minbrightness; j--){
		for(uint16_t i=0; i<NUM_LEDS; i++) {
		  int brightness = (j*j + i*64)>> 8;
		  //Serial.print(brightness);
		  //Serial.print(' ');
		  strip.setPixelColor(i, interpolatergbw(0, strip.Color(128,32,16, 255), brightness ));
		}
		//Serial.println(' ');
		if (btn()) return;
		delayled(wait);
		strip.show();
	}


}

void pulseWhite(uint8_t wait) {
  for(int j = 0; j < 256 ; j++){
      for(uint16_t i=0; i<NUM_LEDS; i++) {
          strip.setPixelColor(i, strip.Color(0,0,0, gamma(j) ) );
          btn();
        }
        delayled(wait);
        strip.show();
      }

  for(int j = 255; j >= 0 ; j--){
      for(uint16_t i=0; i<NUM_LEDS; i++) {
          strip.setPixelColor(i, strip.Color(0,0,0, gamma(j) ) );
          btn();
        }
        delayled(wait);
        strip.show();
      }
}


void pulsemap(int intemp){
	for (int j = 1600; j< 2800; j += 1){
		uint32_t newcolor = colormap(j);
		//Serial.print(j);
		//Serial.print(" colormap: ");
		//printColor(newcolor);
		for (byte i = 0; i < NUM_LEDS ; i++){
			strip.setPixelColor(i,newcolor);
		}
		strip.show();
		delayled(20);
		if (btn()) return;
	}
	return;
}

void showtemperature(int intemp){
	for (byte i = 0; i < NUM_LEDS ; i++){
		strip.setPixelColor(i,colormap(intemp));
	}
  strip.show();
  if(btn()) return;
  delayled(500);
	return;
}
#define VIOLET strip.Color(128,0,255,10)
#define BLUE strip.Color(0,0,255,10)
#define CYAN strip.Color(0,128,255,10)
#define GREEN strip.Color(0,255,0,10)
#define YELLOW strip.Color(255,128,0,10)
#define RED strip.Color(255,0,0,10)
#define COOL_WHITE strip.Color(255,50,10,255)
#define WARM_WHITE strip.Color(255,0,0,255)


uint32_t colormap(unsigned int temperature){ 
	if (temperature < 1700){ //Deep violet
		return VIOLET;
	}else if (temperature < 1900){
		return interpolatergbw(BLUE, VIOLET , ((1900 - temperature) * 256 )/200);
	}else if (temperature < 2100){
		return interpolatergbw(CYAN, BLUE, ((2100 - temperature) * 256)/200);
	}else if (temperature < 2300){
		return interpolatergbw(YELLOW, CYAN , (2300 - temperature) * 256/200);
	}else if (temperature < 2500){
		return interpolatergbw(RED, YELLOW , (2500 - temperature) * 256/200);
	}else if (temperature < 2700){
		return interpolatergbw(WARM_WHITE, RED , (2700 - temperature) * 256/200);
	}else {
		return WARM_WHITE;
	}
	return 0;
}
/*
void rainbowFade2White(uint8_t wait, int rainbowLoops, int whiteLoops) {
  float fadeMax = 100.0;
  int fadeVal = 0;
  uint32_t wheelVal;
  int redVal, greenVal, blueVal;

  for(int k = 0 ; k < rainbowLoops ; k ++){

    for(int j=0; j<256; j++) { // 5 cycles of all colors on wheel

      for(int i=0; i< NUM_LEDS; i++) {

        wheelVal = Wheel(((i * 256 / NUM_LEDS) + j) & 255);

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

        for(uint16_t i=0; i < NUM_LEDS; i++) {
            strip.setPixelColor(i, strip.Color(0,0,0, gamma(j) ) );
          }
          strip.show();
        }

        delay(2000);
    for(int j = 255; j >= 0 ; j--){

        for(uint16_t i=0; i < NUM_LEDS; i++) {
            strip.setPixelColor(i, strip.Color(0,0,0, gamma(j) ) );
          }
          strip.show();
        }
  }

  delay(500);


}
*/
/*

void whiteOverRainbow(uint8_t wait, uint8_t whiteSpeed, uint8_t whiteLength ) {

  if(whiteLength >= NUM_LEDS) whiteLength = NUM_LEDS - 1;

  int head = whiteLength - 1;
  int tail = 0;

  int loops = 3;
  int loopNum = 0;

  static unsigned long lastTime = 0;


  while(true){
    for(int j=0; j<256; j++) {
      for(uint16_t i=0; i<NUM_LEDS; i++) {
        if((i >= tail && i <= head) || (tail > head && i >= tail) || (tail > head && i <= head) ){
          strip.setPixelColor(i, strip.Color(0,0,0, 255 ) );
        }
        else{
          strip.setPixelColor(i, Wheel(((i * 256 / NUM_LEDS) + j) & 255));
        }

      }

      if(millis() - lastTime > whiteSpeed) {
        head++;
        tail++;
        if(head == NUM_LEDS){
          loopNum++;
        }
        lastTime = millis();
      }

      if(loopNum == loops) return;

      head%=NUM_LEDS;
      tail%=NUM_LEDS;
        strip.show();
        delay(wait);
    }
  }

}
*/
void fullWhite() {

    for(uint16_t i=0; i<NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(255,255,255, 255 ) );
    }
      strip.show();
}
/*

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
*/
