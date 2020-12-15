
#include <VirtualWire.h>
 
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

unsigned long arrayValuesA[10];         // array per gli ultime 10 ricezioni ricevute via radio (diffContatoreA)
unsigned long arrayValuesB[10];
unsigned long arrayTiming[10];
unsigned long arrayTimingB[10];

unsigned long minutoTempo;

boolean flagNoRadioSignal = false;

void setup()
{
  vw_set_rx_pin(receive_pin);
  vw_setup(2000);	 // Bits per sec
  vw_rx_start();       // Start the receiver PLL running
}

void loop() {
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  // se in trenta secondi non riceve nulla
  if ((30000 + minutoTempo) < millis()) {
    minutoTempo = millis();
    flagNoRadioSignal = true;
    //---------------
    // noRadioSignal (true) quando sono passati più di trenta secondi
    //                      dall'ultima ricezione
    //---------------

    //---------------
    // getRcvdRadioSignal() ogniqualvolta riceve un segnale radio diventa TRUE deve essere messo a false manualmente con
    // setRcvdRadioSignal(false) quando viene registrato l'evento 
    //----------------

    // getRawValueA (valori disponibili solo quando getRcvdRadioSignal== TRUE
    // getRawValueB

    // getWattA valori disponibili solo quando getRcvdRadioSignal== TRUE
    // getWattB 
    // se la potenza è maggiore di 9999 restituisce comunque 9999

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
          // --------------------
          // noRadioSignal(false)
          // --------------------
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
        //--------------------
        // getRawValueA
        // getRawValueB
        // -------------------
        //Serial.println("A"+radioContatoreA);
        //Serial.println("B"+radioContatoreB);

        // differenze con i valori precedenti dei due contatori
        diffContatoreA = radioContatoreA - radioCOntatoreAprev;
        diffContatoreB = radioContatoreB - radioContatoreBprev;

        tempoPrec = tempo;
        tempo = micros();

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
  float ff = truncate(watt, 0);
  unsigned long tt = ff;

  if (tt <= 9999UL) {
    // getWatt
  } else {
    // getWatt
  }
}

void addForDiagramA(unsigned long value, unsigned long time) {
  // spostamento array
  for (byte i = 0; i < 10; i++) {
    arrayValuesA[i] = arrayValuesA[i + 1];
    arrayTiming[i] = arrayTiming[i + 1];
  }
  // aggiunta dei valori
  arrayValuesA[9] = value;
  arrayTiming[9] = time;

  // calcolo dell'integrale
  // tra l'ultimo e il primo valore
  sum = 0;
  for (byte i = 0; i < 10; i++) {
    sum += arrayValuesA[i];
  }

  // differenza del tempo tra l'ultimo e il primo
  tempoDiff = arrayTiming[9] - arrayTiming[0];

  // calcolo secondi
  secondi = tempoDiff / 1000000;
  // calcolo watt
  watt = sum / secondi * 3600;

  printWatt(10, 1);

}

void addForDiagramB(unsigned long value, unsigned long time) {
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

  printWatt(10, 0);
}
