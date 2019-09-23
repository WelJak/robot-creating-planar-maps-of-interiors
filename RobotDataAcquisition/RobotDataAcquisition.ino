#define in1A 7
#define in2A 8
#define in1B 5
#define in2B 4
#include <Servo.h>
#include <TFMPlus.h>
TFMPlus tfmP;
#define SSID "#####"
#define PASSWORD "#########"
#define DEBUG 1
uint16_t tfDist;
uint16_t tfFlux;
uint16_t tfTemp;
int dotPosition;
String sendData();
String wysylka;
String  cmd;
int a;
int c;
Servo mojeserwo; //utworzenie obiektu o nazwie mojeserwo
float przebyty;
float korektapoz = 0;
String Skorektapoz;
int skret = 420;
float sumaobrotu = 0;
float sumaobroturad = 0;
String Sdystans;
String Skat;
float katobrotu;
String Skatobrotu;
int enkodery [] = {2, 3}; // pin 2 - enkoder a pin 3 - enkoder b
volatile float impulsy [] = {0, 0}; //impulsy zczytane z enkoderow a-do obliczenia odleglosci podczas jazdy na wprost i b- kontrolowania kątu skrętu
void licznik1() {
  impulsy [0] ++; // funkcja zliczająca impulsy w przypadku wystapenia zbocza opadajacego, dla zmiennej impulsy[0](endkoder a)
}
void licznik2() {
  impulsy[1] ++; // funkcja zliczająca impulsy w przypadku wystapenia zbocza opadajacego, dla zmiennej impulsy[1](endkoder b)
}
void setup() {
  // put your setup code here, to run once:
  c = 0;
  Serial.begin (9600);
  delay(20);
  Serial3.begin( 115200);
  delay(20);
  tfmP.begin( &Serial3);
  pinMode(in1A, OUTPUT);
  pinMode(in2A, OUTPUT);
  pinMode(in1B, OUTPUT);
  pinMode(in2B, OUTPUT);
  pinMode(enkodery [0], INPUT);// przypisanie enkodera a jako wejscie (pin2)
  pinMode(enkodery [1], INPUT);// przypisanie enkodera b jako wejscie (pin3)
  attachInterrupt(0, licznik1, FALLING); // 0 oznacza pin 2; gdy wystapi przerwanie(wykryte zbocze opadajace) zostanie wywołana funkcja licznik1
  attachInterrupt(1, licznik2, FALLING);// podobnie jak wyzej ale 1 oznacza pin 3
  mojeserwo.attach(10); // przypisanie sterowania serwomechanizmu do pinu 10
  //mojeserwo.write(90);
  Serial1.begin(9600);
  sendData("AT+CIOBAUD=9600\r\n", 2000, DEBUG);
  sendData("AT+RST\r\n", 2000, DEBUG);
  cmd = "AT+CWJAP=\"";
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASSWORD;
  cmd += "\"";
  cmd += "\r\n";
  sendData(cmd, 4000, DEBUG);
  delay(2000);
  cmd = "";
  // 1 = Station mode (client)
  //2 = AP mode (host)
  //3 = AP + Station mode
  sendData("AT+CWMODE=1\r\n", 1000, DEBUG);
  //sendData("AT+CIFSR\r\n", 1000, DEBUG);
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);
  sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG);
}

