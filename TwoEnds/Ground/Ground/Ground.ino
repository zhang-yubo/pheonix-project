#include <RFM69.h>         
#include <RFM69_ATC.h>     
#include <SPIFlash.h>
#include <SPI.h>
#include <SoftwareSerial.h>


#define NODEID        1
#define NETWORKID     100
#define GATEWAYID     2
#define FREQUENCY     RF69_433MHZ
#define ENCRYPTKEY    "VCHS"
#define IS_RFM69HW_HCW  //uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!


#define ENABLE_ATC    
#define SERIAL_BAUD   115200

#if defined (MOTEINO_M0) && defined(SERIAL_PORT_USBVIRTUAL)
  #define Serial SERIAL_PORT_USBVIRTUAL // Required for Serial on Zero based boards
#endif

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif


SPIFlash flash(SS_FLASHMEM, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

boolean GPSbuffer = true; 

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial1.begin(115200);
  delay(10);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  #ifdef IS_RFM69HW_HCW
    radio.setHighPower(); //must include this only for RFM69HW/HCW!
  #endif
  radio.promiscuous(promiscuousMode);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  if (flash.initialize())
  {
    Serial.print("SPI Flash Init OK. Unique MAC = [");
    flash.readUniqueId();
    for (byte i=0;i<8;i++)
    {
      Serial.print(flash.UNIQUEID[i], HEX);
      if (i!=8) Serial.print(':');
    }
    Serial.println(']');
  }
  else
    Serial.println("SPI Flash MEM not found (is chip soldered?)...");
    
  #ifdef ENABLE_ATC
    Serial.println("RFM69_ATC Enabled (Auto Transmission Control)");
  #endif

}

byte ackCount=0;
uint32_t packetCount = 0;

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

void loop() {
  radioReceive();
  listenCommand();
}

void radioReceive()
{
  if (radio.receiveDone())
  {
    Serial.print("#[");
    Serial.print(++packetCount);
    Serial.print(']');
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");

    if (radio.ACKRequested())
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
      Serial.print(" - ACK sent.");
    }

    if (GPSbuffer)
    {
      GPSdata = *(GPSpayload*)radio.DATA;
    }
    else
    {
      TPdata = *(TPpayload*)radio.DATA;
    }
      
    Send();
    Serial.println();
    Blink(LED_BUILTIN,3);
  }
  
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}


void Send()
{
    byte buff[sizeof(GPSdata)];
    memcpy(buff, &GPSdata, sizeof(GPSdata));

    if (GPSbuffer)
    {
      Serial.print('$');
    }
    else
    {
      Serial.print('%');
    }
    
    for (int i=0; i<sizeof(buff); i++)
    {
      Serial.print(buff[i]);
    }
    Serial.println();
}

void listenCommand()
{
    if (Serial.available()>0)
    {
        char input = Serial.read();
        if (input == '$')
        {
          char b[1] = "$";
          if (radio.sendWithRetry(GATEWAYID, b, 1, 0))
            Serial.println("Changing to GPSbuffer");
          else Serial.print("nothing");
          GPSbuffer = true;
        }
        if (input == '%')
        {
          char b[1] = "%";
          if (radio.sendWithRetry(GATEWAYID, b, 1, 0))
            Serial.println("Changing to TPbuffer");
          else Serial.print("nothing");
          GPSbuffer = false;
        }
    }
}
