#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <SD.h>

// =============================================================================================================
// --- Mapeamento de Hardware ---
#define ONE_WIRE_BUS 2

#define bt_r 10  //botão direita
#define bt_l 9  //botão esquerda
#define bt_e 8 //botão enter
#define bt_b 7 //botão voltar

//Sensores de temperatura: Endereços unicos 64-bit atribuidos a cada de fabrica

uint8_t sensor1[8] = { 0x28, 0x19, 0x00, 0x00, 0xC8, 0xEE, 0x00, 0x0C };
uint8_t sensor2[8] = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0x09, 0x81, 0xA4 };
uint8_t sensor3[8] = { 0x28, 0xFF, 0x64, 0x18, 0x98, 0x68, 0xDF, 0x55 };
uint8_t sensor4[8] = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0x3D, 0xA5, 0xF0 };

uint8_t* sensores[] = {sensor1, sensor2, sensor3, sensor4};

// =============================================================================================================
// --- Constantes e Objetos ---
#define NUMERO_BOTOES 4
#define MENU_MAX 6 //número máximo de menus existentes
// Código para inicialização do display sem módulo I2C
/*
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
*/
// Inicialização do display com módulo I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Inicialização de objetos OneWire para sensores de temperatura
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);	
DeviceAddress Thermometer;

// Para cartão SD
File myFile;

int intervaloColetaTemperaturaMinutos = 5; // tempo de intervalo a cada temperatura em minutos

// Variáveis de temporizador
unsigned long timerStart;
unsigned long timerIntervalo = 5 ; //em milisegundos
unsigned long tempoAtual;
unsigned long ultimaLeituraTemperatura = 0;

// Variaveis de botoes para debounce
int leituraAnteriorDebounce[NUMERO_BOTOES] = {LOW,LOW,LOW,LOW};
int leituraAtualDebounce[NUMERO_BOTOES] = {LOW,LOW,LOW,LOW};

bool coletaIniciada = false;

int sensoresAtivos = 4;
// String nomeArquivoSD = "captura1.txt";

// =============================================================================================================
// --- Protótipo das Funções ---
void capturaBotao();
void menu1();
void menu2();
void menu3();
void menu4();
void menu5();
void menu6();
// void menu7();
bool debounce(int pin, int posicaoBotao, int intervaloDebouncing);
void esperaTempo(int milisegundos);
void printAddress(DeviceAddress deviceAddress);
void escreveSD(int sensor);
void lerSD();

// =============================================================================================================
// --- Variáveis Globais ---
int menu_num = 1, sub_menu = 1;

// =============================================================================================================
// --- Configurações Iniciais ---
void setup()
{
  sensors.begin();
  Serial.begin(9600);
  pinMode(bt_r, INPUT_PULLUP);
  pinMode(bt_l, INPUT_PULLUP);
  pinMode(bt_e, INPUT_PULLUP);
  pinMode(bt_b, INPUT_PULLUP);

  // Imprime quantidade de sensores de temperatura
  int deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" devices.");
  Serial.println("");

  // Imprime endereços de sensores
  Serial.println("Printing addresses...");
  for (int i = 0;  i < deviceCount;  i++)
  {
    Serial.print("Sensor ");
    Serial.print(i+1);
    Serial.print(" : ");
    sensors.getAddress(Thermometer, i);
    printAddress(Thermometer);
  }

  if (!SD.begin(4)) {
    Serial.println("Falha em inicializar cartão de memória");
    while (1);
  }else
  {
    Serial.println("Cartão de memória positivo e operante.");  
  } 

  // Inicializa timer
  timerStart = millis();
  lcd.begin();  
} //end setup


void printAddress(DeviceAddress deviceAddress)
{ 
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}

/* Captura dados de temperatura a cada intervalo em minutos especificada e insere no cartão SD
*/
void capturaDados(){
  if (coletaIniciada){
    unsigned long tempoDecorridoColeta = millis() - ultimaLeituraTemperatura;
    if(tempoDecorridoColeta > (unsigned int) intervaloColetaTemperaturaMinutos*60*1000){
      sensors.requestTemperatures();
      for (int i = 0; i < sensoresAtivos; i++)
      {
        escreveSD(i);
      }
      ultimaLeituraTemperatura = millis();
    }
  }
}