void loop() {
  // put your main code here, to run repeatedly:
  tfmP.getData( tfDist, tfFlux, tfTemp);
  if (tfDist <= 17) {
    a = 0;
    c = 0;
    Hamowanie();
    Stop();
    delay(500);
    impulsy[1] = 0;
    przebyty = 3.14 * 6.5 * impulsy[0];
    przebyty = przebyty / 960;
    delay(100);
    dystans_przejechany(przebyty);
    Serial.println(przebyty);
    for (int i = 1; i <= 180; i+=5) {
      mojeserwo.write(i);
      delay(35);
      tfmP.getData( tfDist, tfFlux, tfTemp);
      // tablica_dystans[i - 15] = tfDist;
      Sdystans = String(tfDist);
      Skat = String(i);
      a = Skat.length();
      switch (a) {
        case (1):
          Skat = "k00" + Skat;
          break;
        case (2):
          Skat = "k0" + Skat;
          break;
        case (3):
          Skat = "k" + Skat;
          break;
      }
      Sdystans = Sdystans + Skat;
      a = Sdystans.length();
      switch (a) {
        case (5):
          Sdystans = "0000" + Sdystans;
          break;
        case (6):
          Sdystans = "000" + Sdystans;
          break;
        case (7):
          Sdystans = "00" + Sdystans;
          break;
      }
      pakiet(Sdystans);

    }
    for (int i = 165; i >= 90; i--) {
      mojeserwo.write(i);
      delay(20);
    }

    while (impulsy[1] <= skret) {
      Wlewo();
    }
    Stop();
    delay(1000);
    katobrotu = impulsy[1] / 9.6;
    Skatobrotu = String(katobrotu);
    dotPosition = Skatobrotu.indexOf('.');
    Skatobrotu.setCharAt(dotPosition, ',');
    a = Skatobrotu.length();
    switch (a) {
      case 5:
        Skatobrotu = "hhho" + Skatobrotu;
        break;
      case 6:
        Skatobrotu = "hho" + Skatobrotu;
        break;
      case 7:
        Skatobrotu = "ho" + Skatobrotu;
        break;
    }
    pakiet(Skatobrotu);
    if (c == 0) {
      sumaobrotu = 90 + (katobrotu / 2);
      c++;
    }
    else {
      sumaobrotu += katobrotu;
    }
    sumaobroturad = (3.14 * sumaobrotu) / 180;
    korektapoz = 144 - 144 * cos(katobrotu); //twierdzenie cosinusow
    korektapoz = sqrt(korektapoz);
    korektapoz = korektapoz * cos(sumaobroturad);
    Skorektapoz = String(korektapoz);
    dotPosition = Skorektapoz.indexOf('.');
    Skorektapoz.setCharAt(dotPosition, ',');
    a = Skorektapoz.length();
    switch (a) {
      case 4:
        Skorektapoz = "hhhhx" + Skorektapoz;
        break;
      case 5:
        Skorektapoz = "hhhx" + Skorektapoz;
        break;
      case 6:
        Skorektapoz = "hhx" + Skorektapoz;
        break;
    }
    pakiet(Skorektapoz);
    korektapoz = korektapoz * sin(sumaobroturad);
    Skorektapoz = String(korektapoz);
    dotPosition = Skorektapoz.indexOf('.');
    Skorektapoz.setCharAt(dotPosition, ',');
    a = Skorektapoz.length();
    switch (a) {
      case 4:
        Skorektapoz = "hhhhy" + Skorektapoz;
        break;
      case 5:
        Skorektapoz = "hhhy" + Skorektapoz;
        break;
      case 6:
        Skorektapoz = "hhy" + Skorektapoz;
        break;
    }
    pakiet(Skorektapoz);
    interrupts();
    impulsy[0] = 0;
    Serial.print(katobrotu);
    Serial.println(" stopni");
  }
  else {
    digitalWrite(in1A, HIGH);
    digitalWrite(in2A, LOW);
    digitalWrite(in1B, HIGH);
    digitalWrite(in2B, LOW);
  }

}


void Stop() {
  digitalWrite(in1A, LOW);
  digitalWrite(in2A, LOW);
  digitalWrite(in1B, LOW);
  digitalWrite(in2B, LOW);
}
void Hamowanie() {
  digitalWrite(in1A, LOW);
  digitalWrite(in2A, HIGH);
  digitalWrite(in1B, LOW);
  digitalWrite(in2B, HIGH);
  delay(10);
}
void Wprawo() {
  digitalWrite(in1A, LOW);
  digitalWrite(in2A, HIGH);
  digitalWrite(in1B, HIGH);
  digitalWrite(in2B, LOW);
}
void Wlewo() {
  digitalWrite(in1A, HIGH);
  digitalWrite(in2A, LOW);
  digitalWrite(in1B, LOW);
  digitalWrite(in2B, HIGH);
}
String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  Serial1.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (Serial1.available())
    {
      // The esp has data so display its output to the serial window
      char c = Serial1.read(); // read the next character.
      response += c;
    }
  }
  if (DEBUG)
  {
    Serial.print(response);
  }
  return response;
}
void pakiet(String el_tablicy) {
  if (Serial1.find("OK")) {
    cmd = "AT+CIPSEND=";
    cmd += "0";
    cmd += ",";
    cmd += "9";
    Serial1.println(cmd);
    Serial.println(cmd);
    while (!Serial1.find(">"))
    {
      delay(500);
      Serial.println("brak polaczenia");
    }
    Serial1.println(el_tablicy);
    Serial.println(el_tablicy);
  }
}

void dystans_przejechany(float el_tablicy) {
  int B;
  if (Serial1.find("OK")) {
    wysylka = String(el_tablicy);
    dotPosition = wysylka.indexOf('.');
    wysylka.setCharAt(dotPosition, ',');
    B = wysylka.length();
    switch (B) {
      case 4:
        wysylka = "p0000" + wysylka;
        break;
      case 5:
        wysylka = "p000" + wysylka;
        break;
      case 6:
        wysylka = "p00" + wysylka;
        break;
      case 7:
        wysylka = "p0" + wysylka;
        break;
    }
    cmd = "AT+CIPSEND=";
    cmd += "0";
    cmd += ",";
    cmd += "9";
    Serial1.println(cmd);
    Serial.println(cmd);
    while (!Serial1.find(">"))
    {
      delay(500);
      Serial.println("brak polaczenia");
    }
    Serial1.println(wysylka);
    Serial.println(wysylka);
  }
}
