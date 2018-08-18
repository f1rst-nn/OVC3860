/*
        GNU GPL v3
        (C) Tomas Kovacik [nail at nodomain dot sk]
        https://github.com/tomaskovacik/

	library for BT modules based on OVC3860 chip by omnivision

  based on 2 pdfs:
  https://github.com/tomaskovacik/kicad-library/blob/master/library/datasheet/OVC3860_based_modules/BLK-MD-SPK-B_AT_Command_set_original.pdf
  https://github.com/tomaskovacik/kicad-library/blob/master/library/datasheet/OVC3860_based_modules/OVC3860_AT_Command_Application_Notes.pdf

  some commands did not work for me (P*,MM,MN...)
  (file BLK-MD-SPK-B_AT_Command_set_original.pdf is marked in header as -B but description is mentioning -A version of module, maybe
  this is why some commands did not work)

*/

#include "OVC3860.h"
#include <Arduino.h>

#if defined(USE_SW_SERIAL)
#include <SoftwareSerial.h>
#endif



#if defined(USE_SW_SERIAL)
#if ARDUINO >= 100
OVC3860::OVC3860(SoftwareSerial *ser, uint8_t resetPin)
#else
OVC3860::OVC3860(NewSoftSerial *ser, uint8_t resetPin)
#endif
#else
OVC3860::OVC3860(HardwareSerial *ser, uint8_t resetPin) {
#endif
{
  btSerial = ser;
  _reset = resetPin;
}

/*
   Destructor
*/
OVC3860::~OVC3860() {
  btSerial->end();
}

void OVC3860::begin(uint32_t baudrate) {
  btSerial->begin(baudrate);
  pinMode(_reset, OUTPUT);
  OVC3860::resetHigh();
}

void OVC3860::resetLow() {
  digitalWrite(_reset, LOW);
}

void OVC3860::resetHigh() {
  digitalWrite(_reset, HIGH);
}

void OVC3860::ResetModule() {
  DBG("reseting module\n");
  resetLow();
  delay(100);
  resetHigh();
}

/*
   debug output
*/
void OVC3860::DBG(String text) {
  if (DEBUG) {
    if (!Serial.availableForWrite()) delay(100);
    Serial.print(text);;
  }
}

uint8_t OVC3860::decodeReceivedString(String receivedString) {
  DBG(receivedString);
  DBG("\n");
  switch (receivedString[0]) {
    case 'A':
      {
        switch (receivedString[1]) {
          case 'A':
            {
              switch (receivedString[2]) {
                case '1':
                  ASR = _44800;
                  break;
                case '2':
                  ASR = _44100;
                  break;
                case '4':
                  ASR = _32000;
                  break;
                case '8':
                  ASR = _16000;
                  break;
              }
            }
            break;
          case 'E':
            Audio = ConfirError;
            break;
          case 'F':
            Audio = CodecClosed;
            break;
          case 'S':
            Audio = PhoneCall;
            break;
        }
      }
      break;
    case 'C':
      {
        //        PowerState = On;
        //        switch (receivedString[1]) {
        //          case '1':
        //            BTState = Connected;
        //            break;
        //          case '0':
        //            BTState = Disconnected;
        //            break;
        //        }
      }
      break;
    case 'E':
      {
        //        PowerState = On;
        //        switch (receivedString[1]) {
        //          case 'R':
        //            if (receivedString[2] == 'R') return 0;
        //            break;
        //        }
      }
      break;
    case 'I':// connection info
      {
        PowerState = On;
        switch (receivedString[1]) {
          case 'I': //HSHF enters pairing state, discoverable for 2 minutes
            BTState = Discoverable;
            break;
          case 'J':
            if (receivedString[1] == '2')
              BTState = Listening; //HSHF exits pairing mode and enters listening
            break;
          case 'V':
            HFPState = Connected;
            break;
          case 'A':
            HFPState = Listening;
            break;
          case 'F': //Call-setup status is idle
            PhoneState = PhoneHangUp;//??
            break;
          case 'H':
            //??
            break;
        }
      }
      break;
    /*
      from datasheet:


    */
    case 'M':
      {
        PowerState = On;
        switch (receivedString[1]) {
          case 'A':
            MusicState = Idle;
            break;
          case 'B':
            MusicState = Playing;
            break;
          case 'E':
            //          if (receivedString[2] == 'M' && receivedString[3] == ':')
            //            return receivedString;
            break;
          case 'F':
            {
              switch (receivedString[2]) {
                case '0':
                  AutoAnswer = Off;
                  break;
                case '1':
                  AutoAnswer = On;
                  break;
              }
              switch (receivedString[3]) {
                case '0':
                  AutoConnect = Off;
                  break;
                case '1':
                  AutoConnect = On;
                  break;
              }
            }
            break;
          /*
            HFP Status Value Description:(MG) => uint8_t HFPState
            1. Ready (to be connected)
            2. Connecting
            3. Connected
            4. Outgoing Call
            5. Incoming Call
            6. Ongoing Call
          */
          case 'G':
            {
              switch (receivedString[2]) {
                case '1':
                  HFPState = Ready;
                  break;
                case '2':
                  HFPState = Connecting;
                  break;
                case '3':
                  HFPState = Connected;
                  break;
                case '4':
                  HFPState = OutgoingCall;
                  break;
                case '5':
                  HFPState = IncomingCall;
                  break;
                case '6':
                  HFPState = OngoingCall;
                  break;
              }
            }
            break;
          /*
            AVRCP Status Value Description:(ML) => uint8_t AVRCPState
            1. Ready (to be connected)
            2. Connecting
            3. Connected
          */
          case 'L':
            break;
          case 'P':
            MusicState = Idle;
            break;
          case 'R':
            MusicState = Playing;
            break;
          case 'S':
            MusicState = Rewinding;
            break;
          /*
            A2DP Status Value Description:(MU) => uint8_t A2DPState
            1. Ready (to be connected)
            2. Initializing
            3. Signalling Active
            4. Connected
            5. Streaming
          */
          case 'U':
            break;
          case 'X':
            MusicState = FastForwarding;
            break;
          case '0':
            BTState = Disconnected;
            break;
          case '1':
            BTState = Connected;
            break;
          case '2':
            CallState = IncomingCall;
            break;
          case '3':
            CallState = OutgoingCall;
            break;
          case '4':
            CallState = OngoingCall;
            break;
        }
      }
      break;
    case 'N':
      {
        //        PowerState = On;
        //        if (receivedString[1] == 'A' && receivedString[2] == ': ') {//name
        //          // BT_NAME = OVC3860::returnBtModuleName(receivedString);
        //        }
      }
      break;
    case 'P':
      {
        PowerState = On;
        switch (receivedString[1]) {
          case 'A':
            {
              switch (receivedString[2]) {
                case '0':
                  break;
                case '1':
                  break;

              }
            }
            break;
          case 'C':
            break;
        }
      }
      break;
    case 'O': //BT On
      {
        //        switch (receivedString[1]) {
        //          case 'N':
        //            PowerState = On;
        //            break;
        //          case 'K':
        //            if (PowerState == ShutdownInProgress) PowerState = Off;
        //            return 1;
        //            break;
        //        }
      }
      break;
    case 0xA: //\r
      OVC3860::decodeReceivedString(receivedString.substring(1));
      break;
    case 0x20: //space
      OVC3860::decodeReceivedString(receivedString.substring(1));
      break;
    case 'S':
      switch (receivedString[1]) {
        case 'C':
          BTState = SPPopened;
          DBG("SPP opened");
          break;
        case 'D':
          BTState = SPPclosed;
          DBG("SPP closed");
          break;
      }
      break;
    default :
      DBG("received :" + (char)receivedString[0]);
  }
  return 1;
}

uint8_t OVC3860::sendRawData(uint8_t _size, uint8_t _data[]) {

  for (uint8_t i = 0; i < _size; i++ ) {
    btSerial -> write(_data[i]);
  }

  DBG("sending raw data: ");
  for (uint8_t i = 0; i < _size; i++ ) {
    DBG(String(_data[i], HEX));
  }
  DBG("\n");

}

uint8_t OVC3860::QuitConfigMode() { //responce: 0x60,0x00,0x00,0x00
  if (BTState != ConfigMode) {
    return false;
  } else {
    DBG("Quiting config mode\n");
    // OVC3860::ResetModule();
    uint8_t Data[4] = {0x50, 0x00, 0x00, 0x00};
    sendRawData(4, Data);
  }
  BTState = Disconnected;
}

uint8_t OVC3860::EnterConfigMode() {

  uint8_t initConfigData[9] = {0xC5, 0xC7, 0xC7, 0xC9, 0xD0, 0xD7, 0xC9, 0xD1, 0xCD};

  OVC3860::ResetModule();

  while (!btSerial -> available()) {}

  if (btSerial -> available()) {
    if (btSerial -> read() == 0x04 && btSerial -> read() == 0x0F && btSerial -> read() == 0x04 && btSerial -> read() == 0x00 && btSerial -> read() == 0x01 && btSerial -> read() == 0x00 && btSerial -> read() == 0x00) {
      sendRawData(9, initConfigData);
    }
  }
  else
    return false;

  while (!btSerial -> available()) {}

  if (btSerial -> available()) {
    if (btSerial -> read() == 0x04 && btSerial -> read() == 0x0F && btSerial -> read() == 0x04 && btSerial -> read() == 0x01 && btSerial -> read() == 0x01 && btSerial -> read() == 0x00 && btSerial -> read() == 0x00) {
      //DBG("Config mode");
      BTState = ConfigMode;
    }
    else
      return false;

  }
}

uint8_t OVC3860::ReadName() {
  if (BTState != ConfigMode) {
    return false;
  } else {
    DBG("Reading name\n");
    uint8_t Data[4] = {0x11, 0xc7, 0x00, 0x10};
    sendRawData(4, Data);
  }
  OVC3860::getNextEventFromBT();
}

uint8_t OVC3860::WriteName(String NewName) { //resposce: 0x41,0xc7,0x00,0x10
  if (BTState != ConfigMode) {
    return false;
  } else {
    if (NewName.length() - 2 > 16) { //count for termination char
      DBG("name to long, max 16chars");
      return false;
    } else {
      DBG("Writing name\n");
      uint8_t Data[20] = {0x31, 0xc7, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
      for (uint8_t i = 0; i < NewName.length() - 2; i++) {
        Data[i + 4] = NewName[i];
      }
      sendRawData(20, Data);
    }
  }
  OVC3860::getNextEventFromBT();
}

uint8_t OVC3860::ReadAllPSK() {
  if (BTState != ConfigMode) {
    return false;
  } else {
    DBG("Reading All PSK\n");
    uint8_t Data[4] = {0x10, 0x00, 0x03, 0x3D};
    //uint8_t Data[4] = {0x10, 0x00, 0x00, 0x00};
    sendRawData(4, Data);
  }
  OVC3860::getNextEventFromBT();
}

uint8_t OVC3860::ReadPin() {
  if (BTState != ConfigMode) {
    return false;
  } else {
    DBG("Reading Pin\n");
    uint8_t Data[4] = {0x11, 0xBF, 0x00, 0x08};
    sendRawData(4, Data);
  }
  OVC3860::getNextEventFromBT();
}

uint8_t OVC3860::WritePin(String NewPin) { //resposce: 0x41,0xc7,0x00,0x10
  if (BTState != ConfigMode) {
    return false;
  } else {
    if (NewPin.length() - 2 > 8) { //count for termination char
      DBG("name to long, max 8 chars");
      return false;
    } else {
      DBG("Writing name\n");
      uint8_t Data[12] = {0x31, 0xBF, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
      for (uint8_t i = 0; i < NewPin.length() - 2; i++) {
        Data[i + 4] = NewPin[i];
      }
      sendRawData(12, Data);
    }
  }
  OVC3860::getNextEventFromBT();
}

uint8_t OVC3860::ReadBaudRate() {
  if (BTState != ConfigMode) {
    return false;
  } else {
    DBG("Reading Pin\n");
    uint8_t Data[4] = {0x11, 0x11, 0x00, 0x01};
    sendRawData(4, Data);
  }
  OVC3860::getNextEventFromBT();
}

uint8_t OVC3860::WriteBaudRate(uint8_t NewBaudRate) { //resposce: 0x41,0xc7,0x00,0x10
  if (BTState != ConfigMode) {
    return false;
  } else {
    DBG("Writing name\n");
    uint8_t Data[5] = {0x31, 0x11, 0x00, 0x01, NewBaudRate};
    sendRawData(5, Data);
  }
  OVC3860::getNextEventFromBT();
}

/*String OVC3860::returnCallerID(String receivedString) {
  DBG("Calling: " + receivedString.substring(4,(receivedString.length() - 2)) + "\n");
  return receivedString.substring(4,(receivedString.length() - 2)); //start at 4 cose: IR-"+123456789" or PR-"+123456789" and one before end to remove " and \0
  }
*/
String OVC3860::returnBtModuleName(String receivedString) {
  DBG("Bluetooth module name: " + receivedString.substring(4) + "\n");
  return receivedString.substring(4);
}

uint8_t OVC3860::decodeReceivedDataArray(uint8_t data[]) {
  uint16_t packetSize = ((data[2] << 8) | (data[3] & 0xff)); //+ (start,address, packetsize1 and packetsize2)
  packetSize += 4;

  for (uint16_t i = 0; i < packetSize; i++) {
    Serial.print(data[i], HEX); Serial.print(" "); Serial.write(data[i]); Serial.println();
  }
}

uint8_t OVC3860::getNextEventFromBT() {

  delay(100);

  if (BTState == ConfigMode) {

    if (btSerial -> available()) {

      uint8_t startByte = btSerial -> read();

      switch (startByte) {
        case  0x21:
          {
            uint8_t addressByte = btSerial -> read();
            uint8_t packetSize1 = btSerial -> read();
            uint8_t packetSize2 = btSerial -> read();

            uint16_t packetSize = ((packetSize1 << 8) | (packetSize2 & 0xff)); //+ (start,address, packetsize1 and packetsize2)
            packetSize += 4;

            uint8_t data[packetSize];

            data[0] = startByte;
            data[1] = addressByte;
            data[2] = packetSize1;
            data[3] = packetSize2;

            for (uint16_t i = 4; i < packetSize; i++) {
              data[i] = btSerial -> read();
            }

            DBG("received raw data: ");

            if (DEBUG) {
              for (uint16_t i = 0; i < packetSize; i++) {
                DBG(String(data[i], HEX));
              }
            }

            OVC3860::decodeReceivedDataArray(data);
          }
          break;
        case 0x20:
          {
            uint8_t addressByte = btSerial -> read();
            uint8_t packetSize1 = btSerial -> read();
            uint8_t packetSize2 = btSerial -> read();

            uint16_t packetSize = ((packetSize1 << 8) | (packetSize2 & 0xff)); //+ (start,address, packetsize1 and packetsize2)
            packetSize += 4;

            uint8_t data[packetSize];

            data[0] = startByte;
            data[1] = addressByte;
            data[2] = packetSize1;
            data[3] = packetSize2;

            for (uint16_t i = 4; i < packetSize; i++) {
              data[i] = btSerial -> read();
            }

            DBG("received raw data: ");

            if (DEBUG) {
              for (uint16_t i = 0; i < packetSize; i++) {
                DBG(String(data[i], HEX));
              }
            }

            OVC3860::decodeReceivedDataArray(data);
          }
          break;
        default:
          {
            Serial.println("ID: ");
            Serial.println(startByte, HEX);
          }
      }
    }

  } else {

    char c;
    String receivedString = "";

    while (btSerial -> available() > 0) {
      c = (btSerial -> read());
      //Serial.write(c);Serial.print(" ");Serial.println(c,HEX);
      if (BTState != ConfigMode) {
        if (c == 0xD) {
          if (receivedString == "") { //nothing before enter was received
            //DBG("received only empty string\n running again myself...\n");
            OVC3860::getNextEventFromBT();
          }
          receivedString = receivedString + c;
          decodeReceivedString(receivedString);
          break;
        }
      }
      //append received buffer with received character
      receivedString = receivedString + c;  // cose += c did not work ...
    }
  }
}

uint8_t OVC3860::sendData(String cmd) {
  String Command = "AT#" + cmd + "\r\n";
  DBG("sending " + Command);
  delay(100);
  btSerial -> print(Command);
}

/*
  uint8_t OVC3860::sendAPTData(String cmd) {
  String Command = "APT+" + cmd + "\r\n";
  DBG("sending APT " + Command);
  delay(100);
  #if defined(USE_SW_SERIAL)
  if (btSerial)
  btSerial -> print(Command);
  else
  #endif
  btSerial -> print(Command);
  }
*/

/*
  Pairing
  Set pairing, waiting for the remote device to connect, the command format is:
  AT#CA // discoverable for 2 minutes
  ovc3860 returns the indication:
  II // state indication, HSHF enters pairing state indication
  if 2 minutes' timeout is arrived(no peer connect to ovc3860 device), returns the indication:
  IJ2 // state indication, HSHF exits pairing mode and enters listening
  The device can't be found, if need to be search, repeat Pairing operation.

  Enter Pairing Mode #CA

  Command
  #CA
  Current Status(s)
  Any
  Possible Indication(s)
  II
  Indication Description
  Enter Pairing Mode Indication

  Description
  This command puts the module in the pairing mode. The information response and causes will
  indicate the command success or failure. Enter pairing mode indication or failure indication
  will be sent to the host.

  Note:
  1. This command will cause a disconnection if module has already connected with some device.
  2. Module will exit pairing mode if connection not happen in 2 minutes.

  Syntax: AT#CA
*/
uint8_t OVC3860::PairingInit() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_PAIRING_INIT);
  OVC3860::getNextEventFromBT();
}

/*
  Exit pairing
  Exit pairing, can not be found by peers, the command format is:
  AT#CB // exit pairing mode, non-discoverable
  return:
  IJ2 // state indication, HSHF exits pairing mode and enters listening

  Cancel Pairing Mode #CB

  Command
  #CB
  Current Status(s)
  Pairing
  Possible   Indication(s)
  IJ2
  Indication Description
  Exit Pairing Mode Indication

  Description
  If the module is in pairing mode, this command causes the module to exit the pairing mode and
  enter the idle mode. The information response and causes will indicate the command success or
  failure.

  Syntax: AT#CB
*/
uint8_t OVC3860::PairingExit() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_PAIRING_EXIT);
  OVC3860::getNextEventFromBT();
}

