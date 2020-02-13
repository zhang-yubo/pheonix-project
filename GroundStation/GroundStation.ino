
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

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial1.begin(115200);
  delay(10);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW_HCW
  radio.setHighPower(); //must include this only for RFM69HW/HCW!
#endif
  radio.promiscuous(promiscuousMode);
  //radio.setFrequency(919000000); //set frequency to some custom frequency
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
    
    //alternative way to read it:
    //byte* MAC = flash.readUniqueId();
    //for (byte i=0;i<8;i++)
    //{
    //  Serial.print(MAC[i], HEX);
    //  Serial.print(' ');
    //}
  }
  else
    Serial.println("SPI Flash MEM not found (is chip soldered?)...");
    
#ifdef ENABLE_ATC
  Serial.println("RFM69_ATC Enabled (Auto Transmission Control)");
#endif
}

byte ackCount=0;
uint32_t packetCount = 0;
boolean sendStruct = true;
boolean sendSentence = false;

typedef struct{
  int fix;
  int fixquality;
  double latitude;
  double longitude;
  double altitude;
  int satellites;
}Payload;
Payload data;

void loop() {
  //process any serial input
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    
    if (input == 't')
    {
      byte temperature =  radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
      byte fTemp = 1.8 * temperature + 32; // 9/5=1.8
      Serial.print( "Radio Temp is ");
      Serial.print(temperature);
      Serial.print("C, ");
      Serial.print(fTemp); //converting to F loses some resolution, obvious when C is on edge between 2 values (ie 26C=78F, 27C=80F)
      Serial.println('F');
    }

    if (input == 'S')
    {
      sendStruct = true;
      sendSentence = false;
      while (radio.sendWithRetry(GATEWAYID, "S", 1));
      Serial.println("Receiving structs");
    }

    if (input == 'G')
    {
      sendStruct = false;
      sendSentence = true;
      while (radio.sendWithRetry(GATEWAYID, "G", 1));
      Serial.println("Receiving sentences");
    }
  }

  if (Serial1.available() > 0)
  {
    char input = Serial1.read();
    
    if (input == 'S')
    {
      sendStruct = true;
      sendSentence = false;
    }

    if (input == 'G')
    {
      sendStruct = false;
      sendSentence = true;
    }
  }
  
  radioReceive();

}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}


void displaySendStruct()
{
    data = *(Payload*)radio.DATA;
    
    if (data.fix!=0)
    {
      Serial.print("fix: ");Serial.println(data.fix);
      Serial.print("fixquality: ");Serial.println(data.fixquality);
      Serial.print("longitude: ");Serial.println(data.longitude);
      Serial.print("latitude: ");Serial.println(data.latitude);
      Serial.print("altitude: ");Serial.println(data.altitude);
      Serial.print("number satelites: ");Serial.println(data.satellites);
    }

    else
      Serial.print("No fix");
      
    byte buff[sizeof(data)];
  
    memcpy(buff, &data, sizeof(data));
  
    Serial1.print('$');
  
    for (int i=0; i<sizeof(buff); i++)
    {
      Serial1.println(buff[i]);
    }
}

void displaySendSentence()
{
    Serial1.print('%');

    for (int i=0; i<radio.DATALEN; i++)
    {
      Serial1.println(radio.DATA[i]);
    }
}

void radioReceive()
{
  if (radio.receiveDone())
  {
    Serial.print("#[");
    Serial.print(++packetCount);
    Serial.print(']');
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    if (promiscuousMode)
    {
      Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
    }
    
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
    
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

    if (sendStruct)
      displaySendStruct();

    if (sendSentence)
      displaySendSentence();
    
    Serial.println();
    Blink(LED_BUILTIN,3);
  }
  
}
