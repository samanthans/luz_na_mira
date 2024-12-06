#include <Wire.h>
#include <SoftwareSerial.h>
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <LCD-I2C.h>

SoftwareSerial softSerial(/*rx =*/2, /*tx =*/3);
#define FPSerial softSerial

LCD_I2C lcd(0x27, 20, 4); 
DFRobotDFPlayerMini myDFPlayer;

uint16_t player1leds[5] = {4, 5, 6, 7, 8};
uint16_t player2leds[5] = {9, 10, 11, 12, A0};
const unsigned int btn2Values[5] = {900, 450, 300, 225, 170};
const unsigned int btn1Values[5] = {890, 420, 265, 175, 110};
uint16_t player1btn = A3;
uint16_t player2btn = A2;
long randNumber;
int step_counter = 0;
int p1_score = 0;
int p2_score = 0;
int action_speed;
const int initial_speed = 2000;
int action_speed_min = 500;
uint8_t leds_cnt = 5;
int pin_light = 0;
int buttonNum = 1;
bool p1a = false;
bool p2a = false;
unsigned long startTime;   // Stores the time when LED lights up
unsigned long reactionTimeP1; // Time it takes for player to press button
unsigned long reactionTimeP2;
int scalingFactor = 400;
int prev_pin_light;
bool isReverse = false;

uint8_t row = 1;
// Custom character arrays (0-7 for valid CGRAM addresses)

byte bar[8][8] = {
  { 0x1C, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1C },
  { 0x07, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x07 },
  { 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F },
  { 0x1E, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x18, 0x1C },
  { 0x0F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07 },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0F },
  { 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

byte barU[8][8] = {
  { 0x1C, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1C },
  { 0x07, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x07 },
  { 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F },
  { 0x1C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x1E },
  { 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0F },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F },
  { 0x1E, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};


int buttonPressed(uint8_t player);

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A7));
  // Tela
  Wire.begin();
  lcd.begin(&Wire);
  lcd.display();
  lcd.backlight();

  for(int i = 0; i < 5; i++)
  {
    pinMode(player1leds[i], OUTPUT);
    pinMode(player2leds[i], OUTPUT);
  }

  // MP3

  FPSerial.begin(9600);
  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  myDFPlayer.volume(20);  //Set volume value. From 0 to 30
  startGameSequence();
}

void loop() {

  if(p1_score < 100 && p2_score < 100) {
    
    step_counter++;
    bool step_action = false;
    if (step_counter > action_speed) {
      step_counter = 0;
      
      step_action = true;  

      action_speed = action_speed - round(action_speed/20);
      if (action_speed < action_speed_min) {
        action_speed = action_speed_min;
      }
    }
    
  
    if (step_action && !(p1a && p2a)) { // 
      for (int i = 0; i < 5; i++){
        digitalWrite(player1leds[pin_light], LOW);
        digitalWrite(player2leds[pin_light], LOW);
      }
      do {
        pin_light = random(0,5);
      } while (prev_pin_light == pin_light);
      prev_pin_light = pin_light;

      digitalWrite(player1leds[pin_light], HIGH);
      digitalWrite(player2leds[pin_light], HIGH);
      p1a = true;
      p2a = true;
      startTime = millis();
      Serial.print("Step Counter: ");
      Serial.println(step_counter);
      Serial.print("Action Speed: ");
      Serial.println(action_speed);
      Serial.print("Action Speed Min: ");
      Serial.println(action_speed_min);

    }

    int player1Button = buttonPressed(1);
    int player2Button = buttonPressed(2);

    if (player1Button == pin_light && p1a) {
      digitalWrite(player1leds[pin_light], LOW);
      reactionTimeP1 = millis() - startTime;
      p1_score += calculatePoints(reactionTimeP1);
      p1a = false;
      lcd.setCursor(0, 1);
      lcd.print(p1_score);
      myDFPlayer.play(1);
      printScore();
    }

    if (player2Button == pin_light && p2a) {
      digitalWrite(player2leds[pin_light], LOW);
      reactionTimeP2 = millis() - startTime;
      p2_score += calculatePoints(reactionTimeP2);
      p2a = false;
      lcd.setCursor(0, 2);
      lcd.print(p2_score);
      myDFPlayer.play(1);
      printScore();
    }
    delay(1);
    
  } 
  
  else {
    if (p1_score > p2_score) {
      // Mostrar que o jogador 1 ganhou
      digitalWrite(player2leds[pin_light], LOW);
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Fim de jogo!");
      myDFPlayer.play(4);

      for(int j = 0; j < 7; j++){
        for (int i = 0; i < 5; i++){
          digitalWrite(player1leds[i], LOW);
        }
        delay(300);
        for (int i = 0; i < 5; i++){
          digitalWrite(player1leds[i], HIGH);
        }
        delay(700);
      }
      startGameSequence();
    } 
    else {
      // Mostrar que o jogador 2 ganhou
      digitalWrite(player1leds[pin_light], LOW);
      lcd.clear();

      lcd.setCursor(4,1);
      lcd.print("Fim de jogo!");
      myDFPlayer.play(4);

      for(int j = 0; j < 7; j++){
        for (int i = 0; i < 5; i++){
          digitalWrite(player2leds[i], LOW);
        }
        delay(300);
        for (int i = 0; i < 5; i++){
          digitalWrite(player2leds[i], HIGH);
        }
        delay(700);
      }

      startGameSequence();
    }   
  }
}

// Custom Characters
void custom0(int col)
{ 
  lcd.setCursor(col, row); 
  lcd.write(0); // bar0
  lcd.write(7); // bar7
  lcd.write(1); // bar1
  lcd.setCursor(col, row+1); 
  lcd.write(0); // bar0
  lcd.write(5); // bar5
  lcd.write(1); // bar1
}

void custom1(int col)
{
  lcd.setCursor(col, row);
  lcd.write(32);
  lcd.write(1);
  lcd.write(32);
  lcd.setCursor(col, row+1);
  lcd.write(32);
  lcd.write(1);
  lcd.write(32);
}
void custom1u(int col)
{
  lcd.setCursor(col, row);
  lcd.write(32);
  lcd.write(0);
  lcd.write(32);
  lcd.setCursor(col, row+1);
  lcd.write(32);
  lcd.write(0);
  lcd.write(32);
}

void custom2(int col)
{
  lcd.setCursor(col, row);
  lcd.write(4); // bar4
  lcd.write(2); // bar2
  lcd.write(1); // bar1
  lcd.setCursor(col, row+1);
  lcd.write(0); // bar0
  lcd.write(5); // bar5
  lcd.write(5); // bar5
}

void custom2u(int col)
{
  lcd.setCursor(col, row);
  lcd.write(7); // bar4
  lcd.write(7); // bar2
  lcd.write(1); // bar1
  lcd.setCursor(col, row+1);
  lcd.write(0); // bar0
  lcd.write(2); // bar5
  lcd.write(3); // bar5
}

void custom3(int col)
{
  lcd.setCursor(col, row);
  lcd.write(4); // bar4
  lcd.write(2); // bar2
  lcd.write(1); // bar1
  lcd.setCursor(col, row+1);
  lcd.write(6); // bar6
  lcd.write(5); // bar5
  lcd.write(1); // bar1
}

void custom3u(int col)
{
  lcd.setCursor(col, row);
  lcd.write(0); // bar4
  lcd.write(7); // bar2
  lcd.write(6); // bar1
  lcd.setCursor(col, row+1);
  lcd.write(0); // bar6
  lcd.write(2); // bar5
  lcd.write(3); // bar1
}


void custom4(int col)
{
  lcd.setCursor(col, row);
  lcd.write(0); // bar0
  lcd.write(5); // bar5
  lcd.write(1); // bar1
  lcd.setCursor(col, row+1);
  lcd.write(32);
  lcd.write(32);
  lcd.write(1);
}

void custom4u(int col)
{
  lcd.setCursor(col, row);
  lcd.write(0); // bar0
  lcd.write(32); // bar5
  lcd.write(32); // bar1
  lcd.setCursor(col, row+1);
  lcd.write(0);
  lcd.write(7);
  lcd.write(1);
}

void custom5(int col)
{
  lcd.setCursor(col, row);
  lcd.write(0); // bar0
  lcd.write(2); // bar2
  lcd.write(3); // bar3
  lcd.setCursor(col, row+1);
  lcd.write(6); // bar6
  lcd.write(5); // bar5
  lcd.write(1); // bar1
}

void custom5u(int col)
{
  lcd.setCursor(col, row);
  lcd.write(0); // bar0
  lcd.write(7); // bar2
  lcd.write(6); // bar3
  lcd.setCursor(col, row+1);
  lcd.write(4); // bar6
  lcd.write(2); // bar5
  lcd.write(1); // bar1
}

void custom6(int col)
{
  lcd.setCursor(col, row);
  lcd.write(0); // bar0
  lcd.write(2); // bar2
  lcd.write(3); // bar3
  lcd.setCursor(col, row+1);
  lcd.write(0); // bar0
  lcd.write(5); // bar5
  lcd.write(1); // bar1
}

void custom6u(int col)
{
  lcd.setCursor(col, row);
  lcd.write(0); // bar0
  lcd.write(7); // bar2
  lcd.write(1); // bar3
  lcd.setCursor(col, row+1);
  lcd.write(4); // bar0
  lcd.write(2); // bar5
  lcd.write(1); // bar1
}


void custom7(int col)
{
  lcd.setCursor(col, row);
  lcd.write(0); // bar0
  lcd.write(7); // bar7
  lcd.write(1); // bar1
  lcd.setCursor(col, row+1);
  lcd.write(32);
  lcd.write(32);
  lcd.write(1);
}

void custom7u(int col)
{
  lcd.setCursor(col, row);
  lcd.write(0); // bar0
  lcd.write(32); // bar7
  lcd.write(32); // bar1
  lcd.setCursor(col, row+1);
  lcd.write(0);
  lcd.write(5);
  lcd.write(1);
}

void custom8(int col)
{
  lcd.setCursor(col, row); 
  lcd.write(0); // bar0
  lcd.write(2); // bar2
  lcd.write(1); // bar1
  lcd.setCursor(col, row+1); 
  lcd.write(0); // bar0
  lcd.write(5); // bar5
  lcd.write(1); // bar1
}

void custom8u(int col)
{
  lcd.setCursor(col, row); 
  lcd.write(0); // bar0
  lcd.write(7); // bar2
  lcd.write(1); // bar1
  lcd.setCursor(col, row+1); 
  lcd.write(0); // bar0
  lcd.write(2); // bar5
  lcd.write(1); // bar1
}


void custom9(int col)
{
  lcd.setCursor(col, row); 
  lcd.write(0); // bar0
  lcd.write(2); // bar2
  lcd.write(1); // bar1
  lcd.setCursor(col, row+1); 
  lcd.write(6); // bar6
  lcd.write(5); // bar5
  lcd.write(1); // bar1
}

void custom9u(int col)
{
  lcd.setCursor(col, row); 
  lcd.write(0); // bar0
  lcd.write(7); // bar2
  lcd.write(6); // bar1
  lcd.setCursor(col, row+1); 
  lcd.write(0); // bar6
  lcd.write(2); // bar5
  lcd.write(1); // bar1
}

void printNumber(int value, int col) {
  normal();
  switch(value) {
    case 0: custom0(col); break;
    case 1: custom1(col); break;
    case 2: custom2(col); break;
    case 3: custom3(col); break;
    case 4: custom4(col); break;
    case 5: custom5(col); break;
    case 6: custom6(col); break;
    case 7: custom7(col); break;
    case 8: custom8(col); break;
    case 9: custom9(col); break;
    default: break;
  }
}