// =============================================================================================================
// --- Configurações Iniciais ---
void loop()
{
  tempoAtual = millis();
  capturaDados();
  capturaBotao();

  switch (menu_num)
  {
  case 1:
    menu1();
    break;
  case 2:
    menu2();
    break;
  case 3:
    menu3();
    break;
  case 4:
    menu4();
    break;
  case 5:
    menu5();
    break;
  case 6:
    menu6();
    break;
  // case 7:
  //   menu7();
  //   break;
  } //end switch

} //end loop

// =============================================================================================================
// --- Captura botão pressionado ---
void capturaBotao()
{
  if (debounce(bt_r, 0, timerIntervalo) && sub_menu == 1)
  {
    Serial.print("Direita \n");
    if (menu_num <= MENU_MAX-1)
    {
      menu_num += 1;
    }
  }
  if (debounce(bt_l, 1, timerIntervalo) && sub_menu == 1)
  {
    Serial.print("Esquerda \n");
    if (menu_num > 1)
      menu_num -= 1;
  }

  if (debounce(bt_e, 2, timerIntervalo))
  {
    Serial.print("Enter \n");
    if (sub_menu <= 2)
      sub_menu += 1;
  }
  if (debounce(bt_b, 3, timerIntervalo))
  {
    Serial.print("Back \n");
    if (sub_menu > 1)
      sub_menu -= 1;
  }
}

// Muda Numeros de variaveis no menu
void keyboardVariable(int *entrada)
{
  //  while(true){

  if (!digitalRead(bt_r))
  {
    esperaTempo(150);
    *entrada += 1;

  } //end bt_r

  if (!digitalRead(bt_l))
  {
    esperaTempo(150);
    if (*entrada >= 0)
      *entrada -= 1;
  } //end bt_l
}
// ====================================================================================
// --- MENUS

void menu1()
{
  switch (sub_menu)
  {
  case 1:
    lcd.setCursor(0, 0);
    lcd.print("  Temp. Atual  >");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case 2:
    sensors.requestTemperatures();
    lcd.setCursor(0, 0);
    // lcd.print("  Temperatura   ");
    // lcd.print(sensors.getTempCByIndex(0));
    lcd.print(sensors.getTempC(sensor1));    
    lcd.print((char)223);
    lcd.print("C  ");

    lcd.print(sensors.getTempC(sensor2));
    lcd.print((char)223);
    lcd.print("C");

    lcd.setCursor(0, 1);
    // lcd.print("    ");    
    lcd.print(sensors.getTempC(sensor3));
    lcd.print((char)223);
    lcd.print("C  ");

    lcd.print(sensors.getTempC(sensor4));
    lcd.print((char)223);
    lcd.print("C");

    break;
  }
} //end menu1

void menu2()
{
  switch (sub_menu)
  {
  case 1:
    lcd.setCursor(0, 0);
    lcd.print("<Coletar Dados >");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case 2:
    lcd.setCursor(0, 0);
    lcd.print(" Coleta iniciada ");
    lcd.setCursor(0, 1);
    lcd.print("Esc volta");
    coletaIniciada = true;

  }
} //end menu2

void menu3()
{
  switch (sub_menu)
  {
  case 1:
    lcd.setCursor(0, 0);
    lcd.print("<  Ler dados   >");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case 2:
    lcd.setCursor(0, 0);
    lcd.print("Verifique monitor");
    lcd.setCursor(0, 1);
    lcd.print("              ");
    lerSD();
    // esperaTempo(1000);
    sub_menu = 1; //volta ao menu apos leitura
    break;
  }
} //end menu3

void menu4()
{
  switch (sub_menu)
  {
  case 1:
    lcd.setCursor(0, 0);
    lcd.print("<  Intervalo   >");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case 2:
    lcd.setCursor(0, 0);
    lcd.print("Defina intervalo");
    lcd.setCursor(0, 1);
    lcd.print(intervaloColetaTemperaturaMinutos);
    lcd.print("min");
    keyboardVariable(&intervaloColetaTemperaturaMinutos);
    break;
  }  
} //end menu4


void menu5()
{
  switch (sub_menu)
  {
  case 1:
    lcd.setCursor(0, 0);
    lcd.print("<Num. Sensores >");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case 2:
    lcd.setCursor(0, 0);
    lcd.print("Defina a qtd.   ");
    lcd.setCursor(0, 1);
    lcd.print(sensoresAtivos);
    lcd.print(" sensores");
    keyboardVariable(&sensoresAtivos);
    break;
  }  
} //end menu5


