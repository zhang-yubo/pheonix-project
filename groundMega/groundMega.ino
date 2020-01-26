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

void setup() {
  Serial.begin(115200);
  g_GPS.begin(9600);     //GPS = Serial 1
  Serial2.begin(115200); //Moteino = Seial 2
  Serial3.begin(115200); //Motor = Serial 3
  
}

int period = 2000;
uint32_t timer = millis();
void loop() {
  if (Serial.available()>0)
  {
    char input = Serial.read();

    if (input == 'E')
      GPSECHO = !GPSECHO;

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
      byte buff[32];
      for (int i=0; i<32; i++)
      {
        buff[i]=Serial2.parseInt();
      }
      
      
      rocketGPS = *(Payload*) buff;
      Serial.println(rocketGPS.latitude);
    }
    
   

  }
  if (g_GPS.available())
  {
    if (GPSECHO)
      Serial.write(g_GPS.read());

    groundGPS = {g_GPS.fix, g_GPS.fixquality, g_GPS.latitude, g_GPS.longitude, g_GPS.altitude, g_GPS.satellites};

  }

  
  
  //motorCommand();
  
  
}

void displayRocketData()
{
    Serial.print("fix: ");Serial.println(rocketGPS.fix);
    Serial.print("fixquality: ");Serial.println(rocketGPS.fixquality);
    Serial.print("longitude: ");Serial.println(rocketGPS.longitude);
    Serial.print("latitude: ");Serial.println(rocketGPS.latitude);
    Serial.print("altitude: ");Serial.println(rocketGPS.altitude);
    Serial.print("number satelites: ");Serial.println(rocketGPS.satellites);
}

void displayGroundData()
{
    Serial.print("fix: ");Serial.println(groundGPS.fix);
    Serial.print("fixquality: ");Serial.println(groundGPS.fixquality);
    Serial.print("longitude: ");Serial.println(groundGPS.longitude);
    Serial.print("latitude: ");Serial.println(groundGPS.latitude);
    Serial.print("altitude: ");Serial.println(groundGPS.altitude);
    Serial.print("number satelites: ");Serial.println(groundGPS.satellites);
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

  Serial.print("degrees off North: "); Serial.print(offNorth);
  Serial.print("rising angle: "); Serial.print(risingAngle);

  Serial3.print(offNorth);
  Serial3.print(risingAngle);
}
