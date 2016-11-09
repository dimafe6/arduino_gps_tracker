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

#include <SPI.h>
#include "PCD8544_SPI.h"
#include "TinyGPS++.h"
#include "SoftwareSerial.h"
#include "SdFat.h"
#define PIN_DC        0x01  // D8
#define PIN_RESET     0x02  // D9
#define PIN_CE        0x04  // D10
#define PIN_LCD_LED   6     // D6

#define SOFT_MISO_PIN       2
#define SOFT_MOSI_PIN       4
#define SOFT_SCK_PIN        5
#define SD_CHIP_SELECT_PIN  7

PCD8544_SPI lcd;
TinyGPSPlus gps;
SoftwareSerial serial_connection(3, 3);
SdFatSoftSpi<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> sd;

uint8_t satelite [] = {
0x7E, 0x79, 0x14, 0x12, 0x09,
};

void setup(void)
{
  serial_connection.begin(9600);

  pinMode(PIN_LCD_LED, OUTPUT);
  analogWrite(PIN_LCD_LED, 240);
  
  lcd.begin();

  sd.begin(SD_CHIP_SELECT_PIN);
}

void loop(void) 
{
  while(serial_connection.available())
  {
    gps.encode(serial_connection.read());
  }
  
  if(gps.location.isUpdated())
  {
    lcd.clear();

    lcd.writeBitmap(satelite,0, 0, 5, 1);
    lcd.gotoXY(6,0);
    lcd.print(gps.satellites.value());

    lcd.writeBitmap(satelite,0, 0, 5, 1);
    lcd.gotoXY(1,1);
    lcd.print("Alt: ");
    lcd.print(gps.altitude.meters());

    lcd.writeBitmap(satelite,0, 0, 5, 1);
    lcd.gotoXY(1,2);
    lcd.print("Spd: ");
    lcd.print(gps.speed.kmph());   

lcd.writeBitmap(satelite,0, 0, 5, 1);
    lcd.gotoXY(1,3);
    lcd.print("Dir: ");
    lcd.print(gps.course.deg());
lcd.writeBitmap(satelite,0, 0, 5, 1);
    lcd.gotoXY(1,4);
    lcd.print("Lat: ");
    lcd.print(gps.location.lat(), 5);
lcd.writeBitmap(satelite,0, 0, 5, 1);
    lcd.gotoXY(1,5);
    lcd.print("Lon: ");
    lcd.print(gps.location.lng(), 5);
  }
}
