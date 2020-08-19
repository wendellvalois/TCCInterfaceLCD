#include <arduino.h>
#include <EEPROM.h>
#include <stdint.h>

int NUMEROLIMITECONTAGENS = 5; // Número de contagens realizadas para cada interrupção
int numContagens = 0;
boolean volatile flagIRQ = false;

unsigned long int IRQcount;
int pin = 2; // Pino que receberá a interrupção

unsigned long time_now = 0; // Tempo atual subtraido pela contagem prévia
unsigned long time_prev = micros(); // Tempo prévio

bool print_eeprom = true; // Condição para impressão da memória EEPROM
int endereco = 1; // Endereco atual para escrever na memória EEPROM

void setup() {
  Serial.begin (9600);
  attachInterrupt(digitalPinToInterrupt(pin), IRQcounter, HIGH);
}

void IRQcounter() {
   flagIRQ = true; 
}

void armazenarTempo(unsigned long valor) {
  byte b[4];
  b[0]= (int) valor;
  b[1]= (int) (valor>>8);
  b[2]= (int) (valor>>16);
  b[3]= (int) (valor>>24);
  for (int i=0 ; i< 4; i++) {
    EEPROM.write(endereco, b[i]);
    endereco++;
  }
}

void lerTempo(){
  byte b[4];
  unsigned long valorMedia = 0;
  for (int i=0; i<5; i++) {
    unsigned long valor = 0;
    for (int j=1; j<5; j++) {
      b[j-1] = EEPROM.read((i*4)+j);
    }
    valor += (unsigned long)b[3] << 24;
    valor += (unsigned long)b[2] << 16;
    valor += (unsigned long)b[1] << 8;
    valor += (unsigned long)b[0];
    Serial.print("EEPROM LEITURA CONTAGEM ");
    Serial.print(i+1);
    Serial.println(":");
    Serial.println(valor);
    if (i != 0) valorMedia += valor;
  }
  Serial.println("_________________________________");
  Serial.println("MEDIA DE VALORES DA LEITURA: ");
  Serial.println(valorMedia/4);
}

void loop() {
  if (flagIRQ) {
      time_now = micros() - time_prev;
      time_prev = micros();
      armazenarTempo(time_now);
      numContagens++;
      flagIRQ = false;
  }
  
  if (numContagens == NUMEROLIMITECONTAGENS && print_eeprom) {
     print_eeprom = false;
     lerTempo();
  }
}