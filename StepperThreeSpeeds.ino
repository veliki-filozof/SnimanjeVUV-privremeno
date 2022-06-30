/*
Kontrolor drajvera step motora u VUV spektrometru Jobin Yvon
Pisano za Arduino Nano
Autor: Nikola Goles
Email: golesn9@gmail.com
Datum kreiranja: 09.02.2022.
*/

#include <Stepper.h>

// Definicije pinova
#define DIR_PIN     2
#define STEP_PIN    3

// Globalne varijable
String Komanda = "";
String Brzina = "";
long stp = 0;
int dir = 0; // dir = 1 => DIR_PIN HIGH; skeniranje ka NIZIM TAL DUZ
long cnt = 0;
char bfr[50];
boolean tgl = 0;
boolean tim = 0;

void setup() {
  // Postavke pinova
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  // Postavke tajmera
  cli(); // clear interrupts
  TCCR1A = 0; // init registra
  TCCR1B = 0;
  TCNT1 = 0; // init brojaca
  TCCR1B |= (1<<WGM12); // CTC mode
  TCCR1B |= (1<<CS11) | (1<<CS10); // prescaler 64
  sei(); // enable interrupts

  Serial.begin(9600); // otvori Serial port
}

// Interrupt metoda
ISR (TIMER1_COMPA_vect) {
  finished();
  if (tgl) {
    digitalWrite(STEP_PIN, HIGH);
    tgl = 0;
    cnt++;
    }  
  else {
    digitalWrite(STEP_PIN, LOW);
    tgl = 1;
    }
}

void loop() {
  if (Serial.available() != 0) {
    load(); // ucitaj Serial
    Serial.println("Processing..."); // obavesti o prihvatanju komande
    // Okretanje velikom brzinom
    if (Brzina == "Brzo" and stp != 0) {       
      pravac(); // fix dir pin
      OCR1A = 399; // compare match set
      cnt = 0;
      TIMSK1 |= (1 << OCIE1A); // enable interrupt
    }
    // Okretanje srednjom brzinom
    if (Brzina == "Srednje" and stp != 0) {     
      pravac(); // fix dir pin
      OCR1A = 2999; // compare match set 
      cnt = 0;
      TIMSK1 |= (1 << OCIE1A); // enable interrupt
    }
    // Okretanje malom brzinom
    if (Brzina == "Sporo" and stp != 0) {     
      pravac(); // fix dir pin
      OCR1A = 18999; // compare match set 
      cnt = 0;
      TIMSK1 |= (1 << OCIE1A); // enable interrupt
    }
  }
}

// Metoda za citanje sa Serial porta
// Prihvacen format: "Brzina, dir, brstep"
void load(void) {
  Komanda = Serial.readString(); // citaj Serial
  Komanda.toCharArray(bfr,50); // parse command
  char* cmd;
  int i = 0;
  // Izdvoji iz stringa komande odvojene zapetom
  while(i<3){ 
    cmd = strtok(bfr, ",");
    Brzina = cmd;
    i += 1;
    cmd = strtok(NULL,",");
    dir = atoi(cmd);
    i += 1;
    cmd = strtok(NULL,",");
    stp = atol(cmd);
    i += 1;
  }
}

// Metoda za zaustavljanje tajmera
void finished(void) {
  if (cnt >= stp) {
      TIMSK1 &= ~(1 << OCIE1A); // disable interrupt
      tgl = 0;
      Serial.println("Done!"); // obavesti o zavrsetku komande
 }
}


// Metoda za fiksiranje pravca
void pravac() {
    if (dir == 1) {
        digitalWrite(DIR_PIN, HIGH);
      }
    else {
        digitalWrite(DIR_PIN, LOW);
      }
}