/*
  Active connection
  Actively connect to the last successful connected device
  AT#CC // Connect to remote Device
  then input the local PIN code, and ovc3860 returns HSHF's state indication:
  IV
  HSHF state is connected indication
  if Bluetooth mobile phone is turned off or not in Bluetooth signal range, will return a stat
  us indication:
  IJ2 // HSHF exits pairing mode and enters listening state indication

  Connect HFP to Handset #CC
  Command
  #CC
  Current Status(s)
  HFP Status = 1
  Possible Indication(s)
  IV
  Description Indication
  Connecting Indication

  Note: You can get current HFP status by #CY.

  Description
  This command causes the module to connect to a paired handset. The information response and
  causes will indicate the command success or failure. Connect Indication will be sent to the host
  after the connection is established. Otherwise Disconnect Indication will be sent to the host.

  Syntax: AT#CC
*/
uint8_t OVC3860::ConnectHSHF() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_CONNECT_HSHF);
  OVC3860::getNextEventFromBT();
}

/*
  Exit connecting
  Send the command:
  AT#CD // ACL disconnect from remote
  ovc3860 returns the indication:
  IA // HSHF state is listening
  if the remote device disconnects connection actively, ovc3860 also returns the same indication

  Disconnect HFP from Handset #CD
  Command
  #CD
  Current Status(s)
  HFP Status ≥ 3
  Possible Indication(s)
  IA
  Description Indication
  Disconnected Indication

  Description
  This command causes the module to disconnect from the connected handset. The information
  response and causes will indicate the command success or failure. Disconnect Indication will be
  sent to the host after the connection is dropped.

  Syntax: AT#CD
*/
uint8_t OVC3860::DisconnectHSHF() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_DISCONNECT_HSHF);
  OVC3860::getNextEventFromBT();
}

