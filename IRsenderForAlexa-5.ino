/* 
 This sketch uses Ken Shirriff's *awesome* IRremote library:
   https://github.com/shirriff/Arduino-IRremote
 Updated by Mark Szabo to IRsend class to work on ESP8266 and 
 Sebastien Warin the receiving & decoding part (IRrecv class).
   
   This sketch uses the IR codes to transmit the commands for
   1. Samsung TV
   2. Marantz Amplifier
   3. Miele VaccumCleaner 
   
 Some IR codes are RC5 and some are RAW.
 
 Wifi part (Witty) ESP8266 will receive commands from Alexa and parse them to the IR remote part.
 My code will use some of the code in these files:
 http://www.makermusings.com/2015/07/18/virtual-wemo-code-for-amazon-echo/ and 
 https://github.com/kakopappa/arduino-esp8266-alexa-multiple-wemo-switch and IRremote8266 from
 https://github.com/markszabo/IRremoteESP8266

 1. You should ask "Alexa discover devices"
 2. She should find x amount of devices macthing your setup + the ones in the house
 3. Ask "Alexa, turn on Samsung T.V."
 4. Ask "Alexa, turn off Samsung T.V."
 5. Use the Alexa invocation name under define switches 
----------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
19/12-2016 Update      
Added Amplifier code
20/12-2016 Update      
Added Vaccum code
Added new name to project "IRsenderForAlexa.ino"
Serial TX will be replaced with other names called via Wifi,as this aurdino will serve as a webserver.
Or it will be communicating via a RF2401 module.
23/12-2016 Update
Tested Vacuum code. Works perfect.
Moved delay() from loop into bottom of send part.
31/12-2016 Update
New Wifi module code pasted. 
New name added: IRsenderForAlexa-2 
2/1-2017 Update
Added new Library. IRremoteESP8266.h  whiich can make IRremote codes with ESP8266
Added pin numbers
Added RemoteTV 
4/1-2017 Update
Codes is working. Samsung power on/off
added invocation name: Samsung TV,TV Channel,TV Volume,TV Channel one,Amplifier, Amplifier Volume aso....
5/1-2017 Update
New name added: IRsenderForAlexa-5 
 */


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <functional>
#include "switch.h"
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"
#include <IRremoteESP8266.h> // Include the IRremote library for ESP8266
IRsend irsend(0);            // Enables IR transmit LED  (pin 9 on Mega) an IR led is connected to GPIO pin 0

int Indicator_PIN = 15;       // Red indicator sender LED 
int Blue_LED      = 13;
int Green_LED     = 12;

// Call Connect to Wifi
boolean connectWifi();
//////////////////////////////////////////////////////////////////
// Declare on/off callbacks  
//////////////////////////////////////////////////////////////////
/* Declare Samsung TV callbacks  (on-off)  */
void SamsungTVOn();         //Samsung TV toggle ON/OFF
void SamsungTVOff();        //Samsung TV toggle ON/OFF
void SamsungMuteOn();       //Samsung TV Mute
void SamsungChannelUp();    //Samsung TV Channal up
void SamsungChannelDown();  //Samsung TV Channal Down
void SamsungVolumeUp();     //Samsung TV Volume up
void SamsungVolumeDown();   //Samsung TV Volume down
void SamsungCH1On();        //Samsung TV Ch 1
void SamsungCH2On();        //Samsung TV Ch 2
void SamsungCH3On();        //Samsung TV Ch 3
/* Declare Marantz callbacks  (on-off) */
void AmplifierOn();           //Amplifier ON
void AmplifierOff();          //Amplifier OFF
void AmplifierVolumeUp();     //Amplifier volume up
void AmplifierVolumeDown();   //Amplifier volume down
void AmplifierAuxOn();        //Amplifier Aux input
void AmplifierTunerOn();      //Amplifier Tuner input
void AmplifierTvOn();         //Amplifier TV input
void AmplifierDVDOn();        //Amplifier DVD input
void AmplifierScourceUp();    //Amplifier Source up
void AmplifierScourceDown();  //Amplifier Source down
void AmplifierMuteOn();       //Amplifier Mute toggle
void AmplifierMuteOff();      //Amplifier Mute toggle
/* Declare Mile callbacks  (on-off) */
void VacuumCleanerOn();       //Vaccum Cleaner toggle ON
void VacuumCleanerCornerOn(); //Corners ON
void VacuumCleanerGoOn();     //Go Go Go
void VacuumCleanerBaseOn();   //Return to home


