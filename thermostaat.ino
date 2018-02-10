#include <OneWire.h>   // reminder: OneWire is included in the DallasTemperature library git. Adding it separately gives errors because of duplicate definitions
#include <DallasTemperature.h>  // https://github.com/milesburton/Arduino-Temperature-Control-Library


#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};



// v3

// 7-2-2018 v3.08: first version with time clock
// 9-12-2018 v.09: added heating map


// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

const int plusPin = 10;  // increase temp button
const int minPin = 11;    // decrease temp button   // p9 -> 11
const int relaisPin = 13; // heat signal on/off relais   // 3.07 pin 8 - > 13

bool heatOn = LOW;

int minPinState = LOW;
int plusPinState = LOW;

// measured temps
float currTemp = 0;
float prevTemp = 0; 

// desired temps
float tempMap[24];


float currSetTemp;  // 3.07 hardcoded to 18 -> 16 // moet naar 15
float prevSetTemp;  // 3.07 hardcoded to 18 -> 16 // moet naar 15

// in case of a temperature mismatch, this is the aimed temperature to avoid on/off/on
float targetTemp;  // 3.07 hardcoded to 18 -> 16  // moet naar 15

int hourSetTemp;

int startHourDay = 7;
int endHourDay = 19;



int measureCounter = 20;
int displayCounter = 20;

int relaisState = !heatOn;
int prevRelaisState = !heatOn;

char* charBuf = "abc123";

char* sourceVer = "3.08r";

int stdCounterDelay = 200;


#define PIN_RESET 5  
#define PIN_SCE   6  
#define PIN_DC    7
#define PIN_SDIN  8
#define PIN_SCLK  9

#define LCD_C     LOW
#define LCD_D     HIGH

#define LCD_X     84
#define LCD_Y     48

static const byte ASCII[][5] =
{
 {0x00, 0x00, 0x00, 0x00, 0x00} // 20  
,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c Ã‚Â¥
,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j 
,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e Ã¢â€ ï¿½
,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f Ã¢â€ â€™
};


void initTimeClock(void)
{

	// time module setup
	if (!rtc.begin()) {
		Serial.println("Couldn't find RTC");
		while (1);
	}

	//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void LcdCharacter(char character)
{
  LcdWrite(LCD_D, 0x00);
  for (int index = 0; index < 5; index++)
  {
    LcdWrite(LCD_D, ASCII[character - 0x20][index]);
  }
  LcdWrite(LCD_D, 0x00);
}

void LcdClear(void)
{
  for (int index = 0; index < LCD_X * LCD_Y / 8; index++)
  {
    LcdWrite(LCD_D, 0x00);
  }
}

void LcdInitialise(void)
{
  pinMode(PIN_SCE, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);
  pinMode(PIN_DC, OUTPUT);
  pinMode(PIN_SDIN, OUTPUT);
  pinMode(PIN_SCLK, OUTPUT);
  digitalWrite(PIN_RESET, LOW);
  digitalWrite(PIN_RESET, HIGH);
  LcdWrite(LCD_C, 0x21 );  // LCD Extended Commands.
  LcdWrite(LCD_C, 0xA7 );  // Set LCD Vop (Contrast).    0xB1
  LcdWrite(LCD_C, 0x04 );  // Set Temp coefficent. //0x04
  LcdWrite(LCD_C, 0x14 );  // LCD bias mode 1:48. //0x13
  LcdWrite(LCD_C, 0x20 );  // LCD Basic Commands
  LcdWrite(LCD_C, 0x0C );  // LCD in normal mode.

  LcdClear();

}

// gotoXY routine to position cursor 
// x - range: 0 to 84
// y - range: 0 to 5

void gotoXY(int x, int y)
{
  LcdWrite( 0, 0x80 | x);  // Column.
  LcdWrite( 0, 0x40 | y);  // Row.  

}



void LcdString(char *characters)
{
  while (*characters)
  {
    LcdCharacter(*characters++);
  }
}

void LcdWrite(byte dc, byte data)
{
  digitalWrite(PIN_DC, dc);
  digitalWrite(PIN_SCE, LOW);
  shiftOut(PIN_SDIN, PIN_SCLK, MSBFIRST, data);
  digitalWrite(PIN_SCE, HIGH);
}


void printNow(DateTime now)
{
	Serial.print(now.year(), DEC);
	Serial.print('/');
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);
	Serial.print(" (");
	Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
	Serial.print(") ");
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.print(now.second(), DEC);
	Serial.println();

}

void printTempReadings(void)
{
	Serial.println("targetTemp ** currTemp ** relaisState ** currTemp > targetTemp ** currTemp < (currSetTemp - 0.25) ** currSetTemp");
	Serial.print(targetTemp);
	Serial.print(" ** ");
	Serial.print(currTemp);
	Serial.print(" ** ");
	Serial.print(relaisState);


	Serial.print(" ** ");
	Serial.print(currTemp < (currSetTemp - 0.25));
	Serial.print(" ** ");
	Serial.print(currTemp > targetTemp);
	Serial.print(" ** ");
	Serial.print(currSetTemp);
	Serial.println(" ** ");

	Serial.println("*************************");

	Serial.println("A7");
	Serial.println("A4");
	Serial.print("currTemp: ");
	Serial.println(currTemp);
	Serial.print("prevTemp: ");
	Serial.println(prevTemp);

	Serial.println("A5");
	Serial.print("currTemp: ");
	Serial.println(currTemp);
	Serial.print("prevTemp: ");
	Serial.println(prevTemp);

	Serial.println("*************************");
}

