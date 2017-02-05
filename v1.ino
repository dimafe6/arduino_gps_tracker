/*********************************************************************
  GPS tracker 1.0.1
*********************************************************************/

#include "LowPower.h"
#include <SPI.h>
#include <TinyGPS++.h>
#include "SoftwareSerial.h"
#include "SdFat.h"
#include <Battery.h>
#include <SerialCommand.h>


#define PIN_DC              0x01  // D8
#define PIN_RESET           0x02  // D9
#define PIN_CE              0x04  // D10
#define PIN_GPS_POWER       A5     // A5

#define SD_CHIP_SELECT_PIN  7 //D7

#define BASE_FILE_NAME "Hike"

#define SERIALCOMMAND_HARDWAREONLY true

enum CommandsType {NONE, NMEA, LOCATION, HIKE};

volatile CommandsType command = NONE;

TinyGPSPlus gps;
SoftwareSerial ssGPS(3, 3);
SdFat sd;
SdFile flDataFile;
SerialCommand SCmd;

bool serial = true, sdopen = false;
float flat = 0, flon = 0;
char *currentFileName = NULL;

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
  flat = gps.location.lat();
  flon = gps.location.lng();

  if(gps.date.isValid() && currentFileName == NULL) {
    sprintf(currentFileName, "%s-%02d-%02d-%02d.gps", BASE_FILE_NAME, gps.date.year(), gps.date.month(), gps.date.day());
  }
  
  if(initSD()) {
    unsigned long distance = TinyGPSPlus::distanceBetween(flatOld, flonOld, flat, flon);
    if(distance >= 50 && distance <= 2000) {
      
      char sz[32];
      sprintf(sz, "%02d-%02d-%02d %02d:%02d:%02d ", gps.date.month(), gps.date.day(), gps.date.year(),  gps.time.hour(),  gps.time.minute(),  gps.time.second());
      
      flDataFile.open(currentFileName, O_CREAT | O_WRITE | O_AT_END);
      
      flDataFile.print((!gps.satellites.isValid()) ?0: gps.satellites.value()); flDataFile.print(",");
      flDataFile.print(!gps.hdop.isValid() ? 0 : gps.hdop.value()); flDataFile.print(",");
      flDataFile.print(!gps.location.isValid() ? 0 : flat, 6); flDataFile.print(",");
      flDataFile.print(!gps.location.isValid() ? 0 : flon, 6); flDataFile.print(",");
      flDataFile.print(!gps.location.isValid() ? 0 : gps.location.age()); flDataFile.print(",");
      
      if(!gps.date.isValid() && !gps.time.isValid()) {
        flDataFile.print("0");
      } else {
        flDataFile.print(sz);
      }
      flDataFile.print(",");
      
      flDataFile.print(!gps.altitude.isValid() ? 0 : gps.altitude.meters()); flDataFile.print(",");
      flDataFile.print(!gps.course.isValid() ? 0 : gps.course.deg()); flDataFile.print(",");
      flDataFile.print(!gps.speed.kmph() ? 0 : gps.speed.kmph(), 2);
      flDataFile.println();
      flDataFile.close();
    }
  }
}

void onSerialRead() {
  SCmd.readSerial();
}

void NMEAHandler() {
    command = command == NMEA ? NONE : NMEA;
}

void LOCATIONHandler() {
  command = command == LOCATION ? NONE : LOCATION;
  Serial.println("Refresh location data...");
}

void HIKEHandler() {
  char *arg;  
  arg = SCmd.next();
  if (arg != NULL)
  {
    sprintf(currentFileName, "%s.gps", arg);
  } 
}

void setup(void)
{ 
  Serial.begin(9600);
  
  ssGPS.begin(9600);
  
  pinMode(PIN_GPS_POWER, OUTPUT);

  digitalWrite(PIN_GPS_POWER, HIGH);

  SCmd.addCommand("NMEA",NMEAHandler);
  SCmd.addCommand("LOCATION",LOCATIONHandler);
  SCmd.addCommand("HIKE",HIKEHandler);
  
  attachInterrupt(0, onSerialRead, CHANGE);
}

void loop(void) 
{  
  if(!serial) {
    serial = true;
    ssGPS.begin(9600);
    digitalWrite(PIN_GPS_POWER, HIGH);
    delay(500);
  }

  if(command == LOCATION) {
    Serial.print(flat, 6);Serial.print(",");Serial.print(flon, 6);
    command = NONE;
  }

  while (ssGPS.available()) { 
    char c = ssGPS.read();
    
    if(gps.encode(c) && gps.location.isValid()) {
      writeToSD(); 
           
      if(command != NMEA) {
        delay(50);
        ssGPS.end();
        serial = false;
        digitalWrite(PIN_GPS_POWER, LOW);
        
        LowPower.idle(SLEEP_4S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
        //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
        //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
        //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
        //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
      }
    }

    if(command == NMEA) { 
      Serial.write(c);
    }
  }
}



