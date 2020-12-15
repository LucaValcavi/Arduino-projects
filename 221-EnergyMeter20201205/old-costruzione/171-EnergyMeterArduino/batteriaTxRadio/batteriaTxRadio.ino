// **** INCLUDES *****
#include "LowPower.h"
#include <YAMorseTx.h>
YAMorseTx MORSE = YAMorseTx();
/////////////////////////////////////////////
// This program is a wireless energy meter.

// It uses two photoresistors to receive the light from the measurement instrument.
// It provides two LEDs that blink when the Active-Power (LED 1) or Reactive-Power
// (LED 2) reach 1Wh. My circuit retransmits the status of these LEDs using a radio
// module: 10 LED blinks correspond to 1 radio transmission.

// Since the power supply of the circuit comes from a battery, the power
// consumption is fundamental. Indeed the Arduino is in sleeping-state until one of
// two photoresistors receives the light from the corresponding LED.

// When this occurs, the TX circuit sends a dummy letter via the Morse code. The
// letters are in sequence to detect losing pulses. The Photoresistor A sends
// letters from A to M while the Photoresistor B the letters from N to Z.
// ///////////////////////////////////////
// PIN IDE  1  <------- INPUT - PULL UP
// PIN IDE  2  <------- INPUT - PULL UP (photoresistor A) wakeUpPinA 
// PIN IDE  3  <------- INPUT - PULL UP (photoresistor B) wakeUpPinB 
// PIN IDE  4  <------- INPUT - PULL UP
// PIN IDE  5  <------- INPUT - PULL UP
// PIN IDE  6  <------- INPUT - PULL UP
// PIN IDE  7  <------- INPUT - PULL UP
// PIN IDE  8  <------- INPUT - PULL UP
// PIN IDE  9  <------- INPUT - PULL UP
// PIN IDE 10  <------- INPUT - PULL UP
// PIN IDE 11  <------- INPUT - PULL UP
// PIN IDE 12  -------> OUTPUT          (MODULE TX DATA) pinTx
// PIN IDE 13  -------> OUTPUT          (MODULE TX VCC)  pinPTT     
// PIN IDE A0  <------- INPUT - PULL UP
// PIN IDE A1  <------- INPUT - PULL UP
// PIN IDE A2  <------- INPUT - PULL UP
// PIN IDE A3  <------- INPUT - PULL UP
// PIN IDE A4  <------- INPUT - PULL UP
// PIN IDE A5  <------- INPUT - PULL UP
// ///////////////////////////////////////

//  |-----------------> PIN 2 IDE (ATMEGA328 pin 4)
// (photores A)
//  |
// GND
//
//  |-----------------> PIN 3 IDE (ATMEGA328 pin 5)
// (photores B)
//  |
// GND
//
// |--------------|
// | radio module |
// |--------------|
// |              |
// |          Vcc |<--- PIN 13 IDE (ATMEGA328 pin 19)
// |           TX |<--- PIN 12 IDE (ATMEGA328 pin 18)
// |              |
// |--------------|
//        |
//       GND
 
const int wakeUpPinA = 2;
const int wakeUpPinB = 3;
volatile uint8_t counterA = 0;
volatile uint8_t counterB = 0;
volatile uint8_t toggleA = 0;
volatile uint8_t toggleB = 13;

char bufferTx[]="A"; 

#define dotLength     24 // ms (values from 6ms to 6500ms are allowed)
#define pinTx         12 // you can use any pin
#define freqKeyUp   1500 // Hz
#define freqKeyDown 3000 // Hz

#define pinPTT       13

void wakeUpRoutineA()
{
    counterA++;
}

void wakeUpRoutineB()
{
    counterB++;
}

void setup()
{
    // all pins are configured as INPUT_PULLUP
    // for low power consuption
    for(int i=0; i<14; i++)
    {
      pinMode(i, INPUT_PULLUP);
    }
    pinMode(A0, INPUT_PULLUP);
    pinMode(A1, INPUT_PULLUP);
    pinMode(A2, INPUT_PULLUP);
    pinMode(A3, INPUT_PULLUP);
    pinMode(A4, INPUT_PULLUP);
    pinMode(A5, INPUT_PULLUP);
    // now change tx pin to output
    pinMode(pinTx, OUTPUT); 
    pinMode(pinPTT, OUTPUT);
    digitalWrite(pinPTT, LOW);
    // pinMode(wakeUpPinA, INPUT_PULLUP);  --- already set
    // pinMode(wakeUpPinB, INPUT_PULLUP);  --- already set
    MORSE.mm_setup(dotLength,pinTx,freqKeyUp,freqKeyDown);
}

void sendRadio(){
  digitalWrite(pinPTT, HIGH); // turns on the TX module
  delay(100);                 // wait some milliseconds
  MORSE.mm_send((uint8_t *)bufferTx,sizeof(bufferTx)); // TXs
  // MORSE.mm_txNoTones();    // removes modulation tones
  digitalWrite(pinPTT, LOW);  // turns off the TX module
}

void loop() 
{
    // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(digitalPinToInterrupt(wakeUpPinA), wakeUpRoutineA, FALLING);
    attachInterrupt(digitalPinToInterrupt(wakeUpPinB), wakeUpRoutineB, FALLING);
    
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(digitalPinToInterrupt(wakeUpPinA)); 
    detachInterrupt(digitalPinToInterrupt(wakeUpPinB)); 
    
    // main routine

    
    if (counterA >= 3){
      counterA = 0;               //       0  1  2  ...  12
      //bufferTx[0] = 65 + toggleA; // sends A, B, C until M
      // char to send: alternatively R and D
      if (toggleA == 0){
        toggleA = 1;
        bufferTx[0] = 'N';
      } else {
        toggleA = 0;
        bufferTx[0] = 'A';
      }
      // send char to Tx Radio
      sendRadio();
    }

    if (counterB >= 3){
      counterB = 0;               //       0  1  2  ...  12
      //bufferTx[0] = 78 + toggleB; // sends N, O, P until Z
      // char to send: alternatively R and D
      if (toggleB == 0){
        toggleB = 1;
        bufferTx[0] = 'R';
      } else {
        toggleB = 0;
        bufferTx[0] = 'D';
      }
      // send char to Tx Radio
      sendRadio();
    } 
}
