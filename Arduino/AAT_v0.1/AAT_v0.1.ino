#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <GyverNTC.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "GyverEncoder.h"
#include <microDS3231.h>
MicroDS3231 rtc;



boolean backlight = true;
int contrast=50;

int menuitem = 1;
int page = 0;

volatile boolean up = false;
volatile boolean down = false;
volatile boolean middle = false;

int downButtonState = 0;
int upButtonState = 0;  
int selectButtonState = 0;          
int lastDownButtonState = 0;
int lastSelectButtonState = 0;
int lastUpButtonState = 0;
int timeHon = 12;
int timeHoff = 12;

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 8, 12);


GyverNTC therm(0, 10000, 3950);

#define CLK 2
#define DT 3
#define SW 4

Encoder enc1(CLK, DT, SW);

#define analogPin A0
#define analogPin A1
#define LED_bl 5
#define RELAY1_PIN 9
#define RELAY2_PIN 10
#define BUZZER_PIN 6

unsigned long last_time;
unsigned long dTime=0;

int targ = 50;
int tmptarg = 50;
int Ttmptarg =  50;
int timetarg = 30;
int retT = 0;
const int maxTemp = 80;


float filT = 0;
float hysterRuf = 2;
float hysterTun = 1;
int TempDec;
int TempDev;
int bhold = 0;
int timecount = 0;
int brightness = 30;
int brightnessNew = 30;  
int fadeAmount = 5; 

void setup() {

  enc1.setType(TYPE1);

  pinMode(2, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600); /* Initialize serial communication at 115200 */
  /* Initialize the Display*/
  
  display.begin();

  /* Change the contrast using the following API*/
 // display.setContrast(contrastValue);

  /* Clear the buffer */
if (millis() - last_time > 10){
  display.clearDisplay();
  display.display();
  }  
  
   
  display.setContrast(contrast); //Set contrast to 50
  display.clearDisplay(); 
  display.display();   
}

