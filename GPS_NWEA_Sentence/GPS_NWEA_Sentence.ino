// Test code for Ultimate GPS Using Hardware Serial

// ------> https://www.adafruit.com/products/746
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

//#define GPSSerial Serial1;
SoftwareSerial GPSSerial(8,7);
Adafruit_GPS GPS(&GPSSerial);

void setup() {
  // wait for hardware serial to appear
  while (!Serial);

  // make this baud rate fast enough to we aren't waiting on it
  Serial.begin(115200);

  // 9600 baud is the default rate for the Ultimate GPS
  GPSSerial.begin(9600);
}


char seer[6];
int seer_i=0;
char buff[60];
int buff_i=0;
boolean active = false;
boolean see = false;


void loop() {
  if (GPSSerial.available()){
      char c = GPSSerial.read();
      if (c =='$'){     //start inspection, stop adding, start parsing
        buff_i=0;
        if (active){
          int commas=0;
          for (int i=0; i<sizeof(buff); i++){
            char x = buff[i];
            if (x==','){
              commas++;
            }
            if(commas>1&&commas<8){
              Serial.write(x);
            }
          }
          Serial.write('\n');
        }
        active=false;
        see=true;
      }
      if (see){           //inspection
        seer[seer_i] = c;
        seer_i++;        
        if (seer_i==sizeof(buff)){
          see = false;
          seer_i=0;
          active = (strcmp(seer,"$GPGGA")==0);
        }
      }
      if (active){        //adding
        buff[buff_i]=c;
        buff_i++;
      }
  }
}
