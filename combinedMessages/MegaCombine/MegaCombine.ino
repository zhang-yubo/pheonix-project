#include <Adafruit_GPS.h>
#include <math.h>

typedef struct{
  int ID;
  int intOne;
  int intTwo;
  double doubleThree;
  double doubleFour;
  double doubleFive;
  double doubleSix;
  int intSeven;
} payload; //data type payload stores information

payload groundGPS;
payload rocketData; //stores data from rocket for sorting into GPS or TP data
payload rocketGPS;
payload rocketTP;


Adafruit_GPS g_GPS(&Serial1); //set GPS serial to Serial1

int angleOffset = 180; //motor centered at north

boolean GPSECHO = false; //control and display booleans
boolean Display = true;
boolean Motor = false;

boolean newData = false; //booleans that check for data
boolean Receiving = false;
boolean receivingGPS = false;
boolean receivingTP = false;

void setup() {
  Serial.begin(115200);
  g_GPS.begin(9600);     //GPS = Serial 1
  Serial2.begin(115200); //Moteino = Serial 2
  Serial3.begin(9600); //Motor = Serial 3

  Serial.println("game on"); //initialize
}

int period = 2000; //information displayed every 2000 ms
uint32_t timer = millis();

void loop()
{
  if (Serial.available() > 0) //input commands from terminal
  {
    char input = Serial.read();

    if (input == 'D') //toggle display of GPS information
      Display = !Display;
    
    if (input == 'G') //toggle display of raw NMEA sentences
      GPSECHO = !GPSECHO;

    if (input == 'M') //toggle motor control and calculations; turn controller on THEN enter M
    {
      for (int i=0; i<10; i++) //send a carriage return several times to allow controller to match baud rate
      {
        Serial3.write('\r');
        delay(200);
      }
      Motor = !Motor;
    }

    //center motor north or south

    if (input == 'S') //center south at 180 deg (degrees off north)
    {
      angleOffset = 0;
    }
    if (input == 'N') //center north at 180 deg (degrees off south)
    {
      angleOffset = 180;
    }
    
    if (input >= 48 && input <= 57) //[0,9]
    {
      period = 100 * (input - 48);
      if (period == 0) period = 1000;
      Serial.print("\nChanging delay to ");
      Serial.print(period);
      Serial.println("ms\n");
    }
  }

  
  if (timer > millis()) //if timer wraps around, reset it
  {
    timer = millis();
  }

  if (Serial2.available()) //input from Moteino
  {
    Receiving = true;
    
    if (Serial2.read() == '@') //delimiter marks beginning of sentence
    { 
      byte buff[24];
      
      for (int i=0; i<24; i++)
      {
        buff[i]=Serial2.parseInt(); //store data into "buff" array
      }
      
      newData = true; 
      rocketData = *(payload*) buff; //store array into payload struct "rocketData"
      
      if (rocketData.ID == 1) //if GPS data ID "1" is detected
      {
        memcpy(&rocketData, &rocketGPS, sizeof(rocketData)); //store rocketData as GPS data
        receivingGPS = true;
      }
      if (rocketData.ID == 2) //if TP data ID "2" is detected
      {
        memcpy(&rocketData, &rocketTP, sizeof(rocketData)); //store rocketData as TP data
        receivingTP = true;
      }
      else
      {
        Serial.println("Data type IDs not detected");
        Serial.print("Detected IDs: "); Serial.println(rocketData.ID);
        for (int i=0; i<sizeof(buff); i++)
        {
          Serial.print(buff[i]);
        }
      }
    }

    if (Serial2.read() !='@')
    {
      char empty = Serial2.read();
    }
  }
  
  if (g_GPS.available()) //input from ground GPS
  {
    char c = g_GPS.read();
      
    if (g_GPS.newNMEAreceived())
    {
      groundGPS = {
        3,
        g_GPS.fix, 
        g_GPS.fixquality, 
        g_GPS.latitude, 
        g_GPS.longitude, 
        g_GPS.altitude, 
        g_GPS.speed,
        g_GPS.satellites
      }; //store GPS sentence into groundGPS
      
      if (!g_GPS.parse(g_GPS.lastNMEA()))
      {
        return;
      }// this also sets the newNMEAreceived() flag to false
    }
  }

  if (millis() - timer > period) //every period print information/command motor
  {
    if (GPSECHO)
    {
//      displayNMEA();
    }

    if (Display)
    {
      displayRocketData();
      displayGroundData();
    }

    if (Motor)
    {
//      motorCommand();
    }
    
    timer = millis(); 
  }
}

//---End of loop, functions below-----

