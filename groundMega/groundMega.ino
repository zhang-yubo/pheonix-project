#include <Adafruit_GPS.h>
#include <math.h>


typedef struct{
  int fix;
  int fixquality;
  double latitude;
  double longitude;
  double altitude;
  double velocity;
  int satellites;
}Payload;
Payload groundGPS;
Payload rocketGPS;

Adafruit_GPS g_GPS(&Serial1); //set GPS serial to Serial1


boolean GPSECHO = true;
boolean Display = true;
boolean Motor = false;
boolean newData = false;

void setup() {
  Serial.begin(115200);
  g_GPS.begin(9600);     //GPS = Serial 1
  Serial2.begin(115200); //Moteino = Serial 2
  Serial3.begin(115200); //Motor = Serial 3

  Serial.println("game on");
}

int period = 2000;
uint32_t timer = millis();
void loop() {
  if (Serial.available()>0)
  {
    char input = Serial.read();

    if (input == 'D')
      Display = !Display;
    
    if (input == 'G')
      GPSECHO = !GPSECHO;

    if (input == 'M')
      Motor = !Motor;

    if (input >= 48 && input <= 57) //[0,9]
    {
      period = 100 * (input-48);
      if (period == 0) period = 1000;
      Serial.print("\nChanging delay to ");
      Serial.print(period);
      Serial.println("ms\n");
    }
  }

  if (timer > millis())  timer = millis();

  if (millis() - timer > period) {
    timer = millis();
    
  }

  if (Serial2.available())
  {
    if(Serial2.read()=='$')
    { 
      byte buff[22];
      for (int i=0; i<22; i++)
      {
        buff[i]=Serial2.parseInt();
      }
      
      newData = true; 
      rocketGPS = *(Payload*) buff;

      if (GPSECHO)
      {
        displayNMEA();
      }
      else
      {
        if (Display)
        {
          displayRocketData();
          displayGroundData();
        }
  
        if (Motor)
          motorCommand();
      }
    }

    


  }

  if (g_GPS.available())
  {
    char c = g_GPS.read();
      
    if (g_GPS.newNMEAreceived())
    {
      groundGPS = {
        g_GPS.fix, 
        g_GPS.fixquality, 
        g_GPS.latitude, 
        g_GPS.longitude, 
        g_GPS.altitude, 
        g_GPS.speed,
        g_GPS.satellites
      };
      
      if (!g_GPS.parse(g_GPS.lastNMEA()))
      {
        return;
      }
      // this also sets the newNMEAreceived() flag to false
        
    }

    
  }
 
  
  
}

void displayRocketData()
{
    Serial.println("----------Rocket----------");
    if (rocketGPS.fix)
    {
      Serial.print("fix: ");Serial.println(rocketGPS.fix);
      Serial.print("fixquality: ");Serial.println(rocketGPS.fixquality);
      Serial.print("longitude: ");Serial.println(rocketGPS.longitude,8);
      Serial.print("latitude: ");Serial.println(rocketGPS.latitude,8);
      Serial.print("altitude: ");Serial.println(rocketGPS.altitude,3);
      Serial.print("speed: ");Serial.println(rocketGPS.velocity);
      Serial.print("number satelites: ");Serial.println(rocketGPS.satellites);
    }
    if (!rocketGPS.fix)
    {
      Serial.println("On board GPS not fixed");
    }
    if (!newData)
    {
      Serial.println("No rocket signal");
    }
    
    Serial.println("--------------------------");
    newData = false;
}

void displayGroundData()
{
    Serial.println("----------Ground----------");
    if (groundGPS.fix)
    {
      Serial.print("fix: ");Serial.println(groundGPS.fix);
      Serial.print("fixquality: ");Serial.println(groundGPS.fixquality);
      Serial.print("longitude: ");Serial.println(groundGPS.longitude,8);
      Serial.print("latitude: ");Serial.println(groundGPS.latitude,8);
      Serial.print("altitude: ");Serial.println(groundGPS.altitude,3);
      Serial.print("number satelites: ");Serial.println(groundGPS.satellites);
    }
    else
    {
      Serial.println("No ground data");
    }
    Serial.println("--------------------------");
}

void motorCommand()
{
  double risingAngle = atan2 (rocketGPS.altitude, pow(pow(rocketGPS.latitude,2)+pow(rocketGPS.longitude,2),0.5));
  risingAngle *= 180/3.141593 ;

  double lonDiff = (groundGPS.longitude - rocketGPS.longitude)*1000000;
  double latDiff = (rocketGPS.latitude - groundGPS.latitude)*1000000; //reduce digits
  double offSouth = 180;

  offSouth = atan2 (lonDiff, latDiff);
  offSouth *= 180/M_PI;
  offSouth =+ 180;

//  if (lonDiff < 0 && latDiff == 0) offNorth = 90; //if rocket directly west of groundstation
//  else if (lonDiff > 0 && latDiff == 0) offNorth = 270; //if rocket directly east of groundstation
//  else
//  {
//    offNorth = atan2 (lonDiff, latDiff);
//    offNorth *= 180/3.141593;
//    //******the following calculations point north at 180 deg azimuth and use north-south as reference*****
//    if (lonDiff < 0 && latDiff < 0) offNorth += 0; //if rocket southwest of groundstation
//    else if (lonDiff < 0 && latDiff > 0) offNorth += 180; //if rocket north of groundstation
//    else if (lonDiff > 0 && latDiff > 0) offNorth += 180;
//    else if (lonDiff > 0 && latDiff < 0) offNorth +=360; //if rocket southeast of groundstation
//  }

  
  Serial.println("----------Motor-----------");
  Serial.print("degrees off South: "); Serial.print(offSouth);
  Serial.print(" rising angle: "); Serial.println(risingAngle);
  Serial.print("latDiff: "); Serial.print(latDiff); Serial.print(" lonDiff: "); Serial.print(lonDiff);
  Serial.println();
  Serial.println("--------------------------");
  
  Serial3.print(offSouth);
  Serial3.print(risingAngle);
}

void displayNMEA() 
{      
    String sentence = "GPRMC,";
    
    if (g_GPS.hour<10)
      sentence += "0";
    sentence += g_GPS.hour;

    if (g_GPS.minute<10)
      sentence += "0";
    sentence += g_GPS.minute;

    if (g_GPS.seconds<10)
      sentence += "0";
    sentence += g_GPS.seconds; 
    
    sentence += "."; sentence += g_GPS.milliseconds;sentence += ",";
    sentence += (int)rocketGPS.latitude;
    sentence += ".";
    if ((int)((rocketGPS.latitude - (int)rocketGPS.latitude)*1e5)<1e5)
      sentence += "0";
    sentence += (int)((rocketGPS.latitude - (int)rocketGPS.latitude)*1e5);
    sentence += ",";
    
    sentence += g_GPS.lat;sentence += ",";
    sentence += (int)rocketGPS.longitude;
    sentence += ".";
    if ((int)((rocketGPS.longitude - (int)rocketGPS.longitude)*1e5)<1e5)
      sentence += "0";
    sentence += (int)((rocketGPS.longitude - (int)rocketGPS.longitude)*1e5);
    sentence += ",";
    
    sentence += g_GPS.lon;sentence += ",";
    sentence += rocketGPS.velocity;sentence += ",";
    sentence += "100,";

    if (g_GPS.day<10)
      sentence += "0";
    sentence += g_GPS.day;

    if (g_GPS.month<10)
      sentence += "0";
    sentence += g_GPS.month;
    
    sentence += g_GPS.year;
    sentence += ",";
    
    sentence += "0.0,";
    sentence += "E,";
    sentence += "A";
    Serial.print("$");
    Serial.print(sentence);
    Serial.print("*");
    Serial.print(checkSum(sentence),HEX);
    Serial.print((char)13);
    Serial.print((char)10);

//      Serial.println("$GPRMC,232852.0,3717.0754323,N,12146.8765908,W,0.02,100,170220,0.0,E,A*56");

//    if (g_GPS.hour<10)
//      Serial.print("0");
//    Serial.print(g_GPS.hour);
//
//    if (g_GPS.minute<10)
//      Serial.print("0");
//    Serial.print(g_GPS.minute);
//
//    if (g_GPS.seconds<10)
//      Serial.print("0");
//    Serial.print(g_GPS.seconds); 
//    
//    Serial.print("."); Serial.print(g_GPS.milliseconds);Serial.print(",");
//    Serial.print(rocketGPS.latitude,7);Serial.print(",");
//    Serial.print(g_GPS.lat);Serial.print(",");
//    Serial.print(rocketGPS.longitude,7);Serial.print(",");
//    Serial.print(g_GPS.lon);Serial.print(",");
//    Serial.print(rocketGPS.velocity);Serial.print(",");
//    Serial.print("100,");
//    Serial.print(g_GPS.day);Serial.print(g_GPS.month);Serial.print(g_GPS.year);Serial.print(",");
//    Serial.print("0.0,");
//    Serial.print("E,");
//    Serial.print("A");
//    Serial.print("*");
//    Serial.print((char)13);
//    Serial.print((char)10);

}

char checkSum(String str)
{
  char check = 0;
  for (int i=0; i<str.length(); i++)
  {
    check = char (check ^ str.charAt(i));
  }
  return check;
}