/*
  Answering the phone
  The phone receives a call, ovc3860 returns the indication, such as:   02167850001
  Receive a incoming call 02167850001
  at this time, user may refuse to answer the phone:
  AT#CF // Refuse to answer the phone
  ovc3860 returns:
  IF // Call-setup status is idle

  Reject Call #CF

  Command
  #CF
  Current Status(s)
  HFP status = 4
  Possible Indication(s)
  IF
  Indication Description
  Hang up Indication

  Description
  This command causes the module to reject an incoming call. The information response and causes
  will indicate the command success or failure.

  Syntax: AT#CF
*/
uint8_t OVC3860::CallReject() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_CALL_REJECT);
  OVC3860::getNextEventFromBT();
}

/*
  user may answer the phone:
  AT#CE
  return the indication:
  IF // hang-up indication
  if the other party hangs up the phone, also return:
  IF // hang-up indication

  Answer Call #CE
  Command
  #CE
  Current Status(s)
  HFP Status = 4
  Possible
  Indication(s)
  IG
  Indication Description
  Pick up Indication

  Description
  This command causes the module to answer an incoming call. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#CE
*/
uint8_t OVC3860::CallAnswer() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_CALL_ANSWARE);
  OVC3860::getNextEventFromBT();
}

/*
  End Call
  Command
  #CG
  Current Status(s)
  HFP Status = 5 | 6
  Posible Indication(s)
  IF
  Indication Description
  Hang up Indication

  Description
  This command causes the module to end an active call. The information response and causes will
  indicate the command success or failure.
  Syntax: AT#CG
*/
uint8_t OVC3860::CallHangUp() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_CALL_HANGUP);
  OVC3860::getNextEventFromBT();
}

