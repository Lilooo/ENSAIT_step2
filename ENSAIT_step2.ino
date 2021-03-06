// Maurin Donneaud For Lucette Board
// Modified by Alice Giordani for ENSAIT e-textile workshops
// upload with "Arduino Pro ou Pro Mini"
// Atmega328 (3.3V, 8MHz)
// TLC 5940

#include "Tlc5940.h"
#include "tlc_fades.h"

#define  BAUDRATE         38400    // vitesse du port serie
#define  DATA             512      // nombre de d'octets par trame

#define  FOOTER_DATA      255      // flag (/) to stop recording incoming bytes
#define  FOOTER_TEMPO     127      // flag ( ) to stop recording incoming bytes

#define  LED              16       // nombre de LEDs
#define  FRAME            16       // nombre de frames
#define  XERR             2        // pin to sens TLC error (PD2)
#define  ledPin           5        // pin to display TLC error

#define  ULTRA_BRIGHT     4095     // valeur maximale intensité LED
#define  BRIGHT           3069     // valeur intermédiaire intensité LED
#define  LESS_BRIGHT      2046     // valeur moitié intensité LED
#define  MIN_BRIGHT       1023     // valeur minimale intensité LED
#define  OFF              0        // LED éteinte

byte serialData[DATA] = {0};
 
int index = 0;
 
int seq[LED][FRAME] = {
 { ULTRA_BRIGHT,BRIGHT,LESS_BRIGHT,MIN_BRIGHT,0,MIN_BRIGHT,LESS_BRIGHT,BRIGHT,ULTRA_BRIGHT,OFF,OFF,OFF,OFF,OFF,OFF,OFF },
 { OFF,BRIGHT,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF },
 { OFF,OFF,LESS_BRIGHT,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF },
 { OFF,OFF,OFF,MIN_BRIGHT,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,MIN_BRIGHT,OFF,OFF },
 { OFF,OFF,OFF,OFF,MIN_BRIGHT,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF },
 { OFF,OFF,OFF,OFF,OFF,MIN_BRIGHT,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF },
 { OFF,OFF,OFF,OFF,OFF,OFF,MIN_BRIGHT,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF },
 { OFF,OFF,OFF,OFF,OFF,OFF,OFF,MIN_BRIGHT,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF },
 { OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF },
 { OFF,OFF,MIN_BRIGHT,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF },
 { OFF,OFF,OFF,OFF,OFF,OFF,MIN_BRIGHT,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF },
 { OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,MIN_BRIGHT,MIN_BRIGHT,MIN_BRIGHT,MIN_BRIGHT,OFF,OFF },
 { OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,MIN_BRIGHT,OFF,OFF },
 { OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,MIN_BRIGHT,OFF },
 { OFF,OFF,OFF,MIN_BRIGHT,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF },
 { OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,OFF,MIN_BRIGHT,MIN_BRIGHT,MIN_BRIGHT,OFF,OFF,OFF }
};
 

int bpm = 300;
float timeFrame = 100;
unsigned long lastMillis = 0;
unsigned int frameIndex = 0;

boolean ledError = false;
boolean lastLedError = false;
boolean ledState = false;
boolean DEBUG = false;

/////////////////////// INITIALISATION
void setup(){
  Serial.begin(BAUDRATE);    // initialize serial
  pinMode(XERR, INPUT);      //
  pinMode(ledPin, OUTPUT);   //
  digitalWrite(ledPin, LOW); //
  Tlc.init();
}

/////////////////////// BOUCLE PRINCIPALE
void loop(){
  ledUpdate();
}

/////////////////////// Les Fonctions
void serialEvent(){
  byte inputValue = 0;
  int highBit = 0;
  int lowBit = 0;
  int ledId = 0;
  int seqPos = 0;
  int seqValue = 0;

  if (Serial.available() > 0){
    inputValue = Serial.read();

    if(inputValue == FOOTER_DATA){
      for (int i=0; i<DATA; i=i+2){
        highBit = (int)serialData[i] << 6;
        lowBit = serialData[i+1];
        seqValue = highBit + lowBit;
        ledId = (int)i/32;
        seqPos = ((int)i/2) % 16;
        seq[ledId][seqPos] = seqValue;
      }
      index = 0;
    }
    else if(inputValue == FOOTER_TEMPO){
      highBit = (int)serialData[1] << 6;
      lowBit = serialData[0];
      bpm = highBit + lowBit;
      timeFrame = 60000 / bpm;
      index = 0;
    }
    else {
      serialData[index] = inputValue;
      index++;
    }
  }
}

////////////////////////////////////// Parser to read serial datas from an [2][16] array of bytes
void ledUpdate(){


  if (millis() - lastMillis >= timeFrame){    
    lastMillis = millis();

    for (int ledIndex=0; ledIndex<LED; ledIndex++){
      Tlc.set(ledIndex, seq[ledIndex][frameIndex]);
    }
    Tlc.update();
    /////////// CATCH ERROR
    lastLedError = ledError;
    ledError = digitalRead(XERR); // XERR is ERROR pin
    if( ledError == true && ledError != lastLedError ){
      digitalWrite(ledPin, HIGH);
    }
    else{
      digitalWrite(ledPin, LOW);
    }
    frameIndex++;
    frameIndex = frameIndex % FRAME;

  }
}
