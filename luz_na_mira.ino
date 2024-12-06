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
const unsigned int btnValues[5] = {85, 39, 23, 15, 9};
uint16_t player1btn = A3;
uint16_t player2btn = A2;
long randNumber;
int step_counter = 0;
int p1_score = 0;
int p2_score = 0;
int action_speed = 1000;
int action_speed_min = 200;
uint8_t leds_cnt = 5;
int pin_light = 0;
int buttonNum = 1;

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


int buttonPressed(int pin);

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


  for(int i = 0, n = 5; i < n; i++)
  {
    digitalWrite(player1leds[i], HIGH);
    digitalWrite(player2leds[i], HIGH);
  }
  delay(500);
  lcd.setCursor(6, 1);
  lcd.print("INICIAR");
  while (buttonPressed(player1btn) < 0 || buttonPressed(player2btn) < 0 ){}

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
  lcd.clear();
  lcd.print("PONTOS");
}

void loop() {

  if(p1_score < 100 && p2_score < 100) {
    
    step_counter++;
    bool step_action = false;
    if (step_counter > action_speed) {
      step_counter = 0;
      step_action = true;  
      action_speed = action_speed - round(action_speed/50);
      if (action_speed < action_speed_min) {
        action_speed = action_speed_min;
      }
      Serial.println(action_speed);
    }
  
    if (step_action) {
      pin_light = random(0,5);
      digitalWrite(player1leds[pin_light], HIGH);
      digitalWrite(player2leds[pin_light], HIGH);
    }

    for (int i = 0; i < 5; i++) {
      if (buttonPressed(player1btn) == pin_light){
        if(digitalRead(player1leds[i]) == HIGH){
          digitalWrite(player1leds[i], LOW);
          p1_score++;
          lcd.setCursor(0, 1);
          lcd.print(p1_score);
          myDFPlayer.play(1);
        }
      }
    }
  
    for (int i = 0; i < 5; i++) {
      if (buttonPressed(player2btn) == pin_light){
        if(digitalRead(player2leds[i]) == HIGH){
          digitalWrite(player2leds[i], LOW);
          p2_score++;
          lcd.setCursor(0, 2);
          lcd.print(p2_score);
          myDFPlayer.play(1);
        }
      }
    }
    
  } 
  
  else {
    if (p1_score > p2_score) {
      // Mostrar que o jogador 1 ganhou
      lcd.clear();
      lcd.print("Jogador 1");
      lcd.setCursor(0,2);
      lcd.print("Vencedor!");
      myDFPlayer.play(4);
    } else {
      // Mostrar que o jogador 2 ganhou
      lcd.clear();
      lcd.print("Jogador 2");
      lcd.setCursor(0,2);
      lcd.print("Vencedor!");
      myDFPlayer.play(4);
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
  lcd.write(1);
  lcd.write(32);
  lcd.write(32);
  lcd.setCursor(col, row+1);
  lcd.write(1);
  lcd.write(32);
  lcd.write(32);
}
void custom1u(int col)
{
  lcd.setCursor(col, row);
  lcd.write(32);
  lcd.write(32);
  lcd.write(0);
  lcd.setCursor(col, row+1);
  lcd.write(32);
  lcd.write(32);
  lcd.write(0);
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
  for (int i = 0; i < 8; i++)
    lcd.createChar(i, bar[i]);
  
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
  for (int i = 0; i < 8; i++)
    lcd.createChar(i, barU[i]);
  
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


int buttonPressed(int pin){
  int reading = 0;
  for(int i = 0; i < 20; i++){
    reading += analogRead(pin);
  }
  reading /= 20;

  int value = map(reading, 0, 1023, 0, 100);

  if (value > btnValues[0] && value < 100)
  return 0;
  else if (value > btnValues[1] && value < btnValues[0])
  return 1;
  else if (value > btnValues[2] && value < btnValues[1])
  return 2;
  else if (value > btnValues[3] && value < btnValues[2])
  return 3;
  else if (value > btnValues[4] && value < btnValues[3])
  return 4;
  else
  return -1;

}