void displayRocketData()
{
  Serial.println("----------Rocket----------");
  
  if (Receiving == false)
  {
    Serial.println("no Moteino signal");
  }
  
  if (Receiving == true)
  {
    if (receivingGPS == false)
    {
      Serial.println("Rocket GPS data not received");
    }
    
    if (receivingGPS == true)
    {
      Serial.println("-----------GPS-----------");
      if (rocketGPS.intOne)
      {
        Serial.print("fix: ");Serial.println(rocketGPS.intOne);
        Serial.print("fix quality: ");Serial.println(rocketGPS.intTwo);
        Serial.print("longitude: ");Serial.println(rocketGPS.doubleThree,8);
        Serial.print("latitude: ");Serial.println(rocketGPS.doubleFour, 8);
        Serial.print("altitude: ");Serial.println(rocketGPS.doubleFive,3);
        Serial.print("speed: ");Serial.println(rocketGPS.doubleSix);
        Serial.print("# of satellites: ");Serial.println(rocketGPS.intSeven);
      }
    
      if (!rocketGPS.intOne)
      {
        Serial.println("On board GPS not fixed");
      }

      receivingGPS = false;
    }


    if (receivingTP == false)
    {
      Serial.println("Rocket TP data not received");
    }

    if (receivingTP == true)
    {
      Serial.println("----------TP----------");
      Serial.print("1: ");Serial.println(rocketTP.intOne);
      Serial.print("2: ");Serial.println(rocketTP.intTwo);
      Serial.print("3: ");Serial.println(rocketTP.doubleThree);
      Serial.print("4: ");Serial.println(rocketTP.doubleFour);
      Serial.print("5: ");Serial.println(rocketTP.doubleFive);
      Serial.print("6: ");Serial.println(rocketTP.doubleSix);
      Serial.print("7: ");Serial.println(rocketTP.intSeven);

      receivingTP = false;
    }
    

    newData = false;
    Receiving = false;
  }
  
  Serial.println("--------------------------");
}

void displayGroundData()
{
    Serial.println("----------Ground----------");
    if (groundGPS.intOne)
    {
      Serial.print("ID: ");Serial.println(groundGPS.ID);
      Serial.print("fix: ");Serial.println(groundGPS.intOne);
      Serial.print("fixquality: ");Serial.println(groundGPS.intTwo);
      Serial.print("longitude: ");Serial.println(groundGPS.doubleThree,8);
      Serial.print("latitude: ");Serial.println(groundGPS.doubleFour,8);
      Serial.print("altitude: ");Serial.println(groundGPS.doubleFive,3);
      Serial.print("# of satellites: ");Serial.println(groundGPS.doubleSix);
      Serial.print("Size of Ground GPS message: ");Serial.println(sizeof(groundGPS));
    }
    else
    {
      Serial.println("Ground GPS not fixed");
    }
    Serial.println("--------------------------");
}

//void motorCommand()
//{
//  double lonDiff = (groundGPS.longitude - rocketGPS.longitude)*1110;
//  double latDiff = (rocketGPS.latitude - groundGPS.latitude)*887; //reduce digits
//  double azimuth = 180;
//
//  
//  double elevation = atan2 ((rocketGPS.altitude-groundGPS.altitude)*0.305, pow(pow(latDiff,2)+pow(lonDiff,2),0.5));
//  double distance = pow(pow(latDiff,2)+pow(lonDiff,2),0.5);
//  elevation *= 180/3.141593;
//
//  azimuth = atan2 (lonDiff, latDiff);
//  azimuth *= 180/3.14159;
//  azimuth += angleOffset;
//  if (latDiff == 0 && lonDiff > 0) //if rocket directly east
//  {
//    azimuth += (angleOffset + 90);
//  }
//  if (latDiff == 0 && lonDiff < 0) //if rocket directly west)
//  {
//    azimuth -= (angleOffset - 90);
//  }
//  if (azimuth < 0) //if centered south and rocket to west (normally would result in negative angle)
//  {
//    azimuth += 360;
//  }
//
//  //---command motor----
//  
//  long roundAzi = azimuth + 0.5;
//  int intAzi = (int)roundAzi; //round azimuth and convert to integer
//
//  String commandAzi;
//  commandAzi += intAzi;
//  int aziLength = commandAzi.length();
//  if (aziLength < 3) //if azimuth less than 3 digits, add zeros
//  {
//    commandAzi = "";
//    for (int a = 3 - aziLength; a > 0; a--)
//    {
//      commandAzi += "0";
//    }
//    commandAzi += intAzi;
//  }
//
//
//  long roundEle = elevation + 0.5;
//  int intEle = (int)roundEle;
//  
//  String commandEle;
//  if (intEle < 0)
//  {
//    commandEle += "000";
//  } else
//  {
//  commandEle += intEle;
//  }
//  
//  int eleLength = commandEle.length();
//  if (eleLength < 3) //if elevation less than 3 digits, add zeros
//  {
//    commandEle = "";
//    for (int e = 3 - eleLength; e > 0; e--)
//    {
//      commandEle += "0";
//    }
//    commandEle += intEle;
//  }
//
//  //---format is "Wxxx yyy", xxx = azimuth, yyy = elevation---
//  Serial3.print("W");
//  Serial3.print(commandAzi);
//  Serial3.print(" ");
//  Serial3.print(commandEle);
//  Serial3.write('\r');
//
//  //-------print information---------
//  Serial.println("----------Motor-----------");
//  Serial.print("offset: "); Serial.println(angleOffset);
//  if (angleOffset == 180)
//  {
//    Serial.print("degrees off south: "); Serial.println(azimuth);
//  }
//  if (angleOffset == 0)
//  {
//    Serial.print("degrees off north: "); Serial.println(azimuth);
//  }
//  Serial.print("distance: "); Serial.println(distance);
//  Serial.print("rising angle: "); Serial.println(elevation);
//  Serial.print("latDiff: "); Serial.print(latDiff); Serial.print(" lonDiff: "); Serial.println(lonDiff);
//  Serial.print("command: "); Serial.print("W"); Serial.print(commandAzi); Serial.print(" "); Serial.println(commandEle);
//  Serial.println("--------------------------");
//}


