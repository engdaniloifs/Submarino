#include <Q2HX711.h>

const byte OUT_pin = 12;
const byte SCK_pin = 11;

// Configuração do sensor
Q2HX711 sensor_pressao(OUT_pin, SCK_pin);

// Variáveis globais
long leitura_sensorinicio = 0; // Variável global para armazenar a leitura inicial
unsigned long tempo = 0;
unsigned long tempofinal = 0;

long leitura_sensoratual = 0;
float profundidade = 0;

// Leitura do sensor
long lerSensor() {
  long leitura = sensor_pressao.read();
  leitura -= 8388607; // Subtração do valor do offset para normalizar
  return leitura;
}

// Função para estabilizar a leitura do sensor


// Função para conversão da leitura em profundidade
float conversorsensor(long leitura_sensorinicio, long leitura_sensoratual) {
  float leitura_relativa = (float)(leitura_sensoratual - leitura_sensorinicio) / 419430.35;
  float leitura_profundidade = leitura_relativa * 7.246; // Multiplicação pelo fator de calibração
  return leitura_profundidade;
}

void setup() {
  Serial.begin(9600);
  delay(500);

  leitura_sensorinicio = lerSensor(); // Captura o valor inicial para compensação
}

void loop() 
{
  tempo = millis();

  if((tempo-tempofinal) >= 100)
  {
    leitura_sensoratual = lerSensor();
    profundidade = conversorsensor(leitura_sensorinicio, leitura_sensoratual); // Chama a função de conversão
    tempofinal = tempo;
  }
  
  Serial.print("Profundidade (cm): ");
  Serial.println(profundidade); // Exibe o valor da profundidade no Serial Monitor
}
