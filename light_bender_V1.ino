/*
 * Light Bender - version 1
 * 17/03/2018
 * By: Chris Mitchell (Maken it so)
 * 
 * https://www.youtube.com/c/Makenitso
 */

 
//NEOPIXEL start
  #include <Adafruit_NeoPixel.h>
  #ifdef __AVR__
    #include <avr/power.h>
  #endif
  #define PIN 4
  Adafruit_NeoPixel strip = Adafruit_NeoPixel(9, PIN, NEO_GRB + NEO_KHZ800);
//NEOPIXEL end

//OLED start
  #include <SPI.h>
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
  #define OLED_RESET 4
  Adafruit_SSD1306 display(OLED_RESET);
//OLED end

int LDR0 = A0;
int LDR1 = A1;
int LDR2 = A2;
int LDR3 = A3;
int LDR4 = A4;
int LDR5 = A5;
int LDR6 = A6;
int LDR7 = A7;
int LDR8 = A8;

int PROX_OUT = 24;
int PROX_IN = A13;

int buzzerPin = 11;

int laser_red = 13;
int laser_green = 15;
int laser_blue = 17;

#define NOTE_C1  33
#define NOTE_C3  131
#define NOTE_G3  196

int LDRS[] = {
  LDR0, LDR1, LDR2, LDR3, LDR4, LDR5, LDR6, LDR7, LDR8
};

int hit_val[] = {950,900,650};  // set light sensitivity for each laser

int level = 0;
int score = 0;
int countdown = 0;
int countdown_amount = 21;
int ticksound_bump = 0;
bool swipe_flip = false;

bool in_game = false;
unsigned long this_time;
unsigned long last_time;
int target_num = 0;
int target_color = 0;

void setup() {
  delay(100);
  pinMode(buzzerPin, OUTPUT);
  pinMode(LDR0, INPUT);
  pinMode(LDR1, INPUT);
  pinMode(LDR2, INPUT);
  pinMode(LDR3, INPUT);
  pinMode(LDR4, INPUT);
  pinMode(LDR5, INPUT);
  pinMode(LDR6, INPUT);
  pinMode(LDR7, INPUT);
  pinMode(LDR8, INPUT);
  pinMode(PROX_OUT, OUTPUT);
  pinMode(PROX_IN, INPUT);
  pinMode(laser_red, OUTPUT);
  pinMode(laser_green, OUTPUT);
  pinMode(laser_blue, OUTPUT);
  lasers_off();
  
  randomSeed(analogRead(16));
  Serial.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //OLED
  display.clearDisplay(); //OLED
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(20,16);
  display.print("LIGHT");
  display.setTextSize(3);
  display.setCursor(10,42);
  display.print("BENDER");

  
  display.display();
  
  strip.begin();
  colorOff();
  
  last_time = millis();
  delay(500);
}

// G, R, B

void loop(){
    // ******************************************************************************************************
  if(in_game){
    if(countdown>0){
      this_time = millis();
      if(this_time>last_time+1000){
        game_tick();
        last_time = this_time;
        countdown=countdown-1;
        update_display();
      }
    }else{
      loose();
    }
  }else{
    //check prox sensor for game start
    this_time = millis();
    if(this_time>last_time+500){
        last_time = this_time;
        swipe_flip=!swipe_flip;
        if(swipe_flip){
          display.clearDisplay(); //OLED
          display.setTextSize(3);
          display.setTextColor(WHITE);
          display.setCursor(20,16);
          display.print("SWIPE");
          display.setTextSize(3);
          display.setCursor(10,42);
          display.print("> > > > >");
          display.display();
        }else{
          display.clearDisplay(); //OLED
          display.setTextSize(3);
          display.setTextColor(WHITE);
          display.setCursor(20,16);
          display.print("SWIPE");
          display.setTextSize(3);
          display.setCursor(10,42);
          display.print(" > > > >");
          display.display();
        }
    }
    digitalWrite(PROX_OUT, HIGH);
    delay(50);
    int prox_val = analogRead(PROX_IN);
    if(prox_val<700){
      digitalWrite(PROX_OUT, LOW);
      game_start();
    }
  }
  // ******************************************************************************************************
}

void game_tick(){
  switch(countdown){
    case 6: ticksound_bump=ticksound_bump+3; break;
    case 5: ticksound_bump=ticksound_bump+5; break;
    case 4: ticksound_bump=ticksound_bump+5; break;
    case 3: ticksound_bump=ticksound_bump+10; break;
    case 2: ticksound_bump=ticksound_bump+10; break;
    case 1: ticksound_bump=ticksound_bump+15; break;
  }
  tone(buzzerPin, NOTE_C3+ticksound_bump, 50);
  delay(50);
  noTone(buzzerPin);
  bool target_hit = false;
  for(int a=0; a<10; a++){
    int res = analogRead( LDRS[a] );
    if( target_num==a ){
      if(res>hit_val[target_color]){target_hit=true;}
    }
  }
  if(target_hit){
    win();
  }
}

