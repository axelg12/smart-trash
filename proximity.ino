#include <Wire.h>

#include <SmeSFX.h>
#include <VL6180.h>
#include <sl868a.h>
#include <Arduino.h>

const int LONG_DELAY = 2000; // 10 min
const int SHORT_DELAY = 2000; // 5 min
bool lastTimeFull = false;
bool sending = false;
float latitude = 0;
float longitude = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  Wire.begin();

  if (!smeProximity.begin()) {
    while (1) {
      ; // endless loop due to error on VL6180 initialization
    }
  }
  SerialUSB.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
  if (!sending) {
    latitude = 0;
    longitude = 0;
    if (smeGps.ready()) {
      latitude   = smeGps.getLatitude();
      longitude  = smeGps.getLongitude();
      SerialUSB.print("Latitude    =  ");
      SerialUSB.println(latitude, 6);
      SerialUSB.print("Longitude   =  ");
      // checkSensor(); 
    }
   else {
    waitForAnswer();
  }
}
}

void waitForAnswer() {
  bool answerReady = sfxAntenna.hasSfxAnswer();
  if (answerReady) {
        SerialUSB.println((const char*)sfxAntenna.readSwVersion());
        if (sfxAntenna.getSfxMode() == sfxDataMode) {

            switch (sfxAntenna.sfxDataAcknoledge()) {
            case SFX_DATA_ACK_START:
                SerialUSB.println("Waiting Answer");
                break;

            case SFX_DATA_ACK_PROCESSING:
                SerialUSB.print('.');
                break;

            case SFX_DATA_ACK_OK:
#ifndef ASME3_REVISION
                ledGreenLight(HIGH);
#endif
                SerialUSB.println(' ');
                SerialUSB.println("Answer OK");
                sending = false;
                lastTimeFull = false;
                break;

            case SFX_DATA_ACK_KO:
#ifndef ASME3_REVISION
                ledRedLight(HIGH);
#endif
                SerialUSB.println(' ');
                SerialUSB.println("Answer KO");
                sending = false;
                lastTimeFull = false;
                break;
            }
        }
    }
}

void checkSensor() {
  char ligth = smeProximity.rangePollingRead();

  if (ligth == 255) {
    SerialUSB.println("Infinity");
    // delay(LONG_DELAY)
  } else {
    int dist = int(ligth);
    if (dist <= 150) {
      if (lastTimeFull == false) {
        // wait for a short amount of time
        // if the sensor is still firing the trashcan is probably full
        lastTimeFull = true;
        delay(SHORT_DELAY);
      } else {
        
        sendSigFox();
        SerialUSB.println("Sending");
        // send sigfox, wait up til 30 minutes before sending again
        delay(LONG_DELAY * 3);
      }
    }
    SerialUSB.print(int(ligth));
    SerialUSB.println(" mm");
  }
  
  // delay(300000)          // wait 5 minutes
  delay(2000);              // wait for 2 seconds
}

void sendSigFox() {
  // TODO
  char msg[5] = {'F'};
  
  SerialUSB.println("sending over the network");
  sending = true;
  sfxAntenna.begin();
  sfxAntenna.sfxSendData(msg, strlen((char*)msg));
}
