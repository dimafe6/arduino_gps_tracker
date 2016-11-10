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
#include <TinyGPS.h>
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
TinyGPS gps;
SoftwareSerial ss(3, 3);
SdFatSoftSpi<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> sd;

File flDataFile;                                      // Файл для записи GPS-строк.

uint8_t satelite [] = {
0x7E, 0x79, 0x14, 0x12, 0x09,
};

float flat, flon;
unsigned long age;

void writeToSD() {
  int year;
  byte month, day, hour, minute, second, hundredths;
  
  lcd.writeBitmap(satelite,0, 0, 5, 1);
  
  flDataFile = sd.open("track.gps", O_CREAT | O_WRITE | O_AT_END);
  
  gps.f_get_position(&flat, &flon, &age);
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
  char sz[32];
  sprintf(sz, "%02d-%02d-%02d %02d:%02d:%02d",day, month, year, hour, minute, second);
   
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
  lcd.clear();
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void setup(void)
{
  Serial.begin(115200);
  ss.begin(9600);
  pinMode(PIN_LCD_LED, OUTPUT);
  analogWrite(PIN_LCD_LED, 240);
  
  lcd.begin();

  sd.begin(SD_CHIP_SELECT_PIN);
  flDataFile = sd.open("track.gps", O_CREAT | O_WRITE | O_AT_END);
  flDataFile.print(F("satellites, hdop, latitude, longitude, age, date, alt, course, speed"));
  flDataFile.println();
  flDataFile.close();
}

void loop(void) 
{
    
  smartdelay(1000);

  writeToSD();
  
  delay(500);
  /*gps.satellites();
  print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
  gps.f_get_position(&flat, &flon, &age);
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  print_date(gps);
  print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2);
  print_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
  print_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
  print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, LONDON_LAT, LONDON_LON) / 1000, 0xFFFFFFFF, 9);
  print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON)), 6);
*/

 /* while(ss.available())
  {
    gps.encode(ss.read());
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
  }*/
}



