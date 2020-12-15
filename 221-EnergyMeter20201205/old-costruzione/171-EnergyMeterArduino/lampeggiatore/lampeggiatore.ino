/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/

// the setup function runs once when you press reset or power the board
const int pinled1 = 13;
const int pinled2 = 10;
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(pinled1, OUTPUT);
  pinMode(pinled2, OUTPUT);
  pinMode(7, INPUT_PULLUP);
  digitalWrite(pinled1, LOW);
  digitalWrite(pinled2, LOW);
}

// the loop function runs over and over again forever
void loop() {
//  int qualeled;
//  int duratadelay;
//  qualeled = random(1,10);
//  duratadelay = random(100,300);
//  if (qualeled<6){
//    digitalWrite(pinled1, HIGH);   // turn the LED on (HIGH is the voltage level)
//    delay(10);                       // wait for a second
//    digitalWrite(pinled1, LOW);    // turn the LED off by making the voltage LOW
//    delay(duratadelay);                       // wait for a second   
//  } else {
//    digitalWrite(pinled2, HIGH);   // turn the LED on (HIGH is the voltage level)
//    delay(10);                       // wait for a second
//    digitalWrite(pinled2, LOW);    // turn the LED off by making the voltage LOW
//    delay(duratadelay);                       // wait for a second    
//  }

for (int n=1; n< 11; n++){
    digitalWrite(pinled2, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(10);                       // wait for a second
    digitalWrite(pinled2, LOW);    // turn the LED off by making the voltage LOW
    delay(300); 
}

for (int n=1; n< 11; n++){
    digitalWrite(pinled1, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(10);                       // wait for a second
    digitalWrite(pinled1, LOW);    // turn the LED off by making the voltage LOW
    delay(300); 
}

delay(30000);
delay(30000);
delay(30000);
delay(30000);
delay(30000);
delay(30000);


//if (digitalRead(7)==HIGH){
//  digitalWrite(pinled1, HIGH);
//} else {
//  digitalWrite(pinled1, LOW);
//}

delay(100);

}
