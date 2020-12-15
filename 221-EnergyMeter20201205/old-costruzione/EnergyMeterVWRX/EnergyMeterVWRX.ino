// receiver.pde
//
// Simple example of how to use VirtualWire to receive messages
// Implements a simplex (one-way) receiver with an Rx-B1 module
//
// See VirtualWire.h for detailed API docs
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2008 Mike McCauley
// $Id: receiver.pde,v 1.3 2009/03/30 00:07:24 mikem Exp $

#include <VirtualWire.h>
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int led_pin = 13;
//const int transmit_pin = 12;
const int receive_pin = 2;
//const int transmit_en_pin = 3;
byte contatore = 0;

unsigned long contatoreA = 0;
unsigned long contatoreAprev = 0;
unsigned long contatoreB = 0;
unsigned long contatoreBprev = 0;
unsigned long differenzaA = 0;
unsigned long differenzaB = 0;
unsigned long tempo=0;
unsigned long tempoPrec=0;
unsigned long tempoDiff=0;


void setup()
{
  delay(1000);

   lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
//  Serial.begin(57600);	// Debugging only
//  Serial.print("INizio");
  vw_set_rx_pin(receive_pin);
  vw_setup(2000);	 // Bits per sec
  vw_rx_start();       // Start the receiver PLL running

  pinMode(led_pin, OUTPUT);

 
}

void loop()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    int i;

    digitalWrite(led_pin, HIGH); // Flash a light to show received good message
    
    if (char(buf[0]) == 'C') { 
      contatore++;          
      if (char(buf[1]) == 'A') {
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        for (i = 0; i < buflen; i++) {
          if (char(buf[i]) != 0) {
            lcd.print(char(buf[i]));
          } else {
            lcd.print(" ");
          }
        }

        contatoreAprev=contatoreA;
        contatoreBprev=contatoreB;

        // la ricezione è composta 
        // correttamente da 10 caratteri
        char ls[5];
        for (i=2; i<6;i++){
          ls[i-2]=buf[i];
        }
        ls[4]=0;
        contatoreA=atol(ls);
        
        char lt[5];
        for (i=6; i<10;i++){
          lt[i-6]=buf[i];
        }
        lt[4]=0;
        contatoreB=atol(lt);

        differenzaA=contatoreA-contatoreAprev;
        differenzaB=contatoreB-contatoreBprev; 

        tempoPrec=tempo;
        tempo=micros();

        tempoDiff = tempo-tempoPrec; // microsecondi

        float t = tempoDiff/1000000;
        t = differenzaA/t*3600;

        lcd.setCursor(0, 0);
        lcd.print("                ");
        lcd.setCursor(0, 0);
        lcd.print(t);

        t = t = tempoDiff/1000000;
        t = differenzaB/t*3600;
        
        lcd.setCursor(8, 0);
        lcd.print(t);
        
        
      } 
    }


    //Serial.println();
    lcd.setCursor(12, 1);
    lcd.print(contatore);
    digitalWrite(led_pin, LOW);
  }
}
