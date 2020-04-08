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
}GPSpayload;
GPSpayload groundGPS;
GPSpayload rocketGPS;

typedef struct{
  double temperature;
  double latitude;
  double password;
  int poem;
}TPpayload;
TPpayload TPdata;

Adafruit_GPS g_GPS(&Serial1); //set GPS serial to Serial1


boolean GPSECHO = false;
boolean Display = true;
boolean Motor = false;
boolean newData = false;
boolean Receiving = true;


void setup() {
  Serial.begin(115200);
  g_GPS.begin(9600);     //GPS = Serial 1
  Serial2.begin(115200); //Moteino = Serial 2
  Serial3.begin(9600); //Motor = Serial 3
  Serial.println("game on");
}

void loop() {
  char input = Serial.read();
  if (input == '$')
  {
    Serial2.println('$');
    Serial.println("Receiving GPS data");
  }
  if (input == '%')
  {
    Serial2.println('%');
    Serial.println("Receiving TP data");
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
      rocketGPS = *(GPSpayload*) buff;

      displayRocketData();
    }
    if(Serial2.read()=='%')
    { 
      byte buff[14];
      for (int i=0; i<14; i++)
      {
        buff[i]=Serial2.parseInt();
      }
      
      newData = true; 
      TPdata = *(TPpayload*) buff;

      displayTPData();
    }
  }
}

void displayRocketData()
{
    Serial.println("----------Rocket----------");
    
    Serial.print("fix: ");Serial.println(rocketGPS.fix);
    Serial.print("fixquality: ");Serial.println(rocketGPS.fixquality);
    Serial.print("longitude: ");Serial.println(rocketGPS.longitude,8);
    Serial.print("latitude: ");Serial.println(rocketGPS.latitude,8);
    Serial.print("altitude: ");Serial.println(rocketGPS.altitude,3);
    Serial.print("speed: ");Serial.println(rocketGPS.velocity);
    Serial.print("number satelites: ");Serial.println(rocketGPS.satellites);
}

void displayTPData()
{
  Serial.print("1:");Serial.println(TPdata.temperature);
  Serial.print("2:");Serial.println(TPdata.latitude);
  Serial.print("3:");Serial.println(TPdata.password);
  Serial.print("4:");Serial.println(TPdata.poem);
}