void loop() {

int tempdiff = abs(timeHon - timeHoff);

 enc1.tick();
  
  drawMenu();

analogWrite(LED_bl, brightness); 
   if (enc1.isTurn() || enc1.isClick()) { 
    tone(BUZZER_PIN, 2200, 10);
    dTime=millis();
    brightness = 255;
    brightnessNew = 255; 
   }
if (backlight == true){
if (brightnessNew && (millis()-dTime>30)){
  brightnessNew--;
  if (brightnessNew <220) brightness = brightnessNew;
  dTime=millis();
  if (brightness <= 30) brightness = 30;
}
}
else 
{
  brightness = 0;
}
analogWrite(LED_bl, brightness);

//int sensorValue = map(analogRead(A2),0,1023,0,23);
//A1 /=10; 

  filT += (therm.getTemp() - filT) * 0.1;
  // выводим температуру
  Serial.print(filT);
  Serial.print(",");
  Serial.print(Ttmptarg);
  Serial.print(",");
  Serial.print(tmptarg);
  Serial.print(",");
  Serial.print(targ);
  Serial.print(",");
  Serial.print(timetarg);
  Serial.print(",");
  Serial.print(rtc.getHours());
  Serial.print(",");
  Serial.print(timecount);
  Serial.print(",");
  Serial.println(bhold);


if ((abs(timeHon-timeHoff))>=1)
{
  if(timeHon>timeHoff)
     {
       if (timeHon >= rtc.getHours() && rtc.getHours() >= timeHoff && bhold == 0 )
       {
           targ=timetarg;
           timecount =2;
           retT=2;
           timecount =0;
       }
       else 
       {
         if(retT >= 1)
         {
          targ = Ttmptarg;
          retT=0;
          timecount =0;
         }
       }
     }
   else if (timeHon<timeHoff)
     {
       if (timeHon >= rtc.getHours() || rtc.getHours() >= timeHoff && bhold == 0)
       {
            targ=timetarg;
            timecount =1;
            retT=1;
       }
       else 
       {
         if(retT >= 1)
         {
          targ = Ttmptarg;
          retT=0;
          timecount =0;
         }
       }
     }       
}
else timecount =0;

if (timecount == 0 && timeHon==timeHoff)
{
  Ttmptarg=targ;
}

  if (enc1.isHolded() && page == 0){
  bhold+=1;
  tmptarg = targ;
  targ = maxTemp; 
  tone(BUZZER_PIN, 2640, 300);
    } 
  if ((bhold>=1) && (filT >= targ))
  {
    targ = tmptarg; 
    bhold=0;
  }
  if (page==0)
    {
     if (enc1.isRight()) targ +=1;
     if (enc1.isLeft()) targ-=1;
     if (enc1.isFastR()) targ +=5; 
     if (enc1.isFastL()) targ-=5;
    }
  
  TempDec = (((int)filT))%100;
  TempDev = ((int)(filT*100))%100;


static bool relay1State = false;
 if (filT < (targ - hysterRuf )) relay1State = true;
 else if (filT > (targ + hysterRuf )) relay1State = false;  
  digitalWrite(RELAY1_PIN, relay1State);

static bool relay2State = false;
 if (filT < (targ - hysterTun )) relay2State = true;
 else if (filT > (targ + hysterTun )) relay2State = false;  
  digitalWrite(RELAY2_PIN, relay2State);



if (enc1.isDouble()) {
  page +=1;
  if (page >=2) page=0;
}

if (page!=0){
  if (enc1.isRight()){
    downButtonState = 1;
  }
  else {
    downButtonState = 0;
  }
  if (enc1.isSingle()){
    selectButtonState = 1;
  }
  else{
    selectButtonState = 0;
  }
  if (enc1.isLeft()) {
    upButtonState = 1;
  }
  else{
    upButtonState = 0;
  }
}
  if (enc1.isRight()) Serial.println("Right");         // если был поворот
  if (enc1.isLeft()) Serial.println("Left");
  if (enc1.isPress()) Serial.println("Press");   
 
  checkIfDownButtonIsPressed();
  checkIfUpButtonIsPressed();
  checkIfSelectButtonIsPressed();

  if (up && page == 1 ) {
    up = false;
    menuitem--;
    if (menuitem==0)
    {
      menuitem=5;
    }      
  }



  if (down && page == 1) {
    down = false;
    menuitem++;
    if (menuitem==6) 
    {
      menuitem=1;
    }      
  }

    if (page == 1 && menuitem == 1) 
    {
    if (enc1.isRightH()) contrast+=1;
    if (enc1.isLeftH()) contrast-=1;
    setContrast();
    }
    
    if (page == 1 && menuitem == 3) 
    {
    if (enc1.isRightH()) timeHon+=1;
    if (enc1.isLeftH()) timeHon-=1;
    if (timeHon>=24) timeHon = 0;
    if (timeHon<=-1) timeHon = 23;
    
    }
    if (page == 1 && menuitem == 4) 
    {
    if (enc1.isRightH()) timeHoff+=1;
    if (enc1.isLeftH()) timeHoff-=1;
    if (timeHoff>=24) timeHoff = 0;
    if (timeHoff<=-1) timeHoff = 23;
    }

  if (middle) {
    middle = false;
    
    if (page == 1 && menuitem==2) 
    {
      if (backlight) 
      {
        backlight = false;
 //       turnBacklightOff();
        }
      else 
      {
        backlight = true; 
   //     turnBacklightOn();
       }
    }

    if(page == 1 && menuitem ==5)
    {
      resetDefaults();
    }
   }
   
  }

  void checkIfDownButtonIsPressed()
  {
    if (downButtonState != lastDownButtonState) 
  {
    if (downButtonState == 0) 
    {
      down=true;
    }
    
  }
   lastDownButtonState = downButtonState;
  }

void checkIfUpButtonIsPressed()
{
  if (upButtonState != lastUpButtonState) 
  {
    if (upButtonState == 0) {
      up=true;
    }
    
  }
   lastUpButtonState = upButtonState;
}