/*
  Redialing
  Redial the last outgoing call:
  AT#CH
  return:
  ICv//Call-setup status is outgoing

  Redial #CH
  Command
  #CH
  Current Status(s)
  HFP Status = 3 | 6
  Possible Indication(s)
  IC
  Indication Description
  Outgoing Call Indication

  Description
  This command causes the module to redial the last number called in the phone. The information
  response and causes will indicate the command success or failure.

  Syntax: AT#CH
*/
uint8_t OVC3860::CallRedial() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_CALL_REDIAL);
  OVC3860::getNextEventFromBT();
}

/*
  Voice dialing
  The command format is:
  AT#CI
  return:
  PE // The voice dial start indication
  or,
  PF // The voice dial is not supported indication

  Voice Dial #CI
  Command
  #CI
  Current Status(s)
  HFP Status = 3
  Possible Indication(s)
  PE
  PF
  Indication Description
  Voice Dial Start Indication
  Handset Not Support Void Dial

  Description
  This command causes the module to active voice dial functionary in the phone. The information
  response and causes will indicate the command success or failure.
  Note: Voice dialing not works in some handset while .

  Syntax: AT#CI
*/
uint8_t OVC3860::VoiceDialStart() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_VOICE_CALL_START);
  OVC3860::getNextEventFromBT();
}

/*
  cancel the voice dialing:
  AT#CJ
  return:
  PF //The voice dial is stopped indication

  Cancel Voice Dial #CJ
  Command
  #CJ
  Current Status(s)
  HFP Status = 3
  Possible Indication(s)
  PF
  Indication Description
  Voice Dial Stop Indication

  Description
  This command causes the module to cancel on going voice dial in the phone. The information
  response and causes will indicate the command success or failure.

  Syntax: AT#CJ
*/
uint8_t OVC3860::VoiceDialEnd() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_VOICE_CALL_CANCEL);
  OVC3860::getNextEventFromBT();
}

/*
  Mute/Unmute MIC #CM

  Command
  #CM
  Current Status(s)
  HFP Status = 6
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted by Module

  Description
  This command causes the module to mute or unmute the MIC. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#CM
*/
uint8_t OVC3860::MicToggle() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_MIC_TOGGLE);
  OVC3860::getNextEventFromBT();
}

/*
  Audio transfer
  Transfer audio between HSHF and phone speaker:
  AT#CO
  when transfered to HSHF, ovc3860 returns:
  MC //The voice is on Bluetooth indication
  when transfered to HSHF:
  MD //The voice is on phone indication

  Transfer Call to/from Handset #CO

  Command
  #CO
  Current Status(s)
  HFP Status = 6(without audio)
  HFP Status = 6(without audio)
  Possible Indication(s)
  MC
  MD
  Indication Description
  HFP Audio Connected
  MD HFP Audio Disconnect

  Description
  This command causes the module to transfer the active call from the module to the handset ( MD
  will received ) or from the handset to the module ( MC will received ). The information response
  and causes will indicate the command success or failure.

  Syntax: AT#CO
*/
uint8_t OVC3860::TransferAudio() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_TRANSFER_AUDIO_TO_SPEAKER);
  OVC3860::getNextEventFromBT();
}

/*
  Release&Reject Call #CQ
  Command
  #CQ
  Current Status(s)
  HFP Status = 6
  Possible Indication(s)
  IN
  Indication Description
  Release Held Call, Reject Waiting Call
  by
  Description
  This command causes the module to release held call, and reject waiting call. The information
  response and causes will indicate the command success or failure.

  Syntax: AT#CQ
*/
uint8_t OVC3860::CallReleaseReject() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_RELEASE_REJECT_CALL);
  OVC3860::getNextEventFromBT();
}

/*
  Release&Accept Call #CR

  Command
  #CR
  Current Status(s)
  HFP Status = 6
  Possible Indication(s)
  IT
  Indication Description
  Release Active Call, Accept Other Call

  Description
  This command causes the module to release active call, accept other call.
  The information response and causes will indicate the command success or failure.

  Syntax: AT#CR
*/
uint8_t OVC3860::CallReleaseAccept() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_RELEASE_ACCEPT_CALL);
  OVC3860::getNextEventFromBT();
}

/*
  Hold&Accept Call #CS

  Command
  #CS
  Current Status(s)
  HFP Status = 6
  Possible Indication(s)
  IM
  Indication Description
  Release Active Call, Accept Other Call

  Description
  This command causes the module to hold active call, accept other call. The information response
  and causes will indicate the command success or failure.

  Syntax: AT#CS
*/
uint8_t OVC3860::CallHoldAccept() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_HOLD_ACCEPT_CALL);
  OVC3860::getNextEventFromBT();
}

