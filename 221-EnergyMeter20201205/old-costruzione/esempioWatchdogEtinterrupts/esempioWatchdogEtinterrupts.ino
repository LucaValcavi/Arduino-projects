//Using the watchdog timer, sleep for the given number of intervals,
//then print a message. Prints a dot at each WDT wakeup.
//A button is also connected to provide an external interrupt, print
//a message when the button is pressed.
//Finally, blink an LED after each wake-up regardless of source.
//Using an ATmega328P at 16MHz and 5V, draws ~6.3µA while sleeping, which
//is consistent with only the WDT running.
//
//Jack Christensen 19Nov2013
//CC BY-SA, see http://creativecommons.org/licenses/by-sa/3.0/

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include <YAMorseTx.h>
YAMorseTx MORSE = YAMorseTx();

const int pinExtInterrupt = 2;            //wire a button from pin 2 to ground
const int pinPhotoRes1 = 7;            //wire a button from pin 2 to ground
const int pinPhotoRes2 = 8;            //wire a button from pin 2 to ground

#define pinTx         12 // you can use any pin
#define pinPTT       13
#define dotLength     24 // ms (values from 6ms to 6500ms are allowed)
#define freqKeyUp   1500 // Hz
#define freqKeyDown 3000 // Hz

volatile char bufferTx[]="ETSA";

const int LED_PIN = 11;           //wire an LED from pin 13 to ground through a proper current-limiting resistor
const int WDT_INTERVALS = 1;     //number of WDT timeouts before printing message
const unsigned long LED_ON_TIME = 200;  //turn on LED for this many ms after each wake-up


volatile boolean extInterrupt;    //external interrupt flag (button)
volatile boolean wdtInterrupt;    //watchdog timer interrupt flag

void setup(void)
{

      for(int i=0; i<14; i++)
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
    
    pinMode(pinExtInterrupt, INPUT_PULLUP);
    pinMode(pinPhotoRes1, INPUT_PULLUP);
    pinMode(pinPhotoRes2, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    pinMode(pinTx, OUTPUT); 
    pinMode(pinPTT, OUTPUT);
    digitalWrite(pinPTT, LOW);
    digitalWrite(pinTx, LOW);

    MORSE.mm_setup(dotLength,pinTx,freqKeyUp,freqKeyDown);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        wdt_reset();
        MCUSR &= ~_BV(WDRF);                            //clear WDRF
        WDTCSR |= _BV(WDCE) | _BV(WDE);                 //enable WDTCSR change
        WDTCSR =  _BV(WDIE) | _BV(WDP3) | _BV(WDP0);    //~8 sec
    }

}

void loop(void)
{
    static int wdtCount;
    static int btnCount;

    gotoSleep();

   

    if (wdtInterrupt) {
        if (++wdtCount >= WDT_INTERVALS) {
            // interrupt 8 secondi
            wdtCount = 0;
                   // interrupt esterno
//         digitalWrite(LED_PIN, HIGH);    //blink the LED on each wakeup
//    delay(LED_ON_TIME);
//    digitalWrite(LED_PIN, LOW);
        }
    }
    if (extInterrupt) {
      if (digitalRead(pinPhotoRes1)==HIGH){
        digitalWrite(LED_PIN, HIGH);    //blink the LED on each wakeup
        delay(1);
        digitalWrite(LED_PIN, LOW);
      } else {
        if (digitalRead(pinPhotoRes2)==HIGH){
          digitalWrite(LED_PIN, HIGH);    //blink the LED on each wakeup
          sendRadio();
          digitalWrite(LED_PIN, LOW);
        } else {
          // this condition must not happen
        }
      }
    }

}

void gotoSleep(void)
{
    byte adcsra = ADCSRA;          //save the ADC Control and Status Register A
    ADCSRA = 0;                    //disable the ADC
    EICRA = _BV(ISC01);            //configure INT0 to trigger on falling edge
    EIFR = _BV(INTF0);             //ensure interrupt flag cleared
    EIMSK = _BV(INT0);             //enable INT0
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        wdtInterrupt = false;
        extInterrupt = false;
        sleep_enable();
        sleep_bod_disable();       //disable brown-out detection (saves 20-25µA)
    }
    sleep_cpu();                   //go to sleep
    sleep_disable();               //wake up here
    ADCSRA = adcsra;               //restore ADCSRA
}

//external interrupt 0 wakes the MCU
ISR(INT0_vect)
{
    EIMSK = 0;                     //disable external interrupts (only need one to wake up)
    extInterrupt = true;
}


//handles the Watchdog Time-out Interrupt
ISR(WDT_vect)
{
    wdtInterrupt = true;
}

///////////////////////////////// AGGIUNTO //////////////

void sendRadio(){
  digitalWrite(pinPTT, HIGH); // turns on the TX module
  delay(100);                 // wait some milliseconds
  MORSE.mm_send((uint8_t *)bufferTx,sizeof(bufferTx)); // TXs
  // MORSE.mm_txNoTones();    // removes modulation tones
  digitalWrite(pinPTT, LOW);  // turns off the TX module
}
