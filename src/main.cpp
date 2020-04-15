#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <dht.h>
#include <EEPROM.h>

dht DHT;

// =============================================================================================================
// --- Mapeamento de Hardware ---
#define bt_r 8  //botão direita
#define bt_l 9  //botão esquerda
#define bt_e 10 //botão enter
#define bt_b 11 //botão voltar
#define DHT11_PIN 5
#define NUMERO_BOTOES

// =============================================================================================================
// --- Constantes e Objetos ---
#define menu_max 5 //número máximo de menus existentes
// Inicialização do display sem módulo I2C
/*
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
*/
// Inicialização do display com módulo I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

int intervalo = 5;
int enderecomMem;

// Variáveis de temporizador
unsigned long timerStart;
unsigned long timerIntervalo;
unsigned long tempoAtual;

// Button debounce variables
int leituraAnteriorDebounce[NUMERO_BOTOES] = {LOW,LOW,LOW,LOW}; // para todos os botões usados
int leituraAtualDebounce[NUMERO_BOTOES] = {LOW,LOW,LOW,LOW};

// =============================================================================================================
// --- Protótipo das Funções ---
void capturaBotao();
void menu1();
void menu2();
void menu3();
void menu4();
void menu5();
byte lerEEPROMByte(int endereco);
void escreveEEPROMByte(int endereco, byte valor);
void limpaEEPROM();

// =============================================================================================================
// --- Variáveis Globais ---
int menu_num = 1, sub_menu = 1;

/*
  * A função de debounce serve para descobrir se um botão foi pressionado
  * 
  * @param pin : O pino, ou botão a esperar
  * @param estadoBotao : o estado anterior do botao
  * @param intervalo : a quantidade de tempo a esperar desde o pressionar do botão
  * @return novoValor : valor atual do botão
*/
bool debounce(int pin, int posicaoBotao, int intervaloDebouncing)
{
  int novoValor;
  leituraAnteriorDebounce[posicaoBotao] = leituraAtualDebounce[posicaoBotao];
  novoValor = digitalRead(pin);
  if (novoValor != leituraAtualDebounce[posicaoBotao])
  {
    delay(intervaloDebouncing);
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
// --- Configurações Iniciais ---
void setup()
{
  pinMode(bt_r, INPUT_PULLUP);
  pinMode(bt_l, INPUT_PULLUP);
  pinMode(bt_e, INPUT_PULLUP);
  pinMode(bt_b, INPUT_PULLUP);

  // Inicializa timer
  timerStart = millis();
  timerIntervalo = 250;

  lcd.begin();
  Serial.begin(9600);
} //end setup

// =============================================================================================================
// --- Configurações Iniciais ---
void loop()
{
  tempoAtual = millis();
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
  } //end switch

} //end loop

// =============================================================================================================
// --- Captura botão pressionado ---
void capturaBotao()
{
  if (debounce(bt_r, 0, intervalo) && sub_menu == 1)
  {
    Serial.print("Direita \n");
    if (menu_num <= menu_max-1)
    {
      menu_num += 1;
    }
  }
  if (debounce(bt_l, 1, intervalo) && sub_menu == 1)
  {
    Serial.print("Esquerda \n");
    if (menu_num > 1)
      menu_num -= 1;
  }

  if (debounce(bt_e, 2, intervalo))
  {
    Serial.print("Enter \n");
    if (sub_menu <= 2)
      sub_menu += 1;
  }
  if (debounce(bt_b, 3, intervalo))
  {
    Serial.print("Back \n");
    if (sub_menu > 1)
      sub_menu -= 1;
  }
}


// Muda Numeros no menu
void keyboardVariable(int *entrada)
{
  //  while(true){

  if (!digitalRead(bt_r))
  {
    delay(150);
    *entrada += 1;

  } //end bt_r

  if (!digitalRead(bt_l))
  {
    delay(150);
    if (*entrada >= 0)
      *entrada -= 1;
  } //end bt_l
}

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
    DHT.read11(DHT11_PIN);
    lcd.setCursor(0, 0);
    lcd.print("  Temperatura   ");
    lcd.setCursor(0, 1);
    lcd.print("    ");
    lcd.print(DHT.temperature);
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
    lcd.print("Segurar Esc para");
    //modificar essa parte para armazenamento
    //  escreveEEPROMByte(endereco, DHT.humidity);
    enderecomMem += 1;
    //O codigo abaixo faz esperar pelo tempo estabelecido na variavel "intervalo"
    //O usuario pode quebrar a espera segurando esc a qualquer momento
    //sem ter que esperar varios minutos, evitando usar threads
    for (int i; i < intervalo * 60; i++)
    {
      delay(1000);
      if (!digitalRead(bt_b))
      {
        sub_menu = 1;
        break;
      }
    }
    break;
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
    for (size_t i = 0; i < EEPROM.length(); i++)
    {
      Serial.println(lerEEPROMByte(i));
    }
    delay(1000);
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
    lcd.print(intervalo);
    lcd.print("min");
    keyboardVariable(&intervalo);
    break;
  }  
} //end menu4

void menu5()
{
  switch (sub_menu)
  {
  case 1:
    lcd.setCursor(0, 0);
    lcd.print("< Limpar EEPROM  ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    break;
  case 2:
    lcd.setCursor(0, 0);
    lcd.print("   Certeza?   ");
    lcd.setCursor(0, 1);
    lcd.print("Press. enter confirma");

    if (!digitalRead(bt_e))
    {
      limpaEEPROM();
      lcd.setCursor(0, 0);
      lcd.print("                       ");
      lcd.print("EEPROM Resetada");
      lcd.setCursor(0, 1);
      lcd.print("                       ");
      lcd.print("                ");
      delay(1000);
      sub_menu = 1;
    }
    break;
  }
} //end menu5

// =============================================================================================================
//EEPROM

byte lerEEPROMByte(int endereco) //byte: valores de -128 a 127
{
  byte hiByte = EEPROM.read(endereco);
  return hiByte;
}

void limpaEEPROM()
{
  for (unsigned int i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i, 0);
  }
}

void escreveEEPROMByte(int endereco, byte valor) //byte: valores de -128 a 127
{
  EEPROM.write(endereco, valor);
}
