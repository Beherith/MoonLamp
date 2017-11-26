//SET USE_SLEEP TO 0 IF ATTINY85, else keep it at 1
#define USE_SLEEP 1

#include <Adafruit_NeoPixel.h>
#if USE_SLEEP
  #include <Adafruit_SleepyDog.h>
#endif
#ifdef __AVR__
  //#include <avr/power.h>
#endif

#define VERSION "1.0a"

#define NUM_LEDS 4
#define BRIGHTNESS 255
#if USE_SLEEP
  #define DBG 1
#else
  #define DBG 0
#endif

#if USE_SLEEP
#define LEDS_PIN 3
const byte thermpin = A5;
const byte thermpullup = A2;
const byte btnpin = 2;
#else

#define LEDS_PIN 2
const byte thermpin = 0;// oddly enough, this is P5
const byte btnpin = 1;
#endif 

byte mode = 0;
bool oldbtn = 1;
long timeslept = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LEDS_PIN, NEO_GRBW + NEO_KHZ800);

int Thermistor(int thermpin){
  #if USE_SLEEP
    digitalWrite(thermpullup,HIGH);
    delay(10);
  #endif
  unsigned long RawADC = 0;
  byte multisample = 5;
  for (byte b = 0; b<(1 << multisample); b++) {
    RawADC += analogRead(thermpin);
    //Serial.println(RawADC);
  }
  #if USE_SLEEP
    digitalWrite(thermpullup,LOW);
  #endif
  #define PullupR 10000
  long Resistance;
  //RawADC = RawADC>>multisample;
  //float Temp;  // Dual-Purpose variable to save space.
  //Resistance = PullupR * (RawADC/1023.0) / (1-RawADC/1023.0); //changed cause i use thermistors the other way around
  Resistance = (PullupR * RawADC) / ((1023<<multisample)-RawADC); //changed cause i use thermistors the other way around
  //resistance to
  int temperature = r2t(Resistance) -100;
  #if DBG
  Serial.print("VADC = \t");
  Serial.print( (RawADC*5)>>multisample); 
  Serial.print("\tADC = \t"); 
  Serial.print(RawADC);
  Serial.print(" R=\t");
  Serial.print(Resistance);
  Serial.print(" T=\t");
  Serial.println(temperature);
  #endif
  return temperature;
} 

#define VIOLET strip.Color(128,0,255,10)
#define BLUE strip.Color(0,0,255,10)
#define CYAN strip.Color(0,128,255,10)
#define GREEN strip.Color(0,255,0,10)
#define YELLOW strip.Color(255,128,0,10)
#define RED strip.Color(255,0,0,10)
#define COOL_WHITE strip.Color(255,50,10,255)
#define WARM_WHITE strip.Color(255,0,0,255)
#define BLACK strip.Color(0,0,0,0)


//Byte order is 0XWWBBRRGG
/*
#define VIOLET  0x0AFF8000 //strip.Color(128,0,255,10)
#define BLUE    0x0AFF0000 //strip.Color(0,0,255,10)
#define CYAN    0x0AFF0080 //strip.Color(0,128,255,10)
#define GREEN   0x0A0000FF //strip.Color(0,255,0,10)
#define YELLOW  0x0A00FF80 //strip.Color(255,128,0,10)
#define RED     0x0A00FF00 //strip.Color(255,0,0,10)
#define COOL_WHITE 0xFF0AFF40 //strip.Color(255,50,10,255)
#define WARM_WHITE 0xFF00FF00 //strip.Color(255,0,0,255)
#define BLACK   0x00000000     //strip.Color(0,0,0,0)
*/

int r2t(int r){ //returns the temperature given the resistance, for a 10k thermistor with B = 3950K
  //if (r>19690) return map(r,19690,32000,1000,   0); //<10C
  if (r>19690) return 1000;
  if (r>15618) return map(r,15618,19690,1500,1000); //<15C
  if (r>12474) return map(r,12474,15618,2000,1500); //<20C
  if (r>10000) return map(r,10000,12474,2500,2000); //<25C
  if (r>8080 ) return map(r, 8080,10000,3000,2500); //<30C
  return 3000;
  //if (r>6569 ) return map(r, 6569, 8080,3500,3000); //<35C
  //if (r>5370 ) return map(r, 5370, 6569,4000,3500); //<40C
}

byte gamma(unsigned int k){
	return k*k >> 8;
}


void setup() {
  #if DBG
    Serial.begin(115200);
    Serial.print(F("The Moon, Version "));
    Serial.println(VERSION);
    Serial.println(F("Source: https://github.com/Beherith/MoonLamp"));
  #endif
  //strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(btnpin,INPUT_PULLUP);
  delay(500);
  Thermistor(thermpin);
  mode = 5;
  //pinMode(13,OUTPUT);
  #if USE_SLEEP
    pinMode(thermpullup,OUTPUT);
    digitalWrite(thermpullup,LOW);
  #endif
}

bool btn(){
	  bool newbtn = digitalRead(btnpin);
	  bool pressed = false;
    byte tpressed = 0;
	  if ( newbtn == 0 && oldbtn == 1){
	    while((digitalRead(btnpin) ==0) && (tpressed <= 100)){
	      tpressed++;
	      delay(10);
	    }
	    if (tpressed >= 100){
        mode =6;
	    }else{
  	    if (mode == 6){
          mode = 0;
  	    }else{
  	      mode++;
          if (mode >= 6) mode = 0;
          //mode = mode %6;
  	    }
	    }
      #if DBG
	    Serial.print("Button pressed, new mode = ");
	    Serial.println(mode);
      #endif
	    //Thermistor(thermpin);
	    pressed = true;
	    }
	  oldbtn = newbtn;
	  return pressed;
}