/*
  Conference Call #CT

  Command
  #CT
  Current Status(s)
  HFP Status = 6
  Possible Indication(s)
  IM
  Indication Description
  Make Conference Call

  Description
  This command causes the module to make a conference call. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#CT
*/
uint8_t OVC3860::CallConference() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_CONFERENCE_CALL);
  OVC3860::getNextEventFromBT();
}


/*
  taken from BLK module datasheet
  BLK-MD-SPK-B_AT_Command_set_original( BLK-MD-SPK-B AT Command Application Guide)

  no mention of this command in OVC3860_AT_Command_Application_Notes.pdf

  module responce:

  sending AT#CV
  ERR2
  ERR2
  OK
  OK
*/
uint8_t OVC3860::PairingDeleteThenInit() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_PAIRING_DELETE_THEN_INIT);
  OVC3860::getNextEventFromBT();
}

/*
  Dialing
  For example, dialing 10086, the command format is:
  AT#CW10086
  return
  IC // Call-setup status is outgoing
  IP5 // Outgoing call number length indication
  IR10086 // Outgoing call number indication

  Dial One Call #CW

  Command
  #CW
  Current Status(s)
  HFP Status = 3
  Possible Indication(s)
  IC
  IP<lehgth>
  IR<phone number>
  Outgoing Call Indication
  Length of Phone Number
  Current Call Indication

  Description
  This command causes the module to dial one call. The information response and causes will
  indicate the command success or failure.
  Note: IP, IR indications only supported by HFP1.5 version.


  Syntax: AT#CW13800138000
*/
uint8_t OVC3860::CallDialNumber(String number) {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_CALL_DIAL_NUMBER + number);
  OVC3860::getNextEventFromBT();
}

/*
  Sending DTMF
  For example, sending number “1”
  AT#CX1
  return:
  OK // send DTMF successfully indication
  NOTE: sopported sending characters (0-9, #, *, A-D).

  Send DTMF #CX

  Command
  #CX
  Current Status(s)
  HFP Status = 6
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command causes the module to send one DTMF. The information response and causes will
  indicate the command success or failure.
  Syntax:
  AT#CX1
  AT#CX5

*/
uint8_t OVC3860::SendDTMF() {
  OVC3860::sendData(OVC3860_SEND_DTMF);
  OVC3860::getNextEventFromBT();
}

/*
  Query the HSHF applications state, the command is:
  AT#CY
  ovc3860 returns:
  MGX // The HSHF applications state is X indication
  NOTE: X is the return parameter:
          1 – “Ready”
          2 – “Connecting”
          3 – “Connected”
          4 –“Outgoing call”
          5 –“Incoming call”
          6 – “Ongoing call”.

  Query HFP Status #CY

  Command
  #CY
  Current Status(s)
  Any
  Possible Indication(s)
  MG<code>
  Report Current HFP Status

  Description
  This command queries the module’s HFP current status. The information response and causes will
  indicate the command success or failure.

  Syntax: AT#CY
*/
uint8_t OVC3860::QueryHFPStatus() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_QUERY_HFP_STATUS);
  OVC3860::getNextEventFromBT();
}

/*
  Reset #CZ

  Command
  #CZ
  Current Status(s)
  Any
  Possible
  IS<version>
  MF<a><b>
  Indication Description
  Power ON Init Complete
  Report Auto Answer and PowerOn Auto
  Connection Configuration

  Description
  This command causes the module to reset. The information response and causes will indicate the
  command success or failure.

  Syntax: AT#CZ
*/
uint8_t OVC3860::Reset() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_RESET);
  OVC3860::getNextEventFromBT();
}

/*
  Play/Pause Music #MA

  Command
  #MA
  Current Status(s)
  A2DP State = 5
  Possible Indication(s)
  MA
  MB
  Indication Description
  AV pause/stop Indication
  AV play Indication

  Description
  If the module is connected with a AV Source, this command causes the AV source to play/pause
  music. If module isn’t connected AV source, this command will cause module try to connected
  current connected mobile’s AV source. The information response and causes will indicate the
  command success or failure.

  Syntax: AT#MA
*/
uint8_t OVC3860::MusicTogglePlayPause() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_MUSIC_TOGGLE_PLAY_PAUSE);
  OVC3860::getNextEventFromBT();
}

/*
  Stop Music #MC

  Command
  #MC
  Current Status(s)
  A2DP State = 5
  Possible Indication(s)
  MA
  Indication Description
  AV pause/stop Indication

  Description
  If the module is connected with a AV Source, this command causes the AV Source to Stop Music.
  The information response and causes will indicate the command success or failure.

  Syntax: AT#MC
*/
uint8_t OVC3860::MusicStop() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_MUSIC_STOP);
  OVC3860::getNextEventFromBT();
}

/*
  Forward Music #MD

  Command
  #MD
  Current Status(s)
  A2DP State = 5
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted by Module

  Description
  If the module is connected with a AV Source, this command causes the AV Source to Play next
  song. The information response and causes will indicate the command success or failure.

  Syntax: AT#MD
*/
uint8_t OVC3860::MusicNextTrack() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_MUSIC_NEXT_TRACK);
  OVC3860::getNextEventFromBT();
}

/*
  Backward Music #ME

  Command
  #ME
  Current Status(s)
  A2DP State = 5
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted by Module

  Description
  If the module is connected with a AV Source, this command causes the AV Source to play last
  song. The information response and causes will indicate the command success or failure.

  Syntax: AT#ME
*/
uint8_t OVC3860::MusicPreviousTrack() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_MUSIC_PREVIOUS_TRACK);
  OVC3860::getNextEventFromBT();
}

/*
  Query Auto Answer and PowerOn Auto Connection Configuration #MF

  Command Current Status(s)
  #MF
  Current Status(s)
  Any
  Possible Indication(s)
  MF<a><b>
  Indication Description
  Report Auto Answer and PowerOn Auto
  Connection Configuration

  Description
  This command queries the module’s auto answer configuration and poweron auto connect
  configuration. The information response and causes will indicate the command success or failure

  Syntax: MF<a><b>
  Value:
  < a >: auto answer configuration, where 0: disable, 1: enabled
  < b >: poweron auto configuration, where 0: disable, 1: enabled
*/
uint8_t OVC3860::QueryConfiguration() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_QUERY_CONFIGURATION);
  OVC3860::getNextEventFromBT();
}

/*
  Enable PowerOn Auto Connection #MG

  Command
  #MG
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command enables the module to connect to the last used AG after PowerOn. The information
  response and causes will indicate the command success or failure.

  Syntax: AT#MG
*/
uint8_t OVC3860::AutoconnEnable() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_AUTOCONN_ENABLE);
  OVC3860::getNextEventFromBT();
}
/*
  Disable PowerOn Auto Connection #MH

  Command
  #MH
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command disables the module to connect to the Last used AG after PowerOn. The
  information response and causes will indicate the command success or failure.

  Syntax: AT#MH
*/
uint8_t OVC3860::AutoconnDisable() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_AUTOCONN_DISABLE);
  OVC3860::getNextEventFromBT();
}

