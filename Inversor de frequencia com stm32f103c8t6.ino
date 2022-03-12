#include <Arduino.h>
#include <SoftwareSerial.h>

HardwareTimer *timer;
HardwareTimer *Gerador_VTCD;


boolean Flag_1, Flag_2 = true;
int Ma, i, frequencia, duty, seno_braco_1;
int seno_braco_2, Ma_nominal, Ma_VTCD, tempo_Tensao_Nominal, Tempo_VTCD, pegar_valor, passar_valor;
double inicio, percorrido, Tempo_Controle, Tempo_Percorrido_2;
double omega = 6.283185;

#define Braco_superior_esquerdo  PB6
#define Braco_inferior_esquerdo  PB7
#define Braco_superior_direito  PB8
#define Braco_inferior_direito  PB9

#define controle_inversor PB5

void Mudanca_comparador();
void Tempo_atualizacao();


void comunicacao() {
  i = 0;
  Ma_nominal = 0;
  digitalWrite(PC13, HIGH);
  Serial.println("Digite o valor inteiro, maior que 0 e em %, do valor do indice da modulação em amplitude: ");
  while (Ma_nominal <= 0) {

    i++;
    if (i >= 2) {
      Serial.println("Digite novamente o valor inteiro, maior que 0 e em %, do valor do indice da modulação em amplitude: ");
    }
    Ma_nominal = Serial.parseInt();
    Serial.println(Ma_nominal);
  }
  Serial.println("Ma nominal: ");
  Serial.print(Ma_nominal);
  Serial.println("\n");
  tempo_Tensao_Nominal = 0;
  i = 0;
  Serial.println("\nDigite o valor, em milissegundo, o tempo de duracao do Ma nominal:");
  while (tempo_Tensao_Nominal <= 0) {

    i++;
    if (i >= 2) {
      Serial.println("Digite novamente o valor inteiro, maior que 0, o tempo de duracao do Ma nominal: ");
      tempo_Tensao_Nominal = 0;
    }
    tempo_Tensao_Nominal = Serial.parseInt();
    Serial.println(tempo_Tensao_Nominal);
  }
  Serial.println("tempo de duracao do Ma nominal: ");
  Serial.println(tempo_Tensao_Nominal);
  Serial.println("\n\n");
  i = 0;
  Ma_VTCD = 0;
  Serial.println("Digite o valor inteiro, maior que 0 e em %, do valor do indice da modulação em amplitude em vtcd: ");
  while (Ma_VTCD <= 0) {


    i++;
    if (i >= 2) {
      Serial.println("Digite novamente o valor inteiro, maior que 0 e em %, do valor do indice da modulação em amplitude em vtcd: ");
    } Ma_VTCD = Serial.parseInt();
    Serial.println(Ma_VTCD);
  }
  i = 0;
  Serial.println("\n\n");
  Serial.println("Ma_VTCD: ");
  Serial.print(Ma_VTCD);
  Serial.println("\n\n");
  Tempo_VTCD = 0;

  Serial.println("\nDigite o valor, em milissegundo, o tempo de duracao do Ma em vtcd:");
  while (Tempo_VTCD <= 0) {


    i++;
    if (i >= 2) {
      Serial.println("Digite novamente o valor inteiro, maior que 0, o tempo de duracao do Ma em vtcd: ");
      Tempo_VTCD = 0;
    }
    Tempo_VTCD = Serial.parseInt();
    Serial.println(Tempo_VTCD);
  }

  Serial.println("\ntempo de duracao do Ma em vtcd: ");
  Serial.println(Tempo_VTCD);
  Serial.println("\n\n");
  i = 0;

  Serial.println("\n Digite o valor inteiro, maior que 0, da frequencia do inversor: ");
  while (frequencia <= 0) {


    i++;
    if (i >= 2) {
      Serial.println("Digite novamente o valor inteiro, maior que 0, da frequencia do inversor:");
    }
    frequencia = Serial.parseInt();
    Serial.println("frequencia: ");
  }
  Serial.println(frequencia);
  Serial.println("\n\n");
}
void setup()
{

  i = 0;
  Serial.begin(115200);
  Serial.setTimeout(10000);
  pinMode(PC13, OUTPUT);
  pinMode(controle_inversor, OUTPUT);
  digitalWrite(PC13, LOW);
  while (!Serial) {
    digitalWrite(PC13, !digitalRead(PC13));
    delay(200);
  }

  comunicacao();


  tempo_Tensao_Nominal = tempo_Tensao_Nominal * 1000;
  Tempo_VTCD *= 1000;

  Gerador_VTCD = new HardwareTimer(TIM1);
  Gerador_VTCD->pause();
  Gerador_VTCD->setOverflow(tempo_Tensao_Nominal, MICROSEC_FORMAT);
  Gerador_VTCD->attachInterrupt(1, Tempo_atualizacao);
  inicio = micros();
  Gerador_VTCD->resume();

  Ma = 100;
  timer = new HardwareTimer(TIM4);
  timer->pause();
  timer->setOverflow(10000, HERTZ_FORMAT);
  timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, Braco_superior_esquerdo);
  timer->setMode(2, TIMER_OUTPUT_COMPARE_PWM2, Braco_inferior_esquerdo);
  timer->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, Braco_superior_direito);
  timer->setMode(4, TIMER_OUTPUT_COMPARE_PWM2, Braco_inferior_direito);
  timer->setCaptureCompare(1, 100, PERCENT_COMPARE_FORMAT);
  timer->setCaptureCompare(2, 0 , PERCENT_COMPARE_FORMAT);
  timer->setCaptureCompare(3, 100, PERCENT_COMPARE_FORMAT);
  timer->setCaptureCompare(4, 0 , PERCENT_COMPARE_FORMAT);
  timer->attachInterrupt(1, Mudanca_comparador);
  timer->resume();

  //INICIA o contador de funcionamento da tensão nominal
  digitalWrite(controle_inversor, HIGH);
  Serial.println("\n######################################################################");
  Serial.println("Inversor Ligado ");
  Serial.println("#######################################################################!");
}
void Mudanca_comparador() {
  percorrido = (micros() - inicio) / 1000000;
  Ma = passar_valor;
  seno_braco_1 = (int)( Ma * (0.5 + 0.5 * sin(percorrido * frequencia * omega)));
  seno_braco_2 = (int)(Ma * (0.5 - 0.5 * sin(percorrido * frequencia * omega)));
  timer->setCaptureCompare(1, seno_braco_1, PERCENT_COMPARE_FORMAT);
  timer->setCaptureCompare(2, seno_braco_1, PERCENT_COMPARE_FORMAT);
  timer->setCaptureCompare(3, seno_braco_2, PERCENT_COMPARE_FORMAT);
  timer->setCaptureCompare(4, seno_braco_2, PERCENT_COMPARE_FORMAT);
}
void Tempo_atualizacao() {
  if (Flag_2 == true) {
    passar_valor = Ma_nominal;
    pegar_valor = Tempo_VTCD ;
    Flag_2 = false;
    digitalWrite(PC13, !digitalRead(PC13));
    Serial.println("\nOperação com Ma nominal");
  } else {
    passar_valor = Ma_VTCD;
    pegar_valor = tempo_Tensao_Nominal;
    Flag_2 = true;
    digitalWrite(PC13, !digitalRead(PC13));
    Serial.println("\nOperação com Ma em vtcd");
  }
  Gerador_VTCD->setOverflow(pegar_valor, MICROSEC_FORMAT);
}
void loop()
{
}
