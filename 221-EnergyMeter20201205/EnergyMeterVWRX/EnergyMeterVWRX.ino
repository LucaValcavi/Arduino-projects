
#include <VirtualWire.h>
#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int receive_pin = 2;
byte nrRicezioni = 0;

unsigned long radioContatoreA = 0;     // valore del contatoreA ricevuto via radio
unsigned long radioCOntatoreAprev = 0; // valoe precedente
unsigned long radioContatoreB = 0;     // valore del contatoreB ricevuto via radio
unsigned long radioContatoreBprev = 0; // valore precedente

unsigned long sum = 0;                 // variabile generica di somma

unsigned long diffContatoreA = 0;      // contiene la differenza tra il radioContatoreA e radioCOntatoreAprev
unsigned long diffContatoreB = 0;      // contiene la differenza tra il radioContatoreB e radioCOntatoreBprev

unsigned long tempo = 0;               // variabile generica che contiene i microsecondi  
unsigned long tempoPrec = 0;		   // valore precedente della variabile tempo
unsigned long tempoDiff = 0;		   // differenza tra tempo e tempoPrec

float secondi = 0.0f;				   // variabile generica che contiene secondi (float)
float watt = 0.0f;                     // variabile generica che contiene una potenza (float)

unsigned long arrayValues[10];         // array per gli ultime 10 ricezioni ricevute via radio (diffContatoreA)
unsigned long arrayValuesB[10];
unsigned long arrayTiming[10];
unsigned long arrayTimingB[10];

unsigned long minutoTempo;

boolean flagNoRadioSignal = false;

void setup()
{
  delay(1000);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Valk EnergyMeter");

  vw_set_rx_pin(receive_pin);
  vw_setup(2000);	 // Bits per sec
  vw_rx_start();       // Start the receiver PLL running


  byte uno[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
  };
  lcd.createChar(0, uno);

  byte due[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
  };
  lcd.createChar(1, due);

  byte tre[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111,
  };
  lcd.createChar(2, tre);

  byte quattro[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111,
    B11111,
  };
  lcd.createChar(3, quattro);

  byte cinque[8] = {
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
  };
  lcd.createChar(4, cinque);

  byte sei[8] = {
    B00000,
    B00000,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
  };
  lcd.createChar(5, sei);

  byte sette[8] = {
    B00000,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
  };
  lcd.createChar(6, sette);

  byte otto[8] = {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
  };
  lcd.createChar(7, otto);

  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.write(byte(1));
  lcd.write(byte(2));
  lcd.write(byte(3));
  lcd.write(byte(4));
  lcd.write(byte(5));
  lcd.write(byte(6));
  lcd.write(byte(7));

  Serial.begin(115200);
  Serial.print("Valk Energy Meter 20201208");

  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("                ");
}

void loop() {
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  // se in trenta secondi non riceve nulla
  if ((30000 + minutoTempo) < millis()) {
    minutoTempo = millis();
    lcd.setCursor(15, 1);
    lcd.print("?");
    flagNoRadioSignal = true;
  }

  // ==================
  // ricezione messaggi
  // ==================
  if (vw_get_message(buf, &buflen)) {


    if (char(buf[0]) == 'C') {
      if (char(buf[1]) == 'A') {
        // segnale radio ricevuto
        // resetta il segnalatore di mancata ricezione
        minutoTempo = millis();
        if (flagNoRadioSignal) {
          flagNoRadioSignal = false;
          lcd.setCursor(15, 1);
          lcd.print(" ");
        }

        // conta il numero di ricezioni
        // del segnale radio
        nrRicezioni++;
        if (nrRicezioni > 9) {
          nrRicezioni = 0;
        }


        radioCOntatoreAprev = radioContatoreA;
        radioContatoreBprev = radioContatoreB;

        // la ricezione è composta
        // correttamente da 10 caratteri

        // recupero del valore del contatore A
        char ls[5];
        for (byte i  = 2; i < 6; i++) {
          ls[i - 2] = buf[i];
        }
        ls[4] = 0;
        radioContatoreA = atol(ls);



        // recupero del valore del contatore B
        char lt[5];
        for (byte i  = 6; i < 10; i++) {
          lt[i - 6] = buf[i];
        }
        lt[4] = 0;
        radioContatoreB = atol(lt);

        // invio su seriale del valore grezzo dei due contatori
        Serial.println("A"+radioContatoreA);
        Serial.println("B"+radioContatoreA);

        // differenze con i valori precedenti dei due contatori
        diffContatoreA = radioContatoreA - radioCOntatoreAprev;
        diffContatoreB = radioContatoreB - radioContatoreBprev;

        tempoPrec = tempo;
        tempo = micros();


        // tempoDiff = tempo - tempoPrec; // microsecondi

        // secondi = tempoDiff / 1000000;
        // watt = diffContatoreA / secondi * 3600;


        addForDiagramA(diffContatoreA, tempo);


        addForDiagramB(diffContatoreB, tempo);



      }
    }



  }
}