/*
  Connect to AV Source #MI

  Command
  #MI
  Current Status(s)
  A2DP State = 1
  Possible Indication(s)
  MA
  MB
  Indication Description
  AV pause/stop Indication
  AV play Indication

  Description
  If the module is connected with a HFP phone, this command causes the module try to connect to
  the phone’s AV Source. The information response and causes will indicate the command success
  or failure.
  Note: Music will be played automatic after A2DP connected in some handset.

  Syntax: AT#MI
*/
uint8_t OVC3860::AvSourceConnect() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_AV_SOURCE_CONNECT);
  OVC3860::getNextEventFromBT();
}

/*
  Disconnect from AV Source #MJ

  Command
  #MJ
  Current Status(s)
  A2DP Status ≥ 3
  Possible Indication(s)
  MY
  Indication Description
  AV Disconnect Indication

  Description
  This module causes the module to disconnect from the connected phone’s AV source. The
  information response and causes will indicate the command success or failure.

  Syntax: AT#MJ
*/
uint8_t OVC3860::AvSourceDisconnect() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_AV_SOURCE_DISCONNECT);
  OVC3860::getNextEventFromBT();
}

/*
  Change Local Device Name Casually #MM

  Command
  #MM
  Current Status(s)
  Any
  Parameters
  [new name]
  Possible Indication(s)
  [MM<current name>]
  Indication Description
  Report Current Local Device Name

  Description
  This command causes the module to change the device name. The information response and causes will indicate the command
  success or failure.

  Syntax: AT#MM<new name>

  Value:< new name >: local device name

  ※ If new name is empty, the module will report current local device name.

  example:
  AT#MMMy Car Kit\r\n :the new name is “My Car Kit”
  AT#MM\r\n :indication will be MM<current name>
*/
uint8_t OVC3860::ChangeLocalName(String name) {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_CHANGE_LOCAL_NAME + name);
  OVC3860::getNextEventFromBT();
}

/*
  Change Local Device Pin Code #MN

  Command
  #MN
  Current Status(s)
  Any
  Parameters
  [new pin]
  Possible Indication(s)
  [MN<current pin>]
  Indication Description
  Report current local device Pin code

  Description
  This command causes the module to change the device pin code. The information response and causes will indicate the
  command success or failure.

  Syntax: AT#MN<new pin>\r\n

  Value:<new pin >: local device pin (4 digital),

  ※ If new pin is empty, the module will report current local pin code.

  example:
  AT#MN1234 :the new pin is :1234
  AT#MN :indication will be MP<current pin>
*/
uint8_t OVC3860::ChangePin(String pin) {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_CHANGE_PIN + pin);
  OVC3860::getNextEventFromBT();
}

/*
  Query AVRCP Status #MO

  Command
  #MO
  Current Status(s)
  Any
  Possible Indication(s)
  ML<code>
  <code> Status
  1 Ready (to be connected)
  2 Connecting
  3 Connected
  Indication Description
  Report Current AVRCP Status

  Description
  This command queries the module’s AVRCP current status. The information response and causes
  will indicate the command success or failure.

  Syntax: AT#MO

  Responce ML<code>:
  <code>
  1 Ready (to be connected)
  2 Connecting
  3 Connected

*/
uint8_t OVC3860::QueryAvrcpStatus() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_QUERY_AVRCP_STATUS);
  OVC3860::getNextEventFromBT();
}

/*
  Enable Auto Answer #MP

  Command
  #MP
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command enables the module auto answer an incoming call. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#MP
*/
uint8_t OVC3860::AutoAnswerEnable() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_AUTO_ANSWER_ENABLE);
  OVC3860::getNextEventFromBT();
}

/*
  Disable Auto Answer #MQ

  Command
  #MQ
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command disables the module auto answer an incoming call. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#MQ
*/
uint8_t OVC3860::AutoAnswerDisable() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_AUTO_ANSWER_DISABLE);
  OVC3860::getNextEventFromBT();
}

/*
  Start Fast Forward #MR

  Command
  #MR
  Current Status(s)
  A2DP Status = 5 OK
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted by Module

  Description
  If the module is connected with a AV Source, this command causes the AV Source to start fast
  forward. The information response and causes will indicate the command success or failure.

  Syntax: AT#MR
*/
uint8_t OVC3860::MusicStartFF() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_MUSIC_START_FF);
  OVC3860::getNextEventFromBT();
}

/*
  Start Rewind #MS

  Command
  #MS
  Current Status(s)
  A2DP Status = 5
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted by Module

  Description
  If the module is connected with a AV Source, this command causes the AV Source to start rewind.
  The information response and causes will indicate the command success or failure.

  Syntax: AT#MS
*/
uint8_t OVC3860::MusicStartRWD() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_MUSIC_START_RWD);
  OVC3860::getNextEventFromBT();
}

/*
  Stop Fast Forward / Rewind #MT

  Command
  #MT
  Current Status(s)
  A2DP Status = 5 (after started Fast Forward or Rewind)
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted by Module

  Description
  If the module is connected with a AV Source, this command causes the AV Source to stop fast
  forward or rewind. The information response and causes will indicate the command success or failure.

  Syntax: AT#MT
*/
uint8_t OVC3860::MusicStopFFRWD() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_MUSIC_STOP_FF_RWD);
  OVC3860::getNextEventFromBT();
}

/*
  Query A2DP Status #MV

  Command
  #MV
  Current Status(s)
  Any
  Possible Indication(s)
  MU<code>
  Indication Description
  Report Current A2DP Status

  Description
  This command queries the module’s A2DP current status. The information response and causes
  will indicate the command success or failure.

  Syntax: AT#MV

  Responce:
  MU<code>:1-5, status of A2DP
  1 Ready
  2 Initiating
  3 SignallingActive
  4 Connected
  5 Streaming
*/
uint8_t OVC3860::QueryA2DPStatus() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_QUERY_A2DP_STATUS);
  OVC3860::getNextEventFromBT();
}

/*
  Write to Memory #MW

  Command
  #MW
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command causes the module to write a byte into a given memory address. The information
  response and causes will indicate the command success or failure.

  Syntax: AT#MWADDR_VAL
  ADDR: a given 32-bit, hexadecimal address
  VAL: a written hexadecimal byte value
*/
uint8_t OVC3860::WriteToMemory(String data) {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_WRITE_TO_MEMORY + data);
  OVC3860::getNextEventFromBT();
}