//void displayNMEA() 
//{      
//    String sentence = "GPRMC,";
//    
//    if (g_GPS.hour<10)
//      sentence += "0";
//    sentence += g_GPS.hour;
//
//    if (g_GPS.minute<10)
//      sentence += "0";
//    sentence += g_GPS.minute;
//
//    if (g_GPS.seconds<10)
//      sentence += "0";
//    sentence += g_GPS.seconds; 
//    
//    sentence += "."; sentence += g_GPS.milliseconds;sentence += ",";
//    sentence += (int)rocketGPS.latitude;
//    sentence += ".";
//    if ((int)((rocketGPS.latitude - (int)rocketGPS.latitude)*1e5)<1e5)
//      sentence += "0";
//    sentence += (int)((rocketGPS.latitude - (int)rocketGPS.latitude)*1e5);
//    sentence += ",";
//    
//    sentence += g_GPS.lat;sentence += ",";
//    sentence += (int)rocketGPS.longitude;
//    sentence += ".";
//    if ((int)((rocketGPS.longitude - (int)rocketGPS.longitude)*1e5)<1e5)
//      sentence += "0";
//    sentence += (int)((rocketGPS.longitude - (int)rocketGPS.longitude)*1e5);
//    sentence += ",";
//    
//    sentence += g_GPS.lon;sentence += ",";
//    sentence += rocketGPS.velocity;sentence += ",";
//    sentence += "100,";
//
//    if (g_GPS.day<10)
//      sentence += "0";
//    sentence += g_GPS.day;
//
//    if (g_GPS.month<10)
//      sentence += "0";
//    sentence += g_GPS.month;
//    
//    sentence += g_GPS.year;
//    sentence += ",";
//    
//    sentence += "0.0,";
//    sentence += "E,";
//    sentence += "A";
//    Serial.print("$");
//    Serial.print(sentence);
//    Serial.print("*");
//    Serial.print(checkSum(sentence),HEX);
//    Serial.print((char)13);
//    Serial.print((char)10);
//
////      Serial.println("$GPRMC,232852.0,3717.0754323,N,12146.8765908,W,0.02,100,170220,0.0,E,A*56");
//
////    if (g_GPS.hour<10)
////      Serial.print("0");
////    Serial.print(g_GPS.hour);
////
////    if (g_GPS.minute<10)
////      Serial.print("0");
////    Serial.print(g_GPS.minute);
////
////    if (g_GPS.seconds<10)
////      Serial.print("0");
////    Serial.print(g_GPS.seconds); 
////    
////    Serial.print("."); Serial.print(g_GPS.milliseconds);Serial.print(",");
////    Serial.print(rocketGPS.latitude,7);Serial.print(",");
////    Serial.print(g_GPS.lat);Serial.print(",");
////    Serial.print(rocketGPS.longitude,7);Serial.print(",");
////    Serial.print(g_GPS.lon);Serial.print(",");
////    Serial.print(rocketGPS.velocity);Serial.print(",");
////    Serial.print("100,");
////    Serial.print(g_GPS.day);Serial.print(g_GPS.month);Serial.print(g_GPS.year);Serial.print(",");
////    Serial.print("0.0,");
////    Serial.print("E,");
////    Serial.print("A");
////    Serial.print("*");
////    Serial.print((char)13);
////    Serial.print((char)10);
//
//}

char checkSum(String str)
{
  char check = 0;
  for (int i=0; i<str.length(); i++)
  {
    check = char (check ^ str.charAt(i));
  }
  return check;
}