uint32_t last_random_color = 0;
uint32_t random4[NUM_LEDS];

uint32_t interpolatergbw(uint32_t a, uint32_t b, int level){// Byte order is 0XWWBBRRGG
  level = min(max(0,level),255);
  //Linear Interpolation between two RGBW colors A and B, a level of 0 is A, a level of 255 is B
	uint32_t result = 0;
	uint8_t *result_pointer = (uint8_t *) & result;
	uint8_t *apointer = (uint8_t *) & a;
	uint8_t *bpointer = (uint8_t *) & b;
	for (byte i = 0; i < 4; i ++) result_pointer[i] = (uint8_t) (((255 - level) * apointer[i] + level * bpointer[i] )>>8) ;
	return result;
}

void loop() {
  #if DBG
  Serial.print("Mode: ");
  Serial.print(mode);
  Serial.print(" Time Slept = ");
  Serial.println(timeslept);
  #endif
  int temperature = Thermistor(thermpin);
  btn();

  //The part that decides what we show on the pixels:
  if (mode == 0) randomColor(30);
  
  if (mode == 1) smoothWhitePulse(20);
   
  if (mode == 2) pulsemap(50);

  if (mode == 3) randomColor4(20);

  if (mode == 4) fullWhite(100);

  if (mode == 5) showtemperature(temperature);

  if (mode == 6) black(200);
}

void printColor(uint32_t color){
	uint8_t *colorpointer = (uint8_t *) & color;
  #if DBG
	Serial.print('R');
	Serial.print(colorpointer[1]);
	Serial.print('G');
	Serial.print(colorpointer[0]);
	Serial.print('B');
	Serial.print(colorpointer[2]);
	Serial.print('W');
	Serial.println(colorpointer[3]);
  #endif 
}

void randomBrightColor(int wait){
    uint32_t new_random_color = colormap(random(1700,2750));
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


uint32_t expRandomColor(){
  byte r1 = gamma(random(0,255));
  byte r2 = gamma(random(0,255));
  byte r3 = gamma(random(0,255));
  byte r4 = min(r1,min(r2,r3))>>1;
  return strip.Color(r1,r2,r3,r4);
}

void delayled(int t){ //we can pretty much choose between 30ms and 15ms. 
  #if USE_SLEEP
    #if DBG
    Serial.flush();
    #endif
    digitalWrite(13,LOW);
    //delay(t);
    int sleepms = Watchdog.sleep(t);
    timeslept = timeslept + sleepms;
    digitalWrite(13,HIGH); 
    //Serial.print(sleepms);
  #else
    delay(t);
  #endif
  //Serial.println("ms");
  }

void black(int wait){
      for(uint16_t i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, BLACK);
      }
      //if (btn()) return;
      strip.show();
      delayled(wait);
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


void smoothWhitePulse(int wait){
	uint16_t minbrightness = 16;
	for(uint16_t j = minbrightness; j <= 494; j++){
    unsigned int k = j;
    if (j>255) k = 510-j;
		for(uint16_t i=0; i<NUM_LEDS; i++) {
		  int brightness = (k*k+i*64) >> 8;
		  strip.setPixelColor(i, interpolatergbw(0, strip.Color(128,32,16, 255), brightness ));
		}
		if (btn()) return;
		delayled(wait);
		strip.show();
	}
}

void pulsemap(int wait){
	for (int j = 0; j< 2000; j += 1){
		uint32_t newcolor = colormap(1600+abs(1000-j));
    //Serial.println(1600+abs(1000-j));
		for (byte i = 0; i < NUM_LEDS ; i++){
			strip.setPixelColor(i,newcolor);
		}
		strip.show();
		delayled(wait);
		if (btn()) return;
	}
	return;
}

void showtemperature(int intemp){
	for (byte i = 0; i < NUM_LEDS ; i++){
		strip.setPixelColor(i,colormap(intemp));
	}
  strip.show();
  for (byte i = 0; i< 10;i++){
    if(btn()) return;
    delayled(50);
  }
	return;
}

uint32_t colormap(unsigned long int temperature){ 
	if (temperature < 1700){ //Deep violet
		return VIOLET;
	}else if (temperature < 1900){
		return interpolatergbw(BLUE, VIOLET , ((1900 - temperature) * 256 )/200);
	}else if (temperature < 2100){
		return interpolatergbw(CYAN, BLUE, ((2100 - temperature) * 256)/200);
  }else if (temperature < 2200){
    return interpolatergbw(GREEN, CYAN , (2200 - temperature) * 256/100);
  }else if (temperature < 2300){
    return interpolatergbw(YELLOW, GREEN , (2300 - temperature) * 256/100);
	}else if (temperature < 2500){
		return interpolatergbw(RED, YELLOW , (2500 - temperature) * 256/200);
	}else if (temperature < 3000){  
		return interpolatergbw(WARM_WHITE, RED , (3000 - temperature) * 256/500);
	}else {
		return WARM_WHITE;
	}
	return 0;
}

void fullWhite(int wait) {
    for(byte i=0; i<NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(255,255,255, 255 ) );
    }
    strip.show();
	  delayled(wait);
}
