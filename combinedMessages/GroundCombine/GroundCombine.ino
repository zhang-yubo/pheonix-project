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

typedef struct{
  int ID;
  int intOne;
  int intTwo;
  double doubleThree;
  double doubleFour;
  double doubleFive;
  double doubleSix;
  int intSeven;
} payload;

payload data;

//setting booleans
boolean displayData = false;

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial1.begin(115200);
  delay(10);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  #ifdef IS_RFM69HW_HCW
    radio.setHighPower(); //must include this only for RFM69HW/HCW!
  #endif

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

void loop() {
  listenCommand(); //read for any user input from terminal
  radioReceive();
}


void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

void listenCommand()
{
  if (Serial.available() > 0)
  {
    char input = Serial.read();

    if (input == 'D') //toggle displaying data to terminal
    {
      displayData = !displayData;
    }
    
    if (input == 'i')
    {
      Serial.print("DeviceID: ");
      word jedecid = flash.readDeviceId();
      Serial.println(jedecid, HEX);
    }
    
    if (input == 't') //print temperature
    {
      byte temperature =  radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
      byte fTemp = 1.8 * temperature + 32; // 9/5=1.8
      Serial.print( "Radio Temp is ");
      Serial.print(temperature);
      Serial.print("C, ");
      Serial.print(fTemp); //converting to F loses some resolution, obvious when C is on edge between 2 values (ie 26C=78F, 27C=80F)
      Serial.println('F');
    }
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

    data = *(payload*)radio.DATA;

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
        Serial.print(GATEWAYID);
        Serial.print(" - ACK...");
        delay(3); //need this when sending right after reception .. ?
        if (radio.sendWithRetry(NODEID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
          Serial.print("connected!");
        else Serial.print("nothing");
      }
    }
    
    Blink(LED_BUILTIN,3); //flash the LED
    displayAndSend();
    Serial.println();
  }
}


void displayAndSend()
{
  if (displayData)
  {
    Serial.println();
    Serial.print("Size: "); Serial.println(sizeof(data));
    Serial.print("ID: "); Serial.println(data.ID);
    Serial.print("fix: ");Serial.println(data.intOne);
    Serial.print("fix quality: ");Serial.println(data.intTwo);
    Serial.print("longitude: ");Serial.println(data.doubleThree);
    Serial.print("latitude: ");Serial.println(data.doubleFour);
    Serial.print("altitude: ");Serial.println(data.doubleFive);
    Serial.print("speed: ");Serial.println(data.doubleSix);
    Serial.print("number of satellites: ");Serial.println(data.intSeven);
  }
  
  byte buff[sizeof(data)];
  memcpy(buff, &data, sizeof(data));
  
  Serial1.print('$'); //delimiter to signify start of sentence to Mega
  for (int i=0; i<sizeof(buff); i++)
  {
    Serial1.println(buff[i]);
  }
}
