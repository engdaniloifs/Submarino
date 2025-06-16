#include <Servo.h>  // Inclui a biblioteca Servo para controlar servos

// Define os objetos e variáveis para os sensores e controle do servo
Servo myservo;  // Objeto para o servo motor

// Pinos de leitura dos sensores
int flutualeitura = A3;
int afundaleitura = A2;
int frenteleitura = A1;
int trasleitura = A4;

// Variáveis para armazenar os valores lidos dos sensores
int flutua = 0;
int afunda = 0;
int frente = 0;
int tras = 0;

// Posição inicial do servo
int pos = 180;

// Pinos para controlar o motor (IN1 e IN2)
int IN1 = 3;
int IN2 = 5;

// Flag usada para controlar a direção do movimento
int flag = 0;

void setup() {
  // Inicializa a comunicação serial para monitoramento
  Serial.begin(9600);
  
  // Anexa o servo ao pino 2 e configura a posição inicial
  myservo.attach(2);
  myservo.write(pos);  // Define a posição inicial do servo para 180 graus
  
  // Configura os pinos IN1 e IN2 como saída para controle do motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
}

void loop() {
  // Lê os valores dos sensores analógicos
  flutua = analogRead(flutualeitura);
  afunda = analogRead(afundaleitura);
  frente = analogRead(frenteleitura);
  tras = analogRead(trasleitura);

  // Envia a posição do servo para o monitor serial
  Serial.println(pos);
  
  // Reseta os sinais de controle do motor
  analogWrite(IN2, 0);
  analogWrite(IN1, 0);
  
  // Reset da flag
  flag = 0;
  
  // Controla a flutuação do sistema, ajustando a posição do servo
  if (afunda < 50) {
    pos = pos - 5;  // Diminui a posição do servo (afundar)
    delay(500);     // Atraso de 500ms para dar tempo para o servo mover
  }
  
  if (flutua < 50) {
    pos = pos + 5;  // Aumenta a posição do servo (subir)
    delay(500);     // Atraso de 500ms para dar tempo para o servo mover
  }

  // Controle do movimento para frente baseado no sensor de frente
  while (frente < 50) {
    frente = analogRead(frenteleitura);  // Lê o valor do sensor de frente
    if (flag == 0) {
      // Se a flag ainda não foi ativada, configura a direção do motor para frente
      analogWrite(IN2, 0);
      analogWrite(IN1, 255);  // Motor para frente
      delay(300);  // Atraso de 300ms para dar tempo ao motor
    }
    flag = 1;  // Marca a flag para indicar que a direção foi configurada
    analogWrite(IN1, 150);  // Ajusta a velocidade do motor para frente
  }

  // Controle do movimento para trás baseado no sensor de trás
  while (tras < 50) {
    tras = analogRead(trasleitura);  // Lê o valor do sensor de trás
    if (flag == 0) {
      // Se a flag ainda não foi ativada, configura a direção do motor para trás
      analogWrite(IN1, 0);
      analogWrite(IN2, 255);  // Motor para trás
      delay(300);  // Atraso de 300ms para dar tempo ao motor
    }
    flag = 1;  // Marca a flag para indicar que a direção foi configurada
    analogWrite(IN2, 150);  // Ajusta a velocidade do motor para trás
  }

  // Limita a posição do servo para não ultrapassar os limites de 0 a 180 graus
  if (pos > 180) {
    pos = 180;  // Limita a posição do servo para 180 graus (máximo)
  }

  if (pos < 0) {
    pos = 0;  // Limita a posição do servo para 0 graus (mínimo)
  }

  // Define a posição do servo conforme a variável 'pos'
  myservo.write(pos);
}