float truncate(float val, byte dec)
{
  float x = val * pow(10, dec);
  float y = round(x);
  float z = x - y;
  if ((int)z == 5)
  {
    y++;
  } else {}
  x = y / pow(10, dec);
  return x;
}

void printWatt(int cursorCol, int CursorRow) {

  //if (watt <= 9999) {

  float ff = truncate(watt, 0);
  unsigned long tt = ff;

  if (tt <= 9999UL) {

    lcd.setCursor(cursorCol, CursorRow);
    lcd.print(tt);
  } else {
    lcd.setCursor(cursorCol, CursorRow);
    lcd.print("----");
  }
  //}

}


void addForDiagramB(unsigned long value, unsigned long time) {

  // spostamento array
  for (byte i = 0; i < 10; i++) {
    arrayValues[i] = arrayValues[i + 1];
    arrayTiming[i] = arrayTiming[i + 1];
  }
  // aggiunta dei valori
  arrayValues[9] = value;
  arrayTiming[9] = time;

  // calcolo dell'integrale
  // tra l'ultimo e il primo valore
  sum = 0;
  for (byte i = 0; i < 10; i++) {
    sum += arrayValues[i];
  }

  // differenza del tempo tra l'ultimo e il primo
  tempoDiff = arrayTiming[9] - arrayTiming[0];

  // calcolo secondi
  secondi = tempoDiff / 1000000;
  // calcolo watt
  watt = sum / secondi * 3600;


  lcd.setCursor(10, 1);
  lcd.print("    ");

  printWatt(10, 1);

  lcd.setCursor(0, 1);

  for (byte i = 0; i < 10; i++) {
    switch (arrayValues[i]) {
      case 0:
        lcd.print(" ");
        break;
      case 1:
        lcd.write(byte(0));
        break;
      case 2:
        lcd.write(byte(1));
        break;
      case 3:
        lcd.write(byte(2));
        break;
      case 4:
        lcd.write(byte(3));
        break;
      case 5:
        lcd.write(byte(4));
        break;
      case 6:
        lcd.write(byte(5));
        break;
      case 7:
        lcd.write(byte(6));
        break;
      default:
        lcd.write(byte(7));
        break;
    }
  }


}

void addForDiagramA(unsigned long value, unsigned long time) {

  // spostamento array
  for (byte i = 0; i < 10; i++) {
    arrayValuesB[i] = arrayValuesB[i + 1];
    arrayTimingB[i] = arrayTimingB[i + 1];
  }
  // aggiunta dei valori
  arrayValuesB[9] = value;
  arrayTimingB[9] = time;

  // calcolo dell'integrale
  // tra l'ultimo e il primo valore
  sum = 0;
  for (byte i = 0; i < 10; i++) {
    sum += arrayValuesB[i];
  }

  // differenza del tempo tra l'ultimo e il primo
  tempoDiff = arrayTimingB[9] - arrayTimingB[0];

  // calcolo secondi
  secondi = tempoDiff / 1000000;
  // calcolo watt
  watt = sum / secondi * 3600;


  lcd.setCursor(10, 0);
  lcd.print("    ");

  printWatt(10, 0);

  lcd.setCursor(0, 0);

  for (byte i = 0; i < 10; i++) {
    switch (arrayValuesB[i]) {
      case 0:
        lcd.print(" ");
        break;
      case 1:
        lcd.write(byte(0));
        break;
      case 2:
        lcd.write(byte(1));
        break;
      case 3:
        lcd.write(byte(2));
        break;
      case 4:
        lcd.write(byte(3));
        break;
      case 5:
        lcd.write(byte(4));
        break;
      case 6:
        lcd.write(byte(5));
        break;
      case 7:
        lcd.write(byte(6));
        break;
      default:
        lcd.write(byte(7));
        break;
    }
  }


}
