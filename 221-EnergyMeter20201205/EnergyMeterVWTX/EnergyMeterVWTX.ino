// Part of the code (concerning the watchdog timer and external interrupt)
// is by Jack Christensen 19Nov2013 (https://forum.arduino.cc/index.php?topic=199576.0)
// CC BY-SA, see http://creativecommons.org/licenses/by-sa/3.0/


// VDRs have a resistance of about 3K when illuminated with LED light
// and greater than 30K in the dark


//                   33K
//              |---/\/\/\----> + 5V
//              |
// |||--/\/\/\--*-------------> pinPhotoRes1
// GND  VDR A   |   1N4148
//              |---|<|---|
//                        *---> pinExtInterrupt
//              |---|<|---|
// GND  VDR B   |   1N4148
// |||--/\/\/\--*-------------> pinPhotoRes2
//              |   33K
//              |---/\/\/\----> + 5V
//
//      |--------------|
// GND  |           VCC|----------< pinPTT
// |||--| radio module |
//      |           TX |----------< pinTx
//      |--------------|
//
// |||----|<|---/\/\/\------------< pinLED
//        LED


// ==========================
// libraries
// ==========================

#include <avr/wdt.h>      // for interrupts (internal watchdog and external)
#include <avr/sleep.h>    // for interrupts (internal watchdog and external)
#include <util/atomic.h>  // for interrupts (internal watchdog and external)
#include <VirtualWire.h>  // radio communication
#include <EEPROM.h>       // read/write internal EEPROM

// ==========================
// pins
// ==========================

#define pinExtInterrupt 2
#define pinPhotoRes1    8
#define pinPhotoRes2    7
#define pinTx          12
#define pinPTT         13
#define pinLED         11

// ==========================
// EEPROM addresses
// ==========================

#define eePromAddressCounter1 0
#define eePromAddressCounter2 20

// ==========================
// 'volatile' variables
// ==========================

volatile char bufferTx[] = "CA0000000000";     // TX buffer 10+2 chars
volatile boolean flagExtInterrupt;             // external interrupt flag
volatile boolean flagWatchdogInterrupt;        // watchdog timer interrupt flag
volatile int counterHour;                      // timekeeping for long timing
volatile int counterMinute;                    // timekeeping for short timing
volatile unsigned long pulsePowerActive = 0;   // number of pulses for Active Power
volatile unsigned long pulsePowerReactive = 0; // number of pulses for Reactive Power

volatile unsigned long pulsePowerActive_Previous = 0;
volatile boolean txEnabled = false;


void loop(void)
{

  // ======================
  // sleeps the ATMEGA328P
  // ======================

  gotoSleep();

  // =================
  // When it wakes up
  // =================

  if (flagWatchdogInterrupt) {
    // if it have been woken up by the 8-second watchdog
    // increases counters by 8 seconds
    counterMinute += 8;
    counterHour   += 8;

    // If the number of active power counts is very high
    // it will transmit the values every time
    // the active power LED flashes

    //    unsigned long diff;
    //    if (pulsePowerActive >= pulsePowerActive_Previous) {
    //      diff = pulsePowerActive - pulsePowerActive_Previous;
    //    } else {
    //      diff = 0;
    //    }
    //
    //    if (diff > 5) {
    //
    //      // close-up shots: the power consumption is very high
    //      txEnabled = true;
    //
    //    } else {

    // the current consumption is nominal

    if (counterMinute > 15) {

      // if more than one minute has passed
      // since the last lightning strike,
      // it still transmits the active power

      counterMinute = 0;
      TX1();

    } else {

      // one minute has not passed yet:
      // it checks if an hour has passed
      // in the other counter

      if (counterHour > 3600) {

        counterHour = 0;

        // saves the values of both pulse-counters
        unsigned long temp = pulsePowerActive;
        EEPROM.put(eePromAddressCounter1, temp);

        temp = pulsePowerReactive;
        EEPROM.put(eePromAddressCounter2, temp);

      }
    }
    //}

    // takes note of the current pulsePowerActive
    pulsePowerActive_Previous = pulsePowerActive;

  }

  // =======================================
  // Check if it was woken up
  // by the external interrupt (LED flashes)
  // =======================================

  if (flagExtInterrupt) {
    if (pulsePowerActive > 9999) {
      pulsePowerActive = 0;;
    }

    if (pulsePowerReactive > 9999) {
      pulsePowerReactive = 0;;
    }

    // radio trasmission

    if (txEnabled) {
      txEnabled = false;
      TX1();
    }
  }
}