// Change this before you flash
const char* ssid = "Your Wifi Name"; 
const char* password = "Your Wifi Code";
boolean wifiConnected = false;
UpnpBroadcastResponder upnpBroadcastResponder;

//Define switches for TV
Switch *SamsungTV      = NULL;        
Switch *SamsungMute    = NULL;      
Switch *SamsungChannel = NULL;   
Switch *SamsungVolume  = NULL;   
Switch *SamsungCH1     = NULL;  
Switch *SamsungCH2     = NULL;  
Switch *SamsungCH3     = NULL;  

//Define switches for Amplifier
Switch *Amplifier        = NULL;        
Switch *AmplifierVolume  = NULL;      
Switch *AmplifierAux     = NULL;   
Switch *AmplifierTuner   = NULL;   
Switch *AmplifierTv      = NULL;  
Switch *AmplifierDVD     = NULL;  
Switch *AmplifierScource = NULL;  
Switch *AmplifierMute    = NULL;   

//Define switches for Miele Hoover
Switch *VacuumCleaner       = NULL;        
Switch *VacuumCleanerCorner = NULL;      
Switch *VacuumCleanerGo     = NULL;   
Switch *VacuumCleanerBase   = NULL;   
//Define dimmers for light
//Dimmer *KitchenLight       = NULL;        
//Dimmer *OfficeLight        = NULL;

/*  
LDR = A0;
D0  GPIO 16
D1  GPIO 5  
D2  GPIO 4  Pushbutton
D3  GPIO 0  To IR transmitter
D4  GPIO 2
D5  GPIO 14
D6  GPIO 12 Green LED
D7  GPIO 13 Blue LED
D8  GPIO 15 Red LED
*/

void setup()
{
  Serial.begin(115200);            // Initial for ESP8266
   
  // Initialise wifi connection
  wifiConnected = connectWifi();
  
  if(wifiConnected){
    upnpBroadcastResponder.beginUdpMulticast();
    
    /* Define your switches here. Max 14
    * Format: Alexa invocation name, local port no, on callback, off callback */
    /* TV switches */
    SamsungTV      = new Switch("Samsung T.V.", 80, SamsungTVOn, SamsungTVOn);                    //Samsung on/off
    SamsungMute    = new Switch("Samsung T.V Mute", 81, SamsungMuteOn, SamsungMuteOn);           //Samsung Mute toggle
    SamsungChannel = new Switch("Samsung T.V. Channel", 82, SamsungChannelUp, SamsungChannelDown);//Samsung Channel up and down
    SamsungVolume  = new Switch("Samsung T.V. Volume", 83, SamsungVolumeUp, SamsungVolumeDown);   //Samsung volume up and down
    SamsungCH1     = new Switch("TV Channel one", 84, SamsungCH1On, SamsungCH1On);                //Samsung Channel 1  on
    SamsungCH2     = new Switch("TV Channel two", 85, SamsungCH2On, SamsungCH2On);                //Samsung Channel 2  on
    SamsungCH3     = new Switch("TV Channel tree", 86, SamsungCH3On, SamsungCH3On);               //Samsung Channel 3  on
    /* Marantz switches */
    Amplifier        = new Switch("Amplifier", 87, AmplifierOn, AmplifierOff);                     //Amplifier on/off
    AmplifierVolume  = new Switch("Amplifier volume", 88, AmplifierVolumeUp, AmplifierVolumeDown); //Amplifier volume up/down
    AmplifierAux     = new Switch("Amplifier aux", 89, AmplifierAuxOn, AmplifierAuxOn);            //Amplifier Aux on
    AmplifierTuner   = new Switch("Amplifier tuner", 90, AmplifierTunerOn, AmplifierTunerOn);      //Amplifier Tuner on
    AmplifierTv      = new Switch("Amplifier T.V.", 91, AmplifierTvOn, AmplifierTvOn);             //Amplifier TV on
    AmplifierDVD     = new Switch("Amplifier d.v.d.", 92, AmplifierDVDOn, AmplifierDVDOn);         //Amplifier DVD on
    AmplifierScource = new Switch("Amplifier", 93, AmplifierScourceUp, AmplifierScourceDown);      //Amplifier Scounce up/down
    AmplifierMute    = new Switch("Amplifier mute", 94, AmplifierMuteOn, AmplifierMuteOff);        //Amplifier Mute on/off
    /* Mile switches */
    VacuumCleaner       = new Switch("VacuumCleaner", 95, VacuumCleanerOn, VacuumCleanerOn);       //Amplifier Mute toggle on/off
    VacuumCleanerCorner = new Switch("VacuumCleanerCorner", 96, VacuumCleanerCornerOn, VacuumCleanerCornerOn);
    VacuumCleanerGo     = new Switch("VacuumCleanerGo", 97, VacuumCleanerGoOn, VacuumCleanerGoOn);
    VacuumCleanerBase   = new Switch("VacuumCleanerBase", 98, VacuumCleanerBaseOn, VacuumCleanerBaseOn);


    Serial.println("Adding switches upnp broadcast responder");
    
    upnpBroadcastResponder.addDevice(*SamsungTV);
    upnpBroadcastResponder.addDevice(*SamsungMute);
    upnpBroadcastResponder.addDevice(*SamsungChannel);
    upnpBroadcastResponder.addDevice(*SamsungVolume);
    upnpBroadcastResponder.addDevice(*SamsungCH1);
    upnpBroadcastResponder.addDevice(*SamsungCH2);
    upnpBroadcastResponder.addDevice(*SamsungCH3);
    
    upnpBroadcastResponder.addDevice(*Amplifier);
    upnpBroadcastResponder.addDevice(*AmplifierVolume);
    upnpBroadcastResponder.addDevice(*AmplifierAux);
    upnpBroadcastResponder.addDevice(*AmplifierTuner);
    upnpBroadcastResponder.addDevice(*AmplifierTv);
    upnpBroadcastResponder.addDevice(*AmplifierDVD);
    upnpBroadcastResponder.addDevice(*AmplifierScource);
    upnpBroadcastResponder.addDevice(*AmplifierMute);

    upnpBroadcastResponder.addDevice(*VacuumCleaner);
    upnpBroadcastResponder.addDevice(*VacuumCleanerCorner);
    upnpBroadcastResponder.addDevice(*VacuumCleanerGo);
    upnpBroadcastResponder.addDevice(*VacuumCleanerBase);
    
    //Set LED pins to outputs
       pinMode(12,OUTPUT);  //Green
       pinMode(13,OUTPUT);  //Blue
       pinMode(14,OUTPUT);  //Not in use 
       pinMode(15,OUTPUT);  //Red  sender indicator
       pinMode(0,OUTPUT);   // IR diode transmitter

       digitalWrite(0, LOW);//Init for PIN0
       digitalWrite(Green_LED, HIGH);  // Setup was OK
  }
}
 
void loop()
{
 if(wifiConnected){
      upnpBroadcastResponder.serverLoop();
      
      SamsungTV->serverLoop();
      SamsungMute->serverLoop();
      SamsungChannel->serverLoop();
      SamsungVolume->serverLoop();
      SamsungCH1->serverLoop();
      SamsungCH2->serverLoop();
      SamsungCH3->serverLoop();

      Amplifier->serverLoop();
      AmplifierVolume->serverLoop();
      AmplifierAux->serverLoop();
      SamsungVolume->serverLoop();
      AmplifierTuner->serverLoop();
      AmplifierTv->serverLoop();
      AmplifierDVD->serverLoop();
      AmplifierScource->serverLoop();
      AmplifierMute->serverLoop();

      VacuumCleaner->serverLoop();
      VacuumCleanerCorner->serverLoop();
      VacuumCleanerGo->serverLoop();
      VacuumCleanerBase->serverLoop();
	 }
}

//////////////////////////////////////////////////////////////////
// Here comes the IR part
// Input from the Callback, and case sorting by the input number.
//////////////////////////////////////////////////////////////////
void SerialCase(int ValueforIRcodes)
{
//SAMSUNG CODES
#define POWER         0xE0E040BF       
#define Mute          0xE0E0F00F 
#define CH_UP         0xE0E048B7     
#define CH_DOWN       0xE0E008F7     
#define VOL_UP        0xE0E0E01F     
#define VOL_DOWN      0xE0E0D02F     
#define CH0           0xE0E08877
#define CH1           0xE0E020DF     
#define CH2           0xE0E0A05F
#define CH3           0xE0E0609F
#define CH4           0xE0E010EF
#define CH5           0xE0E0906F
#define CH6           0xE0E050AF
#define CH7           0xE0E030CF
#define CH8           0xE0E0B04F
#define CH9           0xE0E0708F
#define SAMSUNG_BITS  32 

//MARANTS CODES
#define POWER_ON      0xC0C
#define POWER_OFF     0x40C
#define VOL_UP        0x410
#define VOL_DOWN      0x411
//#define AUX         RAW
//#define TUNER       RAW
#define TV            0x17F 
//#define DVD         RAW
//#define SOURCE_UP   RAW
//#define SOURCE_DOWN RAW
#define MUTE_ON       0xC0D
#define MUTE_OFF      0x40D

// MIELE CODES
#define VacuumPOWER_ON      0x807FE01F
#define VacuumCorner        0x807F708F
#define VacuumGO            0x807F20DF
#define VacuumBase          0x807F609F
#define MIELE_BITS  32 

    
    digitalWrite(Green_LED, LOW);  // Turn off the green LED
    digitalWrite(Indicator_PIN, HIGH);
  // Heres all the cases with the RemoteCodes
  
  switch (ValueforIRcodes) 
  {
      case '1':
        {
          Serial.println("TV ON/OFF, sending");
          irsend.sendSAMSUNG(POWER, SAMSUNG_BITS); 
        }
        break;
      case '2': 
        {
          Serial.println("TV Mute, sending");
          irsend.sendSAMSUNG(Mute, SAMSUNG_BITS); 
        }
        break;
      case '3':
        {
          Serial.println("TV CH_UP, sending");
          irsend.sendSAMSUNG(CH_UP, SAMSUNG_BITS); 
        }
        break;
      case '4':
        {
          Serial.println("TV CH_DOWN, sending");
          irsend.sendSAMSUNG(CH_DOWN, SAMSUNG_BITS); 
        }
        break;
      case '5':
        {
          Serial.println("TV VOL_UP, sending");
          irsend.sendSAMSUNG(VOL_UP, SAMSUNG_BITS); 
        }
        break;
      case '6':
        {
          Serial.println("TV VOL_DOWN, sending");
          irsend.sendSAMSUNG(VOL_DOWN, SAMSUNG_BITS); 
        }
        break;
      case '7':
        {
          Serial.println("TV CH1, sending");
          irsend.sendSAMSUNG(CH1, SAMSUNG_BITS); 
        }
        break;
      case '8':
        {
          Serial.println("TV CH2, sending");
          irsend.sendSAMSUNG(CH2, SAMSUNG_BITS); 
        }
        break;
      case '9':
        {
          Serial.println("TV CH3, sending");
          irsend.sendSAMSUNG(CH3, SAMSUNG_BITS); 
        }
        break;
      case '0':
        {
          Serial.println("TV CH4, sending");
          irsend.sendSAMSUNG(CH4, SAMSUNG_BITS); 
        }
        break;
      case '!':
         {
          Serial.println("TV CH5, sending");
          irsend.sendSAMSUNG(CH5, SAMSUNG_BITS); 
         }
        break;
      case '#':
        {
          Serial.println("TV CH6, sending");
          irsend.sendSAMSUNG(CH6, SAMSUNG_BITS); 
        }
        break;
      case '¤': 
        {
          Serial.println("TV CH7, sending");
          irsend.sendSAMSUNG(CH7, SAMSUNG_BITS); 
        }
        break;
      case '%': 
        {
          Serial.println("TV CH8, sending");
          irsend.sendSAMSUNG(CH8, SAMSUNG_BITS); 
        }
        break;
      case '&': 
        {
          Serial.println("TV CH9, sending");
          irsend.sendSAMSUNG(CH9, SAMSUNG_BITS); 
        }
        break;
      case '/': 
        {
          Serial.println("TV CH10, sending");
          irsend.sendSAMSUNG(CH1, SAMSUNG_BITS); 
          delay(50); 
          irsend.sendSAMSUNG(CH0, SAMSUNG_BITS); 
        }
        break;
        
      case 'a':
        {//Marantz On
          Serial.println("Amp ON, sending");
          irsend.sendRC5(POWER_ON, 12);
          delay(50);   
          irsend.sendRC5(POWER_ON, 12);
        } 
        break;
      case 'b':
        { //Marantz Off
          Serial.println("Amp OFF, sending");
          irsend.sendRC5(POWER_OFF, 12);
        }
        break;
      case 'c':
        { //Marantz VOL_UP
          Serial.println("Amp VOL+, sending");
          irsend.sendRC5(VOL_UP, 12);
        }
        break;
      case 'd':
        { //Marantz VOL_DOWN
          Serial.println("Amp Vol-, sending");
          irsend.sendRC5(VOL_DOWN, 12);
        }
        break;
      case 'e':
        {// Marantz Aux2  [unsigned int = kun positive tal]
          unsigned int RawIrData[] ={900,850, 900,850, 850,900, 1750,850, 900,850, 900,850, 900,4350, 900,850, 850,850, 900,900, 850,900, 850,900, 850,900, 850,900, 850,850, 900,1750, 850,850, 900,850, 900};
          Serial.println("Amp Aux2, sending");
          irsend.sendRaw(RawIrData, sizeof(RawIrData)/sizeof(int), 38);
          delay(50);
          irsend.sendRaw(RawIrData, sizeof(RawIrData)/sizeof(int), 38);
        }
        break;
      case 'f':
        {// Marantz Tuner
          unsigned int RawIrData[] ={900,850, 900,850, 900,850, 1750,850, 900,850, 900,1700, 900,850, 900,850, 900,850, 900,850, 900,850, 900,850, 900};
          Serial.println("Amp Tuner, sending");
          irsend.sendRaw(RawIrData, sizeof(RawIrData)/sizeof(int), 38);
          delay(50);
          irsend.sendRaw(RawIrData, sizeof(RawIrData)/sizeof(int), 38);
        }
        break;
      case 'g':
        {// Marantz TV scource 
          Serial.println("Amp TV, sending");
          irsend.sendRC5(TV, 12);
        }  
        break;
      case 'h':
        {// Marantz DVD scource 
          unsigned int RawIrData[] ={800, 950, 800, 950, 800, 1000, 1650, 950, 800, 950, 750, 1000, 750, 4450, 800, 950, 800, 950, 800, 1000, 750, 950, 800, 950, 800, 950, 800, 950, 800, 1800, 1650, 1850, 1650};
          Serial.println("Amp DVD, sending");
          irsend.sendRaw(RawIrData, sizeof(RawIrData)/sizeof(int), 38);
          delay(50);
          irsend.sendRaw(RawIrData, sizeof(RawIrData)/sizeof(int), 38);
        }  
        break;
      case 'i':                    
        { //Marantz SCOURCE_UP     
          unsigned int RawIrData[] ={900,850, 900,850, 900,850, 1750,850, 900,850, 900,850, 900,4350, 900,850, 900,850, 900,850, 850,850, 900,850, 900,850, 900,850, 900,1750, 850,850, 1800,1700, 900};
          Serial.println("Amp SCOURCE+, sending");
          irsend.sendRaw(RawIrData, sizeof(RawIrData)/sizeof(int), 38);
        }
        break;
      case 'j':
        { //Marantz SCOURCE_DOWN   
          unsigned int RawIrData[] = {900,850, 900,850, 900,850, 1750,850, 900,850, 900,850, 900,4350, 900,850, 900,850, 900,850, 900,850, 900,850, 900,850, 900,850, 900,1700, 900,850, 900,850, 1750}; 
          Serial.println("Amp SCOURCE-, sending");
          irsend.sendRaw(RawIrData, sizeof(RawIrData)/sizeof(int), 38);
        }
        break;      
      case 'k':
        { //Marantz MUTE_ON /Toggle 
          Serial.println("Amp Mute_On, sending");
          irsend.sendRC5(MUTE_ON, 12);
        }
        break;
      case 'l':
        { //Marantz MUTE_OFF
          Serial.println("Amp Mute_Off, sending");
          irsend.sendRC5(MUTE_OFF, 12);
        }
        break;
      case 'o':
        {
          Serial.println("VacuumCleaner ON/OFF, sending");
          irsend.sendNEC(VacuumPOWER_ON, MIELE_BITS); 
        }
        break;
      case 'p':
        {
          Serial.println("VacuumCleaner Corner, sending");
          irsend.sendNEC(VacuumCorner, MIELE_BITS); 
        }
        break;
      case 'q':
        {
          Serial.println("VacuumCleaner Go, sending");
          irsend.sendNEC(VacuumGO, MIELE_BITS); 
        }
        break;
      case 'r':
        {
          Serial.println("VacuumCleaner Base, sending");
          irsend.sendNEC(VacuumBase, MIELE_BITS); 
        }
        break;

      default:
         Serial.println("Default");

  } //End case
digitalWrite(Indicator_PIN, LOW);
ValueforIRcodes = 'z'; //Resetter værdien.
delay(200);  //LED will light a bit longer
}

