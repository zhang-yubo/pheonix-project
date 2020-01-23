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

boolean GPSECHO = true;

void setup() {
  Serial.begin(115200);
  g_GPS.begin(9600);
  Serial2.begin(115200);
  
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
    
    if(Serial2.read()=='\n')
    { 
      byte buff[32];
      Serial2.readBytes(buff,32);
      byte data[32];
      for (int i=0; i<32; i++)
      {
        data[i] = buff[i]-48;
        Serial.print(data[i]);
      }
      Serial.println();
      
      rocketGPS = *(Payload*) buff;
      Serial.println(rocketGPS.fix);
    }
    
   

  }
  if (g_GPS.available())
  {
    if (GPSECHO)
      Serial.write(g_GPS.read());

    groundGPS = {g_GPS.fix, g_GPS.fixquality, g_GPS.latitude, g_GPS.longitude, g_GPS.altitude, g_GPS.satellites};

  }

  
  int risingAngle = atan2 (rocketGPS.altitude, pow(pow(rocketGPS.latitude,2)+pow(rocketGPS.longitude,2),0.5));
  risingAngle *= 180/3.141593 ;
  
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