/*
  Read from Memory #MX

  Command
  #MX
  Current Status(s)
  Any
  Possible Indication(s)
  MEM:<val>
  Indication Description
  The Returned Value


  Description
  This command causes the module to read a byte from a given memory address. The information
  response and causes will indicate the returned value from module reading.

  Syntax: AT#MXADDR
  ADDR: a given 32-bit, hexadecimal address
  <val>: a read hexadecimal byte value
*/
uint8_t OVC3860::ReadToMemory(String data) {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_READ_FROM_MEMORY + data);
  OVC3860::getNextEventFromBT();
}

/*
  Switch Two Remote Devices #MZ

  Command
  #MZ
  Current Status(s)
  Any
  Possible
  Indication(s)
  SW
  Indication Description
  Command Accepted

  Description
  This command causes the module to switch two remote devices. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#MZ
*/
uint8_t OVC3860::SwitchDevices() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_SWITCH_DEVICES);
  OVC3860::getNextEventFromBT();
}

/* taken from BLK-MD-SPK-B AT Command Application Guide, did not work on module for me (kovo)
    pdf: https://github.com/tomaskovacik/kicad-library/blob/master/library/datasheet/OVC3860_based_modules/BLK-MD-SPK-B_AT_Command_set_original.pdf
  Query Module Software Version #MY

  Command
  #MY
  Current Status(s)
  Any
  Parameters
  MW<version>
  Indication Description
  Report Module Software Version

  Description
  This command queries the module’s software version. The information response and causes will indicate the command
  success or failure.

  Syntax: AT#MY
*/
uint8_t OVC3860::QueryVersion() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_QUERY_VERSION);
  OVC3860::getNextEventFromBT();
}

/*
  Synchronize Phonebook Stored by SIM(via AT Command) #PA

  Command
  #PA
  Current Status(s)
  HFP Status = 3
  Possible Indication(s)
  PA1
  PA0
  Indication Description
  Phonebook Synchronize Indication
  Command Not Supported

  Description
  This command causes the module to synchronize the phonebook which is stored by SIM. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#PA
*/
uint8_t OVC3860::PbSyncBySim() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_PB_SYNC_BY_SIM);
  OVC3860::getNextEventFromBT();
}

/*
  Synchronize Phonebook Stored by Phone(via AT Command) #PB

  Command
  #PB
  Current Status(s)
  HFP Status = 3
  Possible Indication(s)
  PA1
  PA0
  Indication Description
  Phonebook Synchronize Indication
  Command Not Supported

  Description
  This command causes the module to synchronize the phonebook which is stored by phone. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#PB
*/
uint8_t OVC3860::PbSyncByPhone() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_PB_SYNC_BY_PHONE);
  OVC3860::getNextEventFromBT();
}

/*
  Read Next One Phonebook Item #PC

  Command
  #PC
  Current Status(s)
  HFP Status = 3
  Possible Indication(s)
  IP<lehgth>
  PB<phonebook>
  PC
  Indication Description
  Length of Phone Number
  One Phonebook Indication
  End of Phonebook/Call History

  Description
  This command causes the module to read next one phonebook item from phone or local. The information response and causes
  will indicate the command success or failure.

  Syntax: AT#PC
*/
uint8_t OVC3860::PbReadNextItem() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_PB_READ_NEXT_ITEM);
  OVC3860::getNextEventFromBT();
}

/*
  Read Previous One Phonebook Item #PD

  Command
  #PD
  Current Status(s)
  HFP Status = 3
  Possible Indication(s)
  IP<lehgth>
  PB<phonebook>
  PC
  Indication Description
  Length of Phone Number
  One Phonebook Indication
  End of Phonebook/Call History

  Description
  This command causes the module to read previous one phonebook item from phone or local. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#PD
*/
uint8_t OVC3860::PbReadPreviousItem() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_PB_READ_PREVIOUS_ITEM);
  OVC3860::getNextEventFromBT();
}

/*
  Synchronize Dialed Calls List (via AT Command) #PH

  Command
  #PH
  Current Status(s)
  HFP Status = 3
  Possible Indication(s)
  PA1
  PA0
  Indication Description
  Phonebook Synchronize Indication
  Command Not Supported

  Description
  This command causes the module to synchronize the dialed calls list. The information response and causes will indicate the
  command success or failure.

  Syntax: AT#PH
*/
uint8_t OVC3860::PbSyncByDialer() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_PB_SYNC_BY_DIALED);
  OVC3860::getNextEventFromBT();
}

/*
  Synchronize Received Calls List (via AT Command) #PI

  Command
  #PI
  Current Status(s)
  HFP Status = 3
  Possible Indication(s)
  PA1
  PA0
  Indication Description
  Phonebook Synchronize Indication
  Command Not Supported

  Description
  This command causes the module to synchronize the received calls list. The information response and causes will indicate the
  command success or failure.

  Syntax: AT#PI
*/
uint8_t OVC3860::PbSyncByReceiver() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_PB_SYNC_BY_RECEIVED);
  OVC3860::getNextEventFromBT();
}

/*
  Synchronize Missed Calls List (via AT Command) #PJ

  Command
  #PJ
  Current Status(s)
  HFP Status = 3
  Possible Indication(s)
  PA1
  PA0
  Indication Description
  Phonebook Synchronize Indication
  Command Not Supported

  Description
  This command causes the module to synchronize the missed calls list. The information response and causes
  will indicate the command success or failure.

  Syntax: AT#PJ
*/
uint8_t OVC3860::PbSyncByMissed() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_PB_SYNC_BY_MISSED);
  OVC3860::getNextEventFromBT();
}

/*
  Synchronize Last Call List (via AT Command) #PK

  Command
  #PK
  Current Status(s)
  HFP Status = 3
  Possible Indication(s)
  PA1
  PA0
  Indication Description
  Phonebook Synchronize Indication
  Command Not Supported

  Description
  This command causes the module to synchronize the last call list. The information response and causes will indicate the
  command success or failure.

  Syntax: AT#PK
*/
uint8_t OVC3860::PbSyncByLastCall() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_PB_SYNC_BY_LAST_CALL);
  OVC3860::getNextEventFromBT();
}

/*
  Get Recent Dialed Call History (record by module) #PL

  Command
  #PL
  Current Status(s)
  Any
  Possible Indication(s)
  IP<length>
  MN<phone number>
  PC
  Indication Description
  Length of Phone Number Indication
  One Phonebook Indication
  End of Phonbook/Call History Download Ind

  Description
  This command causes the module to read one of recently dialed call number(record by module). The information response and
  causes will indicate the command success or failure.

  Syntax: AT#PL
*/
uint8_t OVC3860::GetLocalLastDialedList() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_GET_LOCAL_LAST_DIALED_LIST);
  OVC3860::getNextEventFromBT();
}