//////////////////////////////////////////////////////////////////
// Here comes the Dimmer part
// Input from the Callback, will redirect to this function
//////////////////////////////////////////////////////////////////
void DimmerSet(int ValueforDimmer)
{
// Code will be added
//          Serial.println("Dimmer 1 Down, sending");
//          irsend.sendSAMSUNG(DIM_DOWN, DIM_BITS); 
}

//////////////////////////////////////////////////////////////////
// Here comes all the code for Alexa to do 
// the requered instructions.
// Jump to serialCase with parameter. This will send the IR code
//////////////////////////////////////////////////////////////////
/* Samsung TV */
void SamsungTVOn()         { SerialCase(1);   };  //Samsung TV on/off
void SamsungMuteOn()       { SerialCase(2);   };  //Samsung TV Mute
void SamsungChannelUp()    { SerialCase(3);   };    //Samsung TV Channal up
void SamsungChannelDown()  { SerialCase(4);   };    //Samsung TV Channal Down
void SamsungVolumeUp()     { SerialCase(5);   };    //Samsung TV Volume up
void SamsungVolumeDown()   { SerialCase(6);   };    //Samsung TV Volume down
void SamsungCH1On()        { SerialCase(7);   };  //Samsung TV Ch 1
void SamsungCH2On()        { SerialCase(8);   };  //Samsung TV Ch 2
void SamsungCH3On()        { SerialCase(9);   };  //Samsung TV Ch 3