void game_start(){
  in_game = true;
  level=0;
  score=0;
  ticksound_bump=0;
  countdown=countdown_amount;
  sfx1();sfx1();sfx1();
  set_board();
}

void level_up(){
  level++;
  if(level>15){
    countdown_amount--;
    if(countdown_amount<6){countdown_amount=6;}
  }
  countdown=countdown_amount;
  set_board();
}

void win(){
  score = score + countdown;
  lasers_off();
  colorOff();
  for(int a=50; a<800; a=a+10){
    tone(buzzerPin, a, 5);
    for(int i=0; i<10; i++){
      int lval = a/4-40;
      if(lval<0){lval=0;}
      if(lval>255){lval=255;}
      strip.setPixelColor(i, strip.Color( lval, lval, lval));
    }
    strip.show();
    delay(5);
  }
  noTone(buzzerPin);
  lasers_off();
  colorOff();
  level_up();
}

void loose(){
  in_game = false;
  lasers_off();
  colorOff();
  for(int a=50; a<800; a=a+10){
    tone(buzzerPin, 800-a, 5);
    for(int i=0; i<10; i++){
      int lval = (800-a)/4-40;
      if(lval<0){lval=0;}
      strip.setPixelColor(i, strip.Color( lval, lval, lval));
    }
    strip.show();
    delay(5);
  }
  noTone(buzzerPin);
  lasers_off();
  colorOff();
  update_display();
  delay(10000);
}

void set_board(){
  int old_target_num = target_num;
  while(old_target_num==target_num){
    target_num = random(0, 9);
  }
  target_color = random(0, 3);
  if(target_color==0){ strip.setPixelColor(target_num, strip.Color(0, 90, 0)); }
  if(target_color==1){ strip.setPixelColor(target_num, strip.Color(90, 0, 0)); }
  if(target_color==2){ strip.setPixelColor(target_num, strip.Color(0, 0, 90)); }
  strip.show();
  lasers(target_color);
}

void update_display(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(1,16);
  display.print("L: ");
  display.setCursor(35,16);
  display.print(level);

  display.setTextSize(3);
  display.setCursor(1,42);
  display.print("S: ");
  display.setCursor(35,42);
  display.print(score);
  
  display.setTextSize(2);
  display.setCursor(1,0);
  if(in_game){
    display.print("TIME: ");
    display.print(countdown);
  }else{
    display.print("GAME OVER");
  }
  display.display();
}

void sfx_low(){
  tone(buzzerPin, NOTE_C1, 100);
  delay(100);
  noTone(buzzerPin);
}

void sfx_mid(){
  tone(buzzerPin, NOTE_C3, 100);
  delay(100);
  noTone(buzzerPin);
}

void sfx_high(){
  tone(buzzerPin, NOTE_G3, 100);
  delay(100);
  noTone(buzzerPin);
}

void lasers_off(){
  digitalWrite(laser_red, LOW);
  digitalWrite(laser_green, LOW);
  digitalWrite(laser_blue, LOW);
}

void lasers(int val){
  if(val==0){ digitalWrite(laser_red, HIGH);}
  if(val==1){ digitalWrite(laser_green, HIGH);}
  if(val==2){ digitalWrite(laser_blue, HIGH);}
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
void colorOff() {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}

void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
      delay(wait);
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void sfx1(){
  //BIG THANKS -> redvalkyrie -> https://forum.arduino.cc/index.php?topic=118757.0
  tone(buzzerPin,2200); // then buzz by going high
    tone(buzzerPin,1000);
    tone(buzzerPin,500);
    tone(buzzerPin,200);
    tone(buzzerPin,500);
    delayMicroseconds(10000);    // waiting
    noTone(buzzerPin);  // going low
    delayMicroseconds(10000);    // and waiting more
    tone(buzzerPin,2200);
    tone(buzzerPin,1000);
    delayMicroseconds(10000);    // waiting
    noTone(buzzerPin);  // going low
    delayMicroseconds(10000);    // and waiting more
    tone(buzzerPin,100);
    delayMicroseconds(10000);    // waiting
    noTone(buzzerPin);  // going low
    delayMicroseconds(10000);    // and waiting more
    tone(buzzerPin,100);
    delayMicroseconds(10000);    // waiting
    noTone(buzzerPin);  // going low
    delayMicroseconds(10000);    // and waiting more
}