/*
  Get Recent Received Call History (record by module) #PM

  Command
  #PM
  Current Status(s)
  Any
  Possible Indication(s)
  IP<length>
  MN<phone number>
  PC
  Indication Description
  Length of Phone Number Indication
  One Phonebook Indication
  End of Phonbook/Call History Download Ind

  Description
  This command causes the module to read one of recently received call number(record by module). The information response
  and causes will indicate the command success or failure.

  Syntax: AT#PM
*/
uint8_t OVC3860::GetLocalLastReceivedList() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_GET_LOCAL_LAST_RECEIVED_LIST);
  OVC3860::getNextEventFromBT();
}

/*
  Get Recent Missed Call History (record by module) #PN

  Command
  #PN
  Current Status(s)
  Any
  Possible Indication(s)
  IP<length>
  MN<phone number>
  PC
  Indication Description
  Length of Phone Number Indication
  One Phonebook Indication
  End of Phonbook/Call History Download Ind

  Description
  This command causes the module to read one of recently missed call number(record by module). The information response
  and causes will indicate the command success or failure.

  Syntax: AT#PN
*/
uint8_t OVC3860::GetLocalLastMissedList() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_GET_LOCAL_LAST_MISSED_LIST);
  OVC3860::getNextEventFromBT();
}

/*
  Dial Last Received Phone Number #PO

  Command
  #PO
  Current Status(s)
  HFP Status = 3 | 6
  Possible Indication(s)
  IC
  IP<length>
  IR<phone number>
  Indication Description
  Outgoing Call Indication
  Length of Phone Number Indication
  Current Call Indication

  Description
  This command causes the module to dial last received phone number. The information response and causes will indicate the
  command success or failure.

  Note: IP,IR indication only supported by HFP1.5 version.

  Syntax: AT#PO
*/
uint8_t OVC3860::DialLastReceivedCall() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_DIAL_LAST_RECEIVED_CALL);
  OVC3860::getNextEventFromBT();
}

/*
  Clear Call History (record by module) #PR

  Command
  Current Status(s)
  Possible Indication(s) Indication Description
  #PO
  Any
  OK
  Command Accepted by Module

  Description
  This command causes the module to clear call history(record by module). The information response and causes will indicate
  the command success or failure.

  Syntax: AT#PR
*/
uint8_t OVC3860::ClearLocalCallHistory() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_CLEAR_LOCAL_CALL_HISTORY);
  OVC3860::getNextEventFromBT();
}

/*
  SPP data transmit #ST

  Command
  #ST
  Current Status(s)
  When SPP is connected
  Possible Indication(s)
  OK
  IndicationDescription
  Command Accepted


  Description
  This command will send SPP data to the remote device.

  Syntax: AT#STdata
  data: the string you need to send. The max len is 20.
*/
uint8_t OVC3860::SppDataTransmit() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_SPP_DATA_TRANSMIT);
  OVC3860::getNextEventFromBT();
}

/*
  Set Clock Debug Mode #VC
  Command
  #VC
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  IndicationDescription
  Command Accepted

  Description
  This command causes the module to enter clock debug mode. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#VC
*/
uint8_t OVC3860::SetClockdebugMode() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_SET_CLOCKDEBUG_MODE);
  OVC3860::getNextEventFromBT();
}

/*
  Speaker Volume Down #VD

  Command
  #VD
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  IndicationDescription
  Command Accepted

  Description
  This command causes the module to decrease the speaker volume. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#VD
*/
uint8_t OVC3860::VolumeDown() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_VOLUME_DOWN);
  OVC3860::getNextEventFromBT();
}

/*
  Enter BQB Test Mode #VE

  Command
  #VE
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  IndicationDescription
  Command Accepted

  Description
  This command causes the module to enter test mode. The information response and causes will
  indicate the command success or failure.

  Syntax: AT#VE
*/
uint8_t OVC3860::EnterTestMode() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_ENTER_TEST_MODE);
  OVC3860::getNextEventFromBT();
}

/*
  Set to Fixed Frequency #VF

  Command
  #VF
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command causes the module to work at 2404MHz. The information response and causes will
  indicate the command success or failure.

  Syntax: AT#VF
*/
uint8_t OVC3860::SetFixedFrequency() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_SET_FIXED_FREQUENCY);
  OVC3860::getNextEventFromBT();
}

/*
  Enter EMC Test Mode #VG

  Command
  #VG
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command causes the module to work at 2404MHz. The information response and causes will
  indicate the command success or failure.

  Syntax: AT#VGxx_yy

  xx: if set to be 0~78, the frequency is fixed at (2402+xx)MHz, If set to 88, the frequency is in
  hopping mode.

  yy: set the tx packet type according to the following table.

  Packet Type Value
  DH1          '04
  DH3          '08
  DH5           10
  2DH1         '05
  2DH3          11
  2DH5          13
  3DH1         '06
  3DH3          12
  3DH5          14
*/
uint8_t OVC3860::EmcTestMode() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_EMC_TEST_MODE);
  OVC3860::getNextEventFromBT();
}

/*
  Set RF Register #VH

  Command
  #VH
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command causes the module to set a RF register with a given value. The information
  response and causes will indicate the command success or failure.

  Syntax: AT#VHxx_yy
  xx: a register address
  yy: a byte value
  Example: AT#VH54_88(set RF reg 0x54 to be 0x88)
*/
uint8_t OVC3860::SetRFRegister() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_SET_RF_REGISTER);
  OVC3860::getNextEventFromBT();
}

/*
  Start Inquiry #VI

  Command
  #VI
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command causes the module to inquiry Bluetooth devices. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#VI
*/
uint8_t OVC3860::InquiryStart() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_INQUIRY_START);
  OVC3860::getNextEventFromBT();
}

/*
  Cancel Inquiry #VJ

  Command
  #VJ
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command causes the module to cancel inquiry Bluetooth devices. The information response
  and causes will indicate the command success or failure.

  Syntax: AT#VJ
*/
uint8_t OVC3860::InquiryStop() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_INQUIRY_STOP);
  OVC3860::getNextEventFromBT();
}

/*
  Speaker Volume Up #VU

  Command
  #VU
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command causes the module to increase the speaker volume. The information response and
  causes will indicate the command success or failure.

  Syntax: AT#VU
*/
uint8_t OVC3860::VolumeUp() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_VOLUME_UP);
  OVC3860::getNextEventFromBT();
}

/*
  Power Off OOL #VX

  Command
  #VX
  Current Status(s)
  Any
  Possible Indication(s)
  OK
  Indication Description
  Command Accepted

  Description
  This command causes the module to power off OOL. The information response and causes will
  indicate the command success or failure.

  Syntax: AT#VX
*/
uint8_t OVC3860::Shutdown() {
  OVC3860::getNextEventFromBT();
  OVC3860::sendData(OVC3860_SHUTDOWN_MODULE);
  OVC3860::getNextEventFromBT();
}