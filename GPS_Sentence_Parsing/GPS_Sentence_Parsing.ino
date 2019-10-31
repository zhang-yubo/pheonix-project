// Adafruit GPS modules using MTK3329/MTK3339 driver
//
// This code shows how to listen to the GPS module in an interrupt
// which allows the program to have more 'freedom' - just parse
// when a new NMEA sentence is available! Then access data when
// desired.

// Adafruit Ultimate GPS module using MTK33x9 chipset
//    ------> http://www.adafruit.com/products/746
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <math.h>

SoftwareSerial mySerial(8, 7);
SoftwareSerial Mo (9,10);
Adafruit_GPS GPS(&mySerial);

#define GPSECHO  false

void setup()
{

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  Mo.begin(115200);
  delay(5000);
  Serial.println("Game on");
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);

  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time

  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
}

uint32_t timer = millis();
void loop()
{
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if ((c) && (GPSECHO))
    Serial.write(c);

  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false

    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  if (timer > millis())  timer = millis();

  if (millis() - timer > 2000) {
    timer = millis();
    
    if (GPS.fix) {
      if (GPS.fix<10) Serial.print('0');
      Serial.print(GPS.fix);
      Serial.print(GPS.fixquality);
      
//      if (GPS.latitude<10000) Serial.print('0');
//      char latitude[14];
//      char a_latitude[13];
//      sprintf(latitude,"%.8f",GPS.latitude);
//      removePoint(latitude,a_latitude);
//      for(int i=0; i<sizeof(a_latitude); i++){
//        Serial.print(latitude[i]);  //5+8 digits
//      }
//      Serial.print((int)GPS.latitude);
//      Serial.print((int)((GPS.latitude-(int)GPS.latitude)*pow(10,2)));
      Serial.print(GPS.latitude,8);
      Serial.print(GPS.lat);

      if (GPS.longitude<10000) Serial.print('0');
//      char longitude[16];
//      char a_longitude[15];
//      sprintf(longitude,"%.10f",GPS.longitude);
//      removePoint(longitude,a_longitude);
//      for(int i=0; i<sizeof(a_longitude); i++){
//        Serial.print(a_longitude[i]); //5+10 digits total 
//      }
      Serial.print(GPS.longitude,10);
      Serial.print(GPS.lon);
      
      for (int i=0; i<5-count_digits(GPS.altitude);i++){
        Serial.print('0');
      }
      Serial.print(GPS.altitude,2);

//      char altitude[count_digits(GPS.altitude)+3];
//      char a_altitude[count_digits(GPS.altitude)+2];
//      sprintf(altitude,"%.2f",GPS.altitude);
//      removePoint(altitude,a_altitude);
//      for(int i=0; i<sizeof(a_longitude); i++){
//        Serial.print(a_altitude[i]); //5+2 digits total
//      }
      if (GPS.satellites<10) Serial.print('0');
      Serial.print(GPS.satellites); //2 digits total

      
      if (GPS.fix<10) Mo.print('0');
      Mo.print(GPS.fix);
      Mo.print(GPS.fixquality);
      if (GPS.latitude<10000) Mo.print('0'); //5+8 digits
          
      Mo.print(GPS.lat);
      if (GPS.longitude<10000) Mo.print('0');
      Mo.print(GPS.longitude, 10); //5+10 digits total 
      Mo.print(GPS.lon);
      for (int i=0; i<5-count_digits(GPS.altitude);i++){
        Mo.print('0');
      }
      Mo.print(GPS.altitude,2); //5+2 digits total
      if (GPS.satellites<10) Mo.print('0');
      Mo.print(GPS.satellites); //2 digits total
    
    }
   //Mo.print("254TheDarkKnightVersion2.0");
   Serial.println();
   Mo.println();
  } 
}

int count_digits (float x){
  int count = 0;
  int n = (int) x;
  while (n!=0){
    n/=10;
    count++;
  }
  return count;
}

void removePoint(char* gain, char* result){
  for (int i=0; i<sizeof(gain); i++){
    if (gain[i]=='.')
    {
      for (int k=i; k<sizeof(gain)-1; k++)
      {
        result[k]=gain[k+1];
      }
    }
    else{
      result[i] = gain[i];
    }
  }
}