#include <Adafruit_GPS.h>


typedef struct{
  int fix;
  int fixquality;
  double latitude;
  double longitude;
  double altitude;
  int satellites;
}Payload;
Payload groundGPS;
Payload rocketGPS;

Adafruit_GPS g_GPS(&Serial1);


boolean GPSECHO = false;
boolean Display = true;
boolean Motor = false;

void setup() {
  Serial.begin(115200);
  g_GPS.begin(9600);     //GPS = Serial 1
  Serial2.begin(115200); //Moteino = Seial 2
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
    {
      GPSECHO = !GPSECHO;
      if (GPSECHO)
        Serial2.print('G');
      else
        Serial2.print('S');
    }

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
      byte buff[18];
      for (int i=0; i<18; i++)
      {
        buff[i]=Serial2.parseInt();
        Serial.print(i);
      }
      
      Serial.println();
      rocketGPS = *(Payload*) buff;

      if (DISPLAY)
      {
        displayRocketData();
        displayGroundData();
      }

      if (Motor)
        motorCommand();
    }

    if(Serial2.read()=='%')
    { 
      char in = Serial2.read();
      while (in != '%' && in != '$')
      {
        Serial.print(in);
        in = Serial2.read();
      }
      Serial.println();
    }

    


  }
  if (g_GPS.available())
  {
    if (GPSECHO)
    {
      Serial.write(g_GPS.read());
    }
      
      
    if (g_GPS.newNMEAreceived())
    {
      groundGPS = {
        g_GPS.fix, 
        g_GPS.fixquality, 
        g_GPS.latitude, 
        g_GPS.longitude, 
        g_GPS.altitude, 
        g_GPS.satellites
      };
      
      Serial.println(g_GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
      if (!g_GPS.parse(g_GPS.lastNMEA()))
      {return;}// this also sets the newNMEAreceived() flag to false
        
    }
  }
  
  
 
  
  
}

void displayRocketData()
{
    Serial.println("----------Rocket----------");
    if (rocketGPS.fix == 0)
      Serial.println("rocket GPS not fixed");
    else
    {
      Serial.print("fix: ");Serial.println(rocketGPS.fix);
      Serial.print("fixquality: ");Serial.println(rocketGPS.fixquality);
      Serial.print("longitude: ");Serial.println(rocketGPS.longitude,8);
      Serial.print("latitude: ");Serial.println(rocketGPS.latitude,8);
      Serial.print("altitude: ");Serial.println(rocketGPS.altitude,3);
      Serial.print("number satelites: ");Serial.println(rocketGPS.satellites);
    }
    Serial.println("--------------------------");
}

void displayGroundData()
{
    Serial.println("----------Ground----------");
    if (groundGPS.fix == 0)
      Serial.println("ground GPS not fixed");
    else
    {
      Serial.print("fix: ");Serial.println(groundGPS.fix);
      Serial.print("fixquality: ");Serial.println(groundGPS.fixquality);
      Serial.print("longitude: ");Serial.println(groundGPS.longitude,8);
      Serial.print("latitude: ");Serial.println(groundGPS.latitude,8);
      Serial.print("altitude: ");Serial.println(groundGPS.altitude,3);
      Serial.print("number satelites: ");Serial.println(groundGPS.satellites);
    }
    Serial.println("--------------------------");
}

void motorCommand()
{
  double risingAngle = atan2 (rocketGPS.altitude, pow(pow(rocketGPS.latitude,2)+pow(rocketGPS.longitude,2),0.5));
  risingAngle *= 180/3.141593 ;

  double latDiff = rocketGPS.latitude - groundGPS.latitude;
  double lonDiff = rocketGPS.longitude - groundGPS.longitude;

  double offNorth = atan2 (lonDiff, latDiff);
  offNorth *= 180/3.141593;
  if (lonDiff > 0 && latDiff < 0) offNorth += 180;
  if (lonDiff < 0 && latDiff < 0) offNorth -= 180;

  
  Serial.println("----------Motor-----------");
  Serial.print("degrees off North: "); Serial.print(offNorth);
  Serial.print("rising angle: "); Serial.print(risingAngle);
  Serial.println();
  Serial.println("--------------------------");
  
  Serial3.print(offNorth);
  Serial3.print(risingAngle);
}