void gotoSleep(void) {
  byte adcsra = ADCSRA;          // save the ADC Control and Status Register A
  ADCSRA = 0;                    // disable the ADC
  EICRA = _BV(ISC01);            // configure INT0 to trigger on falling edge
  EIFR = _BV(INTF0);             // ensure interrupt flag cleared
  EIMSK = _BV(INT0);             // enable INT0
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    flagWatchdogInterrupt = false;
    flagExtInterrupt = false;
    sleep_enable();
    sleep_bod_disable();         // disable brown-out detection (saves 20-25µA)
  }
  sleep_cpu();                   // go to sleep
  sleep_disable();               // wake up here
  ADCSRA = adcsra;               // restore ADCSRA
}

//external interrupt 0 wakes the MCU
ISR(INT0_vect)
{
  EIMSK = 0;                     //disable external interrupts (only need one to wake up)
  flagExtInterrupt = true;
  if (digitalRead(pinPhotoRes1) == LOW) {
    pulsePowerActive++;
  }
  if (digitalRead(pinPhotoRes2) == LOW) {
    pulsePowerReactive++;
  }
}

//handles the Watchdog Time-out Interrupt
ISR(WDT_vect)
{
  flagWatchdogInterrupt = true;
}

void setup(void)
{
  // all pins resetted to OUTPUT with LOW state
  for (int i = 0; i < 14; i++)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }


  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW);
  pinMode(A1, OUTPUT);
  digitalWrite(A1, LOW);
  pinMode(A2, OUTPUT);
  digitalWrite(A2, LOW);
  pinMode(A3, OUTPUT);
  digitalWrite(A3, LOW);
  pinMode(A4, OUTPUT);
  digitalWrite(A4, LOW);
  pinMode(A5, OUTPUT);
  digitalWrite(A5, LOW);
  // cambia i pin specifici considerati come INPUT_PULLUP
  pinMode(pinExtInterrupt, INPUT_PULLUP);
  pinMode(pinPhotoRes1, INPUT_PULLUP);
  pinMode(pinPhotoRes2, INPUT_PULLUP);

  // Watchdog & interrupt esterno
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    wdt_reset();
    MCUSR &= ~_BV(WDRF);                            //clear WDRF
    WDTCSR |= _BV(WDCE) | _BV(WDE);                 //enable WDTCSR change
    WDTCSR =  _BV(WDIE) | _BV(WDP3) | _BV(WDP0);    //~8 sec
  }

  resetBufferTx();

  unsigned long temp;

  // salva i contatori su EEPROM
  //  temp = 999999999UL;
  //  EEPROM.put(eePromAddressCounter1, temp);
  //
  //  temp = 999999999UL;
  //  EEPROM.put(eePromAddressCounter2, temp);

  // carica il contatore da EEPROM
  temp = 0;
  EEPROM.get(eePromAddressCounter1, temp);
  pulsePowerActive = temp;

  temp = 0;
  EEPROM.get(eePromAddressCounter2, temp);
  pulsePowerReactive = temp;

}

///////////////////////////////// AGGIUNTO //////////////

void sendRadio() {
  //digitalWrite(pinLED, HIGH); // turns on the TX module
  vw_send((uint8_t *)bufferTx, sizeof(bufferTx));
  vw_wait_tx(); // Wait until the whole message is gone
  //digitalWrite(pinLED, LOW);  // turns off the TX module
}

void resetBufferTx() {
  for (int i = 0; i < 10; i++) {
    bufferTx[i + 2] = 0;
  }
}

//==========================================
// the unisgned long value is transfered to
// bufferTx. the bufferTx[0] is untouched
//==========================================
void valToTXbuffer() {
  unsigned long theValue=0;

  char buf[10];
  
  theValue = pulsePowerActive;
  
  ltoa(theValue, buf, 10); // 10 is the base-value not the length
  for (int i = 0; i < 4; i++) {
    if (int(buf[i]) > 0) {
      bufferTx[i + 2] = buf[i];
    } else {
      break;
    }
  }

  theValue = pulsePowerReactive;
  
  ltoa(theValue, buf, 10); // 10 is the base-value not the length
  for (int i = 0; i < 4; i++) {
    if (int(buf[i]) > 0) {
      bufferTx[i + 6] = buf[i];
    } else {
      break;
    }
  }  
}

void TX1() {
  vw_set_tx_pin(pinTx);
  vw_set_ptt_pin(pinPTT);
  vw_set_ptt_inverted(false);
  vw_setup(2000);       // Bits per sec

  valToTXbuffer();
  bufferTx[0] = 'C';
  bufferTx[1] = 'A';
  sendRadio();

  resetBufferTx();

//  valToTXbuffer();
//  bufferTx[0] = 'C';
//  bufferTx[1] = 'B';
//  sendRadio();
}
