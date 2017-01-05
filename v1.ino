/*********************************************************************
  GPS tracker 1.0.0
  
  LCD connect:
    PIN_DC    D8 
    PIN_RESET D9 
    PIN_SCE   D10 
    PIN_SDIN  D11 
    PIN_SCLK  D13
    PIN_LED   D6
*********************************************************************/

#include "LowPower.h"
#include <SPI.h>
#include "PCD8544_SPI.h"
#include <TinyGPS.h>
#include "SoftwareSerial.h"
#include "SdFat.h"
#include <Battery.h>

#define PIN_DC              0x01  // D8
#define PIN_RESET           0x02  // D9
#define PIN_CE              0x04  // D10
#define PIN_LCD_LED         A1     // A1
#define PIN_LCD_POWER       A2     // A2
#define PIN_GPS_POWER       A5     // A5
#define PIN_LCD_LED_BTN     2      // D2

#define SD_CHIP_SELECT_PIN  7 //D7

#define BASE_FILE_NAME "Hike"

PCD8544_SPI lcd;
TinyGPS gps;
SoftwareSerial ss(3, 3);
SdFat sd;
SdFile flDataFile;
Battery battery(2500, 4200, A7, A6);

uint8_t satelite [] = {0x7E, 0x79, 0x14, 0x12, 0x09};
uint8_t sd_exist [] = {0x7C, 0x42, 0x41, 0x41, 0x7F};
uint8_t nosd []     = {0x7C, 0x56, 0x49, 0x55, 0x7F};


bool serial = false, sdopen = false;
unsigned long previousMillis = 0;
float flat = 0, flon = 0;
unsigned long age;
int year;
byte month, day, hour, minute, second, hundredths;
char currentFileName[20];
bool lcdLedState = false;
char formattedTime[6];

bool initSD() 
{
  sdopen = sd.begin(SD_CHIP_SELECT_PIN, SPI_HALF_SPEED);
  
  if(sdopen) {
    if(!sd.exists(currentFileName)) {
      flDataFile.open(currentFileName, O_CREAT | O_RDWR | O_AT_END);
      flDataFile.println(F("satellites, hdop, latitude, longitude, age, date, alt, course, speed"));
      flDataFile.close();
    }
  }

  return sdopen;
}

void writeToSD() 
{
  float flatOld = flat, flonOld = flon;
  gps.f_get_position(&flat, &flon, &age);
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
  char sz[32];
  sprintf(sz, "%02d-%02d-%02d %02d:%02d:%02d",day, month, year, hour, minute, second);
  sprintf(formattedTime, "%02d:%02d",hour, minute);
  
  sprintf(currentFileName, "%s-%02d-%02d-%02d.gps", BASE_FILE_NAME, year, month, day);

  if(initSD()) {
    //if(gps.distance_between(flatOld, flonOld, flat, flon) >= 1) { //15 meters
      flDataFile.open(currentFileName, O_CREAT | O_WRITE | O_AT_END);
      
      flDataFile.print(gps.satellites()); flDataFile.print(",");
      flDataFile.print(gps.hdop()); flDataFile.print(",");
      flDataFile.print(flat, 6); flDataFile.print(",");
      flDataFile.print(flon, 6); flDataFile.print(",");
      flDataFile.print(age); flDataFile.print(",");
      flDataFile.print(sz); flDataFile.print(",");
      flDataFile.print(gps.f_altitude()); flDataFile.print(",");
      flDataFile.print(gps.f_course()); flDataFile.print(",");
      flDataFile.print(gps.f_speed_kmph(), 2); flDataFile.print(",");
      flDataFile.println();
      flDataFile.close();
      
      lcd.gotoXY(75,0);
      lcd.print(" ");
    //}
  }
}

void lcdLed()
{
  lcdLedState = true;
}

void setup(void)
{
  ss.begin(9600);
  battery.begin(5000, 1.0485);

  pinMode(PIN_LCD_LED, OUTPUT);
  pinMode(PIN_GPS_POWER, OUTPUT);
  pinMode(PIN_LCD_LED_BTN, INPUT);
  
  digitalWrite(PIN_GPS_POWER, HIGH);

  attachInterrupt(0, lcdLed, HIGH);

  lcd.begin();
  lcd.gotoXY(1,2);
  lcd.print("  Loading...   ");
}

void loop(void) 
{
  analogWrite(PIN_LCD_POWER, lcdLedState ? 175 : 0);

  if(!serial) {
    ss.begin(9600);
    digitalWrite(PIN_GPS_POWER, HIGH);
    serial = true;
    delay(100);
  }
  
  delay(100);
  
  while (ss.available()) {
    if(gps.encode(ss.read())) {

      writeToSD();
      
      lcd.clear();

      lcd.writeBitmap(satelite, 0, 0, 5, 1);
      
      lcd.gotoXY(6,0); lcd.print(gps.satellites());

      lcd.gotoXY(18,0); lcd.print(formattedTime);
      
      lcd.gotoXY(50,0); lcd.print(battery.level());lcd.print("%");

      if(!sdopen) {
        lcd.writeBitmap(nosd, 75, 0, 5, 1);
      } else {
        lcd.writeBitmap(sd_exist, 75, 0, 5, 1);
      }
  
      lcd.gotoXY(1,1); lcd.print("Alt: "); lcd.print(gps.f_altitude());
  
      lcd.gotoXY(1,2); lcd.print("Spd: "); lcd.print(gps.f_speed_kmph());   
  
      lcd.gotoXY(1,3); lcd.print("Dir: "); lcd.print(gps.f_course());
      
      lcd.gotoXY(1,4); lcd.print("Lat: "); lcd.print(flat, 5);
      
      lcd.gotoXY(1,5); lcd.print("Lon: "); lcd.print(flon, 5);    
     
      ss.end();
      serial = false;
      digitalWrite(PIN_GPS_POWER, LOW);
      
      LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);

      //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
      //LowPower.idle(SLEEP_4S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
      lcdLedState = false;
    }   
  }
  
}