/* Marantz  */
void AmplifierOn()         { SerialCase('a');   }; //Amplifier ON
void AmplifierOff()        { SerialCase('b');   }; //Amplifier OFF
void AmplifierVolumeUp()   { SerialCase('c');   }; //Amplifier volume up
void AmplifierVolumeDown() { SerialCase('d');   }; //Amplifier volume down
void AmplifierAuxOn()      { SerialCase('e');   }; //Amplifier Aux input
void AmplifierTunerOn()    { SerialCase('f');   }; //Amplifier Tuner input
void AmplifierTvOn()       { SerialCase('g');   }; //Amplifier TV input
void AmplifierDVDOn()      { SerialCase('h');   }; //Amplifier DVD input
void AmplifierScourceUp()  { SerialCase('i');   }; //Amplifier Source up
void AmplifierScourceDown(){ SerialCase('j');   }; //Amplifier Source up
void AmplifierMuteOn()     { SerialCase('k');   }; //Amplifier Mute toggle
void AmplifierMuteOff()    { SerialCase('l');   }; //Amplifier Mute toggle

/* Mile */
void VacuumCleanerOn()      { SerialCase('o');   }; //Vaccum Cleaner toggle ON/OFF
void VacuumCleanerCornerOn(){ SerialCase('p');   }; //Corners ON
void VacuumCleanerGoOn()    { SerialCase('q');   }; //Go Go Go
void VacuumCleanerBaseOn()  { SerialCase('r');   }; //Return to home



//////////////////////////////////////////////////////////////////
// Heres comes the Wifi connect part
// connect to wifi – returns true if successful or false if not
// If connect is True, the green LED will not turn off
//////////////////////////////////////////////////////////////////
boolean connectWifi(){
  boolean state = true;
  int i = 0;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");
    
  // Wait for connection
  Serial.print("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }
  
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    long rssi = WiFi.RSSI();
    Serial.print("RSSI:");
    Serial.println(rssi);
    
  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
    digitalWrite(Green_LED, LOW);  // Hmm no connect ...pls repeat the start sequence again
  }
  
  return state;
} // End boolean connectWifi