void checkIfSelectButtonIsPressed()
{
   if (selectButtonState != lastSelectButtonState) 
  {
    if (selectButtonState == 0) {
      middle=true;
    }
    
  }
   lastSelectButtonState = selectButtonState;
}


  void drawMenu()
  {

if (page==0){

static bool relay1State = false;
 if (filT < (targ - hysterRuf )) relay1State = true;
 else if (filT > (targ + hysterRuf )) relay1State = false;  
  digitalWrite(RELAY1_PIN, relay1State);

static bool relay2State = false;
 if (filT < (targ - hysterTun )) relay2State = true;
 else if (filT > (targ + hysterTun )) relay2State = false;  
  digitalWrite(RELAY2_PIN, relay2State);
 
display.setTextSize(1);
display.setTextColor(BLACK);
display.setCursor(16,1);
char time[8];
rtc.getTimeChar(time);
display.print(time);

/*display.print(rtc.getHours());
display.setCursor(26,1);
display.print(":");
display.setCursor(30,1);
display.print(rtc.getMinutes());
display.setCursor(40,1);
display.print(":");
display.setCursor(44,1);
display.println(rtc.getSeconds()); */

display.setTextSize(1);
display.setCursor(60,38);
if ((relay1State || relay2State) == 1){
/*display.drawBitmap(4, 10, bitmap_image, 15, 24, 1);
display.display();*/
display.setTextColor(WHITE, BLACK);
display.println("Heat");

}
else {
  display.setTextColor(BLACK);
  display.println(" ");
}


display.setTextSize(3);
display.setTextColor(BLACK);
display.setCursor(22,12);
display.println(TempDec);


display.setTextSize(1);
display.setTextColor(BLACK);
display.setCursor(60,26);
display.println(TempDev);


display.setTextSize(1);
display.setTextColor(BLACK);
display.setCursor(54,26);
display.println(".");


display.setTextSize(2);
display.setTextColor(BLACK);
display.setCursor(37,34);
display.println(targ);


display.setTextSize(1);
display.setTextColor(BLACK);
display.setCursor(1,39);
display.println("Target");


display.drawCircle(70, 15, 2, BLACK);
display.display();
display.setTextSize(2);
display.setCursor(72,16);
display.println("C");
display.display();



if (millis() - last_time > 100){  
  display.clearDisplay();
 // display.display();
}
  
if ((enc1.isHold()) && (maxTemp <= filT) && ((filT > targ)) && (bhold==0)){
  display.fillRect(0, 12, 32, 23, BLACK);
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(4,13);
  display.println("Temp");
  display.setCursor(7,22);
  display.println("Lim");
  display.setCursor(1,31);
  display.println("Reach");
  display.display(); 
  }

}

    
  if (page==1) 
  {    
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);   
    if (menuitem==1) 
    { 
      display.setTextColor(WHITE, BLACK);
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);
    }
    display.setCursor(0, 0);
    display.print(">Contrast:");
    display.setTextColor(BLACK, WHITE);
    display.print(" ");
    display.print(contrast);
    
   
    if (menuitem==2) 
    {
      display.setTextColor(WHITE, BLACK);
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);
    }  
    display.setCursor(0, 8);  
    display.print(">Light: ");
    
    if (backlight) 
    {
      display.print("ON");
    }
    else 
    {
      display.print("OFF");
    }

    if (menuitem==3)
    {
      display.setTextColor(WHITE, BLACK);
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);
    }  
    display.setCursor(0, 16);
    display.print(">Time ON:");
    display.setTextColor(BLACK, WHITE);
    display.print(" ");
    display.print(timeHon);
    display.print("h");
    
    if (menuitem==4)
    {
      display.setTextColor(WHITE, BLACK);
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);
    }  
    display.setCursor(0, 24);
    display.print(">Time OFF:");
    display.setTextColor(BLACK, WHITE);
    display.print(" ");
    display.print(timeHoff);
    display.print("h");
    
    display.fillTriangle(map(timeHon,0,23,0,78), 32, map(timeHon,0,23,0,78)+3, 36, map(timeHon,0,23,0,78)+6, 32, BLACK);
    display.drawTriangle(map(timeHoff,0,23,0,78), 32, map(timeHoff,0,23,0,78)+3, 36, map(timeHoff,0,23,0,78)+6, 32, BLACK);
    display.drawRect(3, 37, 78, 4, BLACK);
    display.fillRect(map(timeHon,0,23,0,78)+3, 37, map(timeHoff,0,23,0,78)-map(timeHon,0,23,0,78), 4, BLACK);
    
    if (menuitem==5) 
    { 
      display.setTextColor(WHITE, BLACK);
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);
    }  
    display.setCursor(0, 41);
    display.print(">Reset");
    display.display();
  }
      
  }

  void resetDefaults()
  {
    contrast = 50;
    setContrast();
    backlight = true;
    timeHon = 12;
    timeHoff = 12;
  }

  void setContrast()
  {
    display.setContrast(contrast);
    display.display();
  }
