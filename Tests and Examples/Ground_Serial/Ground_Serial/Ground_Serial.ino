#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>

typedef struct{
  int fix;
  int fixquality;
  double latitude;
  double longitude;
  double altitude;
  int satellites;
}Payload;
Payload data;


SoftwareSerial GPSerial(3, 2);
Adafruit_GPS GPS(&GPSerial);


void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  GPS.begin(9600);

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PGCMD_ANTENNA);
}


void loop() {
  
  data = {60, 99, 77, 442.45, 68.5, GPS.satellites};

  byte buff[sizeof(data)];
  
  memcpy(buff, &data, sizeof(data));

  Serial1.print('\n');

  for (int i=0; i<sizeof(buff); i++)
  {
    Serial1.print(buff[i]);
  }


  Payload rocket = *(Payload*)buff;

  

  delay(2000);
  Blink(LED_BUILTIN,3);

}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
