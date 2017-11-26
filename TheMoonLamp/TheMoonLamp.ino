#include <Adafruit_NeoPixel.h>
#include <Adafruit_SleepyDog.h>
#ifdef __AVR__
  //#include <avr/power.h>
#endif

#define VERSION "1.0a"

#define LEDS_PIN 3
#define NUM_LEDS 4
#define BRIGHTNESS 255

#define DBG true


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
  delay(10);
  unsigned long RawADC = 0;
  byte multisample = 5;
  for (byte b = 0; b<(1 << multisample); b++) {
    RawADC += analogRead(thermpin);
    //Serial.println(RawADC);
  }
  digitalWrite(thermpullup,LOW);
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
  pinMode(13,OUTPUT);
  pinMode(thermpullup,OUTPUT);
  digitalWrite(thermpullup,LOW);
}

bool btn(){
	  int newbtn = digitalRead(btnpin);
	  bool pressed = false;
    int tpressed = 0;
	  if ( newbtn == 0 && oldbtn == 1){
	    while((digitalRead(btnpin) ==0) && (tpressed <= 1000)){
	      tpressed++;
	      delay(1);
	    }
	    if (tpressed >= 1000){
        mode =6;
	    }else{
  	    if (mode == 6){
          mode = 0;
  	    }else{
  	      mode++;
          mode = mode %6;
  	    }
	    }
	    Serial.print("Button pressed, new mode = ");
	    Serial.println(mode);
	    //Thermistor(thermpin);
	    pressed = true;
	    }
	  oldbtn = newbtn;
	  return pressed;
}

uint32_t last_random_color = 0;
uint32_t random4[NUM_LEDS];

uint32_t interpolatergbw(uint32_t a, uint32_t b, int level){// Byte order is 0XWWBBRRGG
  level = min(level,255);
  level = max(0,level);
	//Linear Interpolation between two RGBW colors A and B, a level of 0 is A, a level of 255 is B
	uint32_t result = 0;
	uint8_t *result_pointer = (uint8_t *) & result;
	uint8_t *apointer = (uint8_t *) & a;
	uint8_t *bpointer = (uint8_t *) & b;
	for (byte i = 0; i < 4; i ++) result_pointer[i] = (uint8_t) (((255 - level) * apointer[i] + level * bpointer[i] )>>8) ;
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
	Serial.print('R');
	Serial.print(colorpointer[1]);
	Serial.print('G');
	Serial.print(colorpointer[0]);
	Serial.print('B');
	Serial.print(colorpointer[2]);
	Serial.print('W');
	Serial.println(colorpointer[3]);
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
  #if DBG
  Serial.flush();
  #endif
  digitalWrite(13,LOW);
  //delay(t);
  int sleepms = Watchdog.sleep(t);
  timeslept = timeslept + sleepms;
  digitalWrite(13,HIGH); 
  //Serial.print(sleepms);
  //Serial.println("ms");
  }

void black(int wait){
      for(uint16_t i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, BLACK);
      }
      if (btn()) return;
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
    strip.show();
		if (btn()) return;
		delayled(wait);
	}


}

void pulseWhite(uint8_t wait) {
  for(int j = 0; j < 256 ; j++){
      for(uint16_t i=0; i<NUM_LEDS; i++) {
          strip.setPixelColor(i, strip.Color(0,0,0, gamma(j) ) );
        }
        strip.show();
        delayled(wait);
        if (btn()) return;
      }

  for(int j = 255; j >= 0 ; j--){
      for(uint16_t i=0; i<NUM_LEDS; i++) {
          strip.setPixelColor(i, strip.Color(0,0,0, gamma(j) ) );
        }
        strip.show();
        delayled(wait);
        if (btn()) return;
      }
}


void pulsemap(int wait){
	for (int j = 1600; j< 2800; j += 1){
		uint32_t newcolor = colormap(j);
		//Serial.print(j);
		//Serial.print(" colormap: ");
		//printColor(newcolor);
		for (byte i = 0; i < NUM_LEDS ; i++){
			strip.setPixelColor(i,newcolor);
		}
		strip.show();
		delayled(wait);
		if (btn()) return;
	}
  for (int j = 2800; j> 1600; j -= 1){
    uint32_t newcolor = colormap(j);
    //Serial.print(j);
    //Serial.print(" colormap: ");
    //printColor(newcolor);
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
    for(uint16_t i=0; i<NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(255,255,255, 255 ) );
    }
    strip.show();
	  delayled(wait);
}