void menu6()
{  
  switch (sub_menu)
  {
  case 1:
    lcd.setCursor(0, 0);
    lcd.print("<Deletar captura  ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case 2:
    lcd.setCursor(0, 0);
    lcd.print("Del:");
    lcd.print("captura1.txt");
    lcd.setCursor(0, 1);
    lcd.print("Press. enter");

      if (debounce(bt_e, 2, timerIntervalo))
      {
        lcd.setCursor(0, 0);
        lcd.print("Deletar arquivo?");      
        lcd.setCursor(0, 1);
        lcd.print("                       ");
        if(SD.exists("captura1.txt")){
          SD.remove("captura1.txt");
          lcd.setCursor(0, 0);
          lcd.print("Captura Deletada");
        }else
        {
          lcd.setCursor(0, 0);
          lcd.print("Captura inexistente");
        }
        lcd.setCursor(0, 1);
        lcd.print("                       ");      
        esperaTempo(2000);
        lcd.setCursor(0, 1);
        lcd.print("                       ");
        lcd.print("                ");      
        sub_menu = 1;
      }

    break;
    
  }
  
} //end menu6




/*

void menu7()
{
  switch (sub_menu)
  {
  case 1:
    lcd.setCursor(0, 0);
    lcd.print("<Mudar Arquivo >");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case 2:
    int numeroArquivo = 1;
    // "captura1.txt"
    lcd.setCursor(0, 0);
    lcd.print("Numero arquivo");
    lcd.setCursor(0, 1);
    lcd.print(numeroArquivo);
    keyboardVariable(&numeroArquivo);
    // char buffer[50];
    // nomeArquivoSD = "captura" + (String)numeroArquivo + ".txt";
    // sprintf(buffer, "captura%d.txt", numeroArquivo);

    break;
  }  
} //end menu7

*/


// =============================================================================================================
//DEBOUNCE
/*
  * A função de debounce serve para descobrir se um botão foi pressionado
  * 
  * @param pin O pino, ou botão a esperar
  * @param posicaoBotao posicao do botao nos array leituraAnteriorDebounce e leituraAtualDebounce
  * @param intervaloDebouncing a quantidade de tempo a esperar desde o pressionar do botão
  * @return true caso tenha detectado botao pressionado false caso contrario
*/
bool debounce(int pin, int posicaoBotao, int intervaloDebouncing)
{
  int novoValor;
  leituraAnteriorDebounce[posicaoBotao] = leituraAtualDebounce[posicaoBotao];
  novoValor = digitalRead(pin);
  if (novoValor != leituraAtualDebounce[posicaoBotao])
  {
    esperaTempo(intervaloDebouncing);
    novoValor = digitalRead(pin);
    if (novoValor != leituraAtualDebounce[posicaoBotao])
    {
      leituraAtualDebounce[posicaoBotao] = novoValor;
    }
  }
  if ((leituraAnteriorDebounce[posicaoBotao] == HIGH) && (leituraAtualDebounce[posicaoBotao]) == LOW)
  {
    return true;
  }
  return false;
}

// =============================================================================================================
//SD
void escreveSD(int sensor){  
  // Serial.print("Iniciando escrita no cartão...");
  myFile = SD.open("captura1.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Escrevendo no cartão... ");
    myFile.print("termometro ");
    myFile.print(sensor + 1);
    myFile.print(" ");
    myFile.println(sensors.getTempC(sensores[sensor]));
    // close the file:
    myFile.close();
    Serial.println("Feito!");
  } else {
    // if the file didn't open, print an error:
    Serial.print("erro ao abrir arquivo ");
    Serial.println("captura1.txt");
  }
}

void lerSD(){
    // re-open the file for reading:
  myFile = SD.open("captura1.txt");
  if (myFile) {
    Serial.println("captura1.txt");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.print("erro ao abrir arquivo ");
    Serial.println("captura1.txt");
  }
}

// O mesmo que delay(milisegundos); porém sem congelar processo
void esperaTempo(int milisegundos)
{      
  unsigned long agora = millis();
  while(millis() < agora + milisegundos){}
}