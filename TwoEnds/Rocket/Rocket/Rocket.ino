#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <RFM69.h>         
#include <RFM69_ATC.h>    
#include <SPIFlash.h>   
#include <SPI.h>  


#define NODEID        2    //unique(range up to 254, 255 is used for broadcast)
#define NETWORKID     100  //(range up to 255)
#define GATEWAYID     1
#define FREQUENCY   RF69_433MHZ
#define ENCRYPTKEY    "VCHS" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW_HCW  //only for RFM69HW/HCW
#define ENABLE_ATC
#define ATC_RSSI      -80


#define SERIAL_BAUD   115200

#if defined (MOTEINO_M0) && defined(SERIAL_PORT_USBVIRTUAL)
  #define Serial SERIAL_PORT_USBVIRTUAL // Required for Serial on Zero based boards
#endif

SPIFlash flash(SS_FLASHMEM, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif

Adafruit_GPS GPS(&Serial1);

int TRANSMITPERIOD = 500; //transmit a packet to gateway so often (in ms)
char buff[20];
int sendSize;
boolean requestACK = false;
boolean newGPSData = false;

typedef struct{
  int fix;
  int fixquality;
  double latitude;
  double longitude;
  double altitude;
  double velocity;
  int satellites;
}GPSpayload;
GPSpayload GPSdata;

typedef struct{
  double temperature;
  double latitude;
  double password;
  int poem;
}TPpayload;
TPpayload TPdata;

boolean GPSbuffer = true;

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Game on");

//radio
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW_HCW
  radio.setHighPower(); //must include this only for RFM69HW/HCW!
#endif

#ifdef ENABLE_ATC
  radio.enableAutoPower(ATC_RSSI);
#endif

  radio.promiscuous(promiscuousMode);

  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);

  if (flash.initialize())
  {
    Serial.print("SPI Flash Init OK ... UniqueID (MAC): ");
    flash.readUniqueId();
    for (byte i=0;i<8;i++)
    {
      Serial.print(flash.UNIQUEID[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }
  else
    Serial.println("SPI Flash MEM not found (is chip soldered?)...");

#ifdef ENABLE_ATC
  Serial.println("RFM69_ATC Enabled (Auto Transmission Control)\n");
#endif

  GPS.begin(9600);

  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  Serial.println(PMTK_Q_RELEASE);

}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

uint32_t timer = millis();
byte ackCount=0;
uint32_t packetCount = 0;

void loop() {

  if (millis() - timer > TRANSMITPERIOD) 
  {
    timer = millis();
    if (radio.receiveDone())
    {
      if (radio.DATA[0]=='%')
        GPSbuffer = false;
      if (radio.DATA[0]=='$')
        GPSbuffer = true; 

      if (radio.ACKRequested())
      {
        byte theNodeID = radio.SENDERID;
        radio.sendACK();
        Serial.print(" - ACK sent.");
  
        // When a node requests an ACK, respond to the ACK
        // and also send a packet requesting an ACK (every 3rd one only)
        // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
        if (ackCount++%3==0)
        {
          Serial.print(" Pinging node ");
          Serial.print(theNodeID);
          Serial.print(" - ACK...");
          delay(3); //need this when sending right after reception .. ?
          if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
            Serial.print("connected!");
          else Serial.print("nothing");
        }
      }
    }
    if(GPSbuffer)
    {
        
//        Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
//  
//        if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
//          return;
//      
//        Serial.println("GPS!");
//        GPSdata = {GPS.fix, GPS.fixquality, GPS.latitude, GPS.longitude, GPS.altitude, GPS.speed, GPS.satellites};

        GPSdata = {10, 20, 30, 40, 50, 60, 70};
        sendSize = sizeof(GPSdata);

        if (radio.sendWithRetry(GATEWAYID, (const void*)(&GPSdata), sizeof(GPSdata)))
          Serial.print("sent!");
        else Serial.print(" nothing...");

        Blink(LED_BUILTIN,3);
    }
    else if(!GPSbuffer)
    {
        TPdata = {30.0,GPS.latitude,1.23456,1};
        sendSize = sizeof(TPdata);
        
        if (radio.sendWithRetry(GATEWAYID, (const void*)(&TPdata), sizeof(TPdata)))
          Serial.print("sent!");
        else Serial.print(" nothing...");
        
        Blink(LED_BUILTIN,3);
    }

  }

}
