// Adafruit GPS modules using MTK3329/MTK3339 driver

// Adafruit Ultimate GPS module using MTK33x9 chipset
//    ------> http://www.adafruit.com/products/746

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

int TRANSMITPERIOD = 2000; //transmit a packet to gateway so often (in ms)
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
}Payload;
Payload data;

SPIFlash flash(SS_FLASHMEM, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif


Adafruit_GPS GPS(&Serial1);
boolean GPSECHO = false;
boolean SEND = true;


void setup()
{

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
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


//GPS
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  //GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  // Ask for firmware version
  Serial1.println(PMTK_Q_RELEASE);
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

uint32_t timer = millis();

void loop()
{
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    
    if (input == 'G') 
      GPSECHO = !GPSECHO;

    if (input == 'E') //E=enable encryption
      radio.encrypt(ENCRYPTKEY);
    if (input == 'e') //e=disable encryption
      radio.encrypt(null);

    if (input >= 48 && input <= 57) //[0,9]
    {
      TRANSMITPERIOD = 100 * (input-48);
      if (TRANSMITPERIOD == 0) TRANSMITPERIOD = 1000;
      Serial.print("\nChanging delay to ");
      Serial.print(TRANSMITPERIOD);
      Serial.println("ms\n");
    }

    if (input == 'S') 
    SEND = !SEND;
  }


  char c = GPS.read();
  if (GPSECHO)
    if (c) Serial.print(c); //if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived())
  {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  if (timer > millis())  timer = millis(); // if millis() or timer wraps around, just reset it

  if (millis() - timer > TRANSMITPERIOD) {
    timer = millis();
    
    Serial.println("GPS!");
    data = {GPS.fix, GPS.fixquality, GPS.latitude, GPS.longitude, GPS.altitude, GPS.speed, GPS.satellites};

    sendSize = sizeof(data);

    if (SEND){
      if (radio.receiveDone())
      {
        Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
        for (byte i = 0; i < radio.DATALEN; i++)
          Serial.print((char)radio.DATA[i]);
        Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
      
        if (radio.ACKRequested())
        {
          radio.sendACK();
          Serial.print(" - ACK sent");
        }
        Blink(LED_BUILTIN,3);
        Serial.println();
      }
    
    
      //send FLASH 
      if(sendSize==0)
      {
        sprintf(buff, "FLASH_MEM_ID:0x%X", flash.readDeviceId());
        byte buffLen=strlen(buff);
        if (radio.sendWithRetry(GATEWAYID, buff, buffLen))
          Serial.print(" connected!");
        else Serial.print(" nothing...");
      }
      else
      {
        Serial.print("Sending[");
        Serial.print(sendSize);
        Serial.print("]: ");
        for(int i = 0; i < sendSize; i++)
          Serial.print(i);
    
        if (radio.sendWithRetry(GATEWAYID, (const void*)(&data), sizeof(data)))
          Serial.print("sent!");
        else Serial.print(" nothing...");
      }
      Serial.println();
      Blink(LED_BUILTIN,3);
    }

    else
    {
       displayData();
    }
    
  }
  
}

void displayData()
{   
    Serial.print("fix: ");Serial.println(GPS.fix);
    Serial.print("fixquality: ");Serial.println(GPS.fixquality);
    Serial.print("longitude: ");Serial.println(GPS.longitude);
    Serial.print("latitude: ");Serial.println(GPS.latitude);
    Serial.print("altitude: ");Serial.println(GPS.altitude);
    Serial.print("speed: ");Serial.println(GPS.speed);
    Serial.print("number satelites: ");Serial.println(GPS.satellites);
    Serial.println();
  
    Serial.print("sentfix: ");Serial.println(data.fix);
    Serial.print("sentfixquality: ");Serial.println(data.fixquality);
    Serial.print("sentlongitude: ");Serial.println(data.longitude);
    Serial.print("sentlatitude: ");Serial.println(data.latitude);
    Serial.print("sentaltitude: ");Serial.println(data.altitude);
    Serial.print("sentspeed: ");Serial.println(data.velocity);
    Serial.print("sentnumber satelites: ");Serial.println(data.satellites);
    Serial.println();
}