void initTemp(DateTime now)
{
	tempMap[0] = 0;
	tempMap[1] = 0;
	tempMap[2] = 0;
	tempMap[3] = 0;
	tempMap[4] = 0;
	tempMap[5] = 0;
	tempMap[6] = 0;
	tempMap[7] = 15;
	tempMap[8] = 15;
	tempMap[9] = 15;
	tempMap[10] = 15;
	tempMap[11] = 0;
	tempMap[12] = 0;
	tempMap[13] = 0;
	tempMap[14] = 0;
	tempMap[15] = 0;
	tempMap[16] = 0;
	tempMap[17] = 0;
	tempMap[18] = 0;
	tempMap[19] = 0;
	tempMap[20] = 15;
	tempMap[21] = 15;
	tempMap[22] = 0;
	tempMap[23] = 0;

	hourSetTemp = now.hour();

	currSetTemp = tempMap[hourSetTemp];

	

	//Serial.println("initTempHour:");
	//Serial.println(now.hour(), DEC);

	



}

void setup(void)
{

 

  // start serial port
  Serial.begin(9600);

  delay(3000); // wait for console opening

  initTimeClock();
  

  DateTime now = rtc.now();

  // Start up the library for the temperature sensor
  sensors.begin();

  pinMode(relaisPin, OUTPUT);
  pinMode(plusPin, INPUT);
  pinMode(minPin, INPUT);

  LcdInitialise();

  digitalWrite(relaisPin, !heatOn);

  initTemp(now);

  targetTemp = currSetTemp;

  sensors.requestTemperatures();
  currTemp = sensors.getTempCByIndex(0);     
  
  
}
 

void refreshLCDScreen(float currTemp, float currSetTemp, int relaisState, char* sourceVer, DateTime now)
{

	LcdClear();
	gotoXY(0, 0);
	LcdString("Room:");
	LcdString(dtostrf(currTemp, 0, 2, charBuf));
	gotoXY(0, 1);
	LcdString("Set :");
	LcdString(dtostrf(currSetTemp, 0, 2, charBuf));
	gotoXY(0, 2);
	LcdString("Heat:");
	if (relaisState == !heatOn) {
		LcdString("Off");
	}
	else {
		LcdString("On");
	}
	gotoXY(0, 3);

/*
	LcdString("Mode:");
	if (now.hour()  < startHourDay or now.hour() >endHourDay) {
		LcdString("Night");
	}
	else
	{
		LcdString("Day");
	}
*/

	gotoXY(0, 3);
	LcdString("v3.09");  // 3.09: introduced tempMap

	prevTemp = currTemp;
	prevSetTemp = currSetTemp;
	prevRelaisState = relaisState;

	displayCounter = 0;
}
 
void loop(void)
{

    DateTime now = rtc.now();
    
	printNow(now);

	if (hourSetTemp < now.hour()) {
		
		initTemp(now);

	}

	Serial.print("hourSetTemp:");
	Serial.println(hourSetTemp, DEC);
  
  if(measureCounter == stdCounterDelay) {
	
	//don't read too often

	sensors.requestTemperatures();

	printTempReadings();
   
	prevTemp = currTemp;
    currTemp = sensors.getTempCByIndex(0);     
    measureCounter = 0;

  }

  
  if (currTemp < (currSetTemp - 0.25)  && relaisState == !heatOn) {
    // if the heating is not already switched on and the reading drops below requested temperature minus treshold raise target temp to make the system less nervous
	targetTemp = currSetTemp + 0.5;
  }





	if (currTemp < targetTemp && relaisState == !heatOn) { 
		prevRelaisState = relaisState;
		relaisState = heatOn;
		digitalWrite(relaisPin, relaisState);
	}
  
	if (currTemp > targetTemp && relaisState == heatOn)
	{
		relaisState = !heatOn;
		digitalWrite(relaisPin, relaisState);
	}


  // only refresh the screen if something changed

  
  if (currTemp != prevTemp || currSetTemp != prevSetTemp || relaisState != prevRelaisState) {


	  refreshLCDScreen(currTemp, currSetTemp, relaisState, sourceVer, now);


  }

  minPinState = digitalRead(minPin);
  plusPinState = digitalRead(plusPin);

  if (minPinState == HIGH) {
    prevSetTemp = currSetTemp;
    currSetTemp = currSetTemp - 0.5; 
    targetTemp = currSetTemp; 
    delay(200);  // avoid double buttonclick readings
    
  }

  if (plusPinState == HIGH) {
    prevSetTemp = currSetTemp;
    currSetTemp = currSetTemp + 0.5;    // MQ: not entirely correct, but fine for now
    targetTemp = currSetTemp + 0.5;
    
    delay(200); // avoid double buttonclick readings
  }
  
  delay(10);
  measureCounter++;
  displayCounter++;
}