void printNumberU(int value, int col) {
  reverse();
  switch(value) {
    case 0: custom0(col); break;
    case 1: custom1u(col); break;
    case 2: custom2u(col); break;
    case 3: custom3u(col); break;
    case 4: custom4u(col); break;
    case 5: custom5u(col); break;
    case 6: custom6u(col); break;
    case 7: custom7u(col); break;
    case 8: custom8u(col); break;
    case 9: custom9u(col); break;
    default: break;
  }
}


int buttonPressed(uint8_t player){
  int value;
  if(player == 1) {

   value = analogRead(player1btn);
            if (value > btn1Values[0] && value < 1023)
            return 0;
            else if (value > btn1Values[1] && value < btn1Values[0])
            return 1;
            else if (value > btn1Values[2] && value < btn1Values[1])
            return 2;
            else if (value > btn1Values[3] && value < btn1Values[2])
            return 3;
            else if (value > btn1Values[4] && value < btn1Values[3])
            return 4;
            else
            return -1;
  }

  else if (player == 2) {
    value = analogRead(player2btn);
            if (value > btn2Values[0] && value < 1023)
            return 0;
            else if (value > btn2Values[1] && value < btn2Values[0])
            return 1;
            else if (value > btn2Values[2] && value < btn2Values[1])
            return 2;
            else if (value > btn2Values[3] && value < btn2Values[2])
            return 3;
            else if (value > btn2Values[4] && value < btn2Values[3])
            return 4;
            else
            return -1;
  }
}

int calculatePoints(unsigned long reactionTime) {
  int offset = 1; // Minimum guaranteed points
  int basePoints = 2000; // Max reaction time to consider
  if (reactionTime > basePoints) return 1;
  return max(0, (basePoints - reactionTime) / scalingFactor + offset);
}

void pause(){
  while (buttonPressed(1) < 0 || buttonPressed(2) < 0 ){}
  return;
}

void startGameSequence(){
  lcd.clear();
  for(int i = 0, n = 5; i < n; i++)
  {
    digitalWrite(player1leds[i], HIGH);
    digitalWrite(player2leds[i], HIGH);
  }
  delay(500);
  lcd.setCursor(6, 0);
  lcd.print("INICIAR");
  lcd.setCursor(3, 2);
  lcd.print("APERTAR BOTAO");
  lcd.setCursor(3, 3);
  lcd.print("EM AMBOS LADOS");
  pause();

  lcd.clear();

  myDFPlayer.play(3);
  for(int i = 0, n = 5; i < n; i++)
  {
    digitalWrite(player1leds[i], LOW);
    digitalWrite(player2leds[i], LOW);
    if ((n-i) % 2 != 0){
      printNumber(n-i, 8);
    } else {
      printNumberU(n-i, 8);
    }
    
    delay(900);
  }
  printNumber(0, 8);
  delay(900);
  lcd.clear();
  p1_score = 0;
  p2_score = 0;
  p1a = false;
  p2a = false;
  step_counter = 0;
  action_speed = initial_speed;
  lcd.print("PONTOS");
  return;
}

void printScore(){
  Serial.println(p1_score);
  Serial.println(p1_score);
  int dezena, unidade;
  uint8_t op;
  if (p1_score > p2_score){
    op = 1;
  }
  else if (p2_score > p1_score){
    op = 2;
  }
  else {
    op = 1;
  }
  switch(op){
    case 1:
      dezena = p1_score / 10;
      unidade = p1_score % 10;
      if (dezena > 0)
      printNumberU(dezena, 16);
      printNumberU(unidade, 12);
      break;

    case 2:
      dezena = p2_score / 10;
      unidade = p2_score % 10;
      if (dezena > 0)
      printNumber(dezena, 12);
      printNumber(unidade, 16);
      break;
  }
  return;
}

void reverse(){
  if (!isReverse){
    for (int i = 0; i < 8; i++)
        lcd.createChar(i, barU[i]); 
  }
  isReverse = true;
}

void normal(){
  if (isReverse){
    for (int i = 0; i < 8; i++)
      lcd.createChar(i, bar[i]);
  }
  isReverse = false;
}
