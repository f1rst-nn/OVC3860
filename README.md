# arduino library for OC3860 based modules

This directory contain all files needed to support A2DP bluetooth module based on OmniVision chip OVC3860.

OVC3860 have AT command control support, supported commands are descriped <a href="https://github.com/tomaskovacik/kicad-library/blob/master/library/datasheet/OVC3860_based_modules/BLK-MD-SPK-B_AT_Command_set_original.pdf">here</a> and <a href="https://github.com/tomaskovacik/kicad-library/blob/master/library/datasheet/OVC3860_based_modules/OVC3860_AT_Command_Application_Notes.pdf">here</a> (some did not work on my modules...).

# how to use it

<a href="https://www.arduino.cc/en/Guide/Libraries">Information about using libraries on arduino site</a>

Copy content of this repository directory or just this two files OVC3860.cpp and OVC3860.h to ~/Arduino/libraries/OVC3860/ directory (if did not exist, create one). Open new project in arduino and use this code, or check code in examples directory examples/OVC3860/OVC3860.ino:

```c
#include "OVC3860.h"
#include <SoftwareSerial.h> //if using SW, with HW no need for this

#define resetBTPin 5
 
SoftwareSerial swSerial(7, 6); //rxPin, txPin

OVC3860 BT(&swSerial, resetBTPin); //in case of HW serial use for example: (&Serial1)

void(){
  BT.begin(); //or BT.begin(9600); for specific baudrate
}

void loop(){
//should be call periodically, to get notifications from module, for example if someone calling...
BT.getNextEventFromBT();
}
```

for more examples look ate examples/OVC3860/OVC3860.ino

Do not forget: getNextEventFromBT function must be called periodicaly, it's run from other function in library to catch responce to commands send to module, but to catch mesages from module which are not expected (incomming call for example) this must be fired periodicaly.

# supported functions
```c
begin(uint32_t baudrate = 115200); //parameter baudrate is communication speed between MCU and arduino, default 115200

sendData(String cmd); //construct string of AT+cmd and send it to serial port

getNextEventFromBT(); //parse data send from module and send internal variales, call this periodicaly, to parse data received from module ASAP

sendRawData(uint8_t _size, uint8_t data[]); //send data[] to serial port

pairingInit(); //initialize pairing mode

pairingExit(); //exit pairing mode

connectHSHF(); //conect to handsfreedevice

disconnectHSHF(); //disconnect handsfree, but it leave A2DP connected

callAnswer();

callReject();

callHangUp();

callRedial();

voiceDialStart(); 

voiceDialEnd();

micToggle();

transferAudio();

callReleaseReject();

callReleaseAccept();

callHoldAccept();

callConference();

pairingDeleteThenInit();

callDialNumber(String number);

sendDTMF();

queryHFPStatus();

reset(); //software reset module

musicTogglePlayPause();

musicStop();

musicNextTrack();

musicPreviousTrack();

queryConfiguration(); //query audo connect and auto answare configuration

autoconnEnable(); //configure auto connect feature to be enabled 

autoconnDisable(); //configure auto connect feature to be disabled

connectA2DP(); //connect A2DP/ Media audio

disconnectA2DP(); //disconnect A2DP / media audio

changeLocalName(String name = ""); //without parameter module should return actual name, did not work for me

changePin(String pin = ""); //without parameter module should return actual pin, did not work for me

queryAvrcpStatus(); 

autoAnswerEnable(); //configure auto answer call feature to be enabled 

autoAnswerDisable(); //configure auto answer call feature to be disabled 

musicStartFF();

musicStartRWD();

musicStopFFRWD();

queryA2DPStatus(); 

writeToMemory(String data);

readFromMemory(String addr);

switchDevices();

queryVersion();

pbSyncBySim();

pbSyncByPhone();

pbReadNextItem();

pbReadPreviousItem();

pbSyncByDialer();

pbSyncByReceiver();

pbSyncByMissed();

pbSyncByLastCall();

getLocalLastDialedList();

getLocalLastReceivedList();

getLocalLastMissedList();

dialLastReceivedCall();

clearLocalCallHistory();

sppDataTransmit();

setClockdebugMode();

volumeDown();

enterTestMode(); //not usefull for normal user

setFixedFrequency(); //not usefull for normal user

emcTestMode(); //not usefull for normal user

setRFRegister(); //not usefull for normal user

inquiryStart(); 

inquiryStop(); 

volumeUp(); 

shutdown(); //shutdown module, need to be power cycled to start again, HW reset did not work

enterConfigMode(); //reset module (hw way) and enable pskey configuration mode 

quitConfigMode(); //disable pskey configuration mode

readName(); //read name of module in configuration mode

writeName(String NewName); //write name of module in configuration mode

readAllPSK(); //read all pskeys - did not work for me right now, also using pskey config app did not work for this.

readPin(); //read pin of module in configuration mode

writePin(String NewPin); //write pin of module in configuration mode
 
readBaudRate(); //read baudrate of module in configuration mode - did not work for me right now, also using pskey config app did not work for this. 

writeBaudRate(uint8_t NewBaudRate = OVC3860_BAUDRATE_115200); //write baudrate of module in configuration mode - did not work for me right now, also using pskey config app did not work for this.

resetModule();//reset module HW way = puting reset pin low and then high
