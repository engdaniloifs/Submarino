// -----------------------------
// Configuração de pinos do sensor
// -----------------------------
const byte OUT_pin = 12; // Pino de saída de dados do sensor
const byte SCK_pin = 11; // Pino de clock do sensor

// -----------------------------
// Variáveis globais
// -----------------------------
long leitura_sensorinicio_media = 0; // Valor médio inicial do sensor (offset para calibração)


// -----------------------------
// Função para leitura bruta do sensor
// -----------------------------
long lerSensor() {
  const byte GAIN = 3;
  byte data[3];

  // Leitura dos 3 bytes de dados
  for (byte j = 3; j--;) {
    data[j] = shiftIn(OUT_pin, SCK_pin, MSBFIRST);
  }

  // Geração dos pulsos de clock adicionais conforme o ganho
  for (byte l = 0; l < GAIN; l++) {
    digitalWrite(SCK_pin, HIGH);
    digitalWrite(SCK_pin, LOW);
  }

  // Combina os 3 bytes em um valor de 24 bits
  data[2] ^= 0x80;

  long leitura = ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[0];
  
  // Normaliza a leitura com base no offset (2^23 - 1)
  leitura -= 8388607;
  return leitura;
}


// -----------------------------
// Função para converter a leitura em profundidade (em cm)
// -----------------------------
float conversorSensor(long leitura_atual) {
  float leitura_relativa = (leitura_atual - leitura_sensorinicio_media) / 419430.35; // Fator de escala (baseado no range do sensor)
  float leitura_profundidade = leitura_relativa * 7.246; // Conversão para profundidade (cm) via fator de calibração
  return leitura_profundidade;
}


// -----------------------------
// Função de setup
// -----------------------------
void setup() {
  Serial.begin(9600);
  long leitura_sensorinicio = 0;
  const byte media = 10; // Número de amostras para média inicial

  pinMode(SCK_pin, OUTPUT);
  pinMode(OUT_pin, INPUT);

  delay(500); // Tempo para estabilização do sensor
  for(byte i = 0; i < 10; i++)
  {
    while(digitalRead(OUT_pin));
    leitura_sensorinicio += lerSensor();
  }
  leitura_sensorinicio_media = leitura_sensorinicio / media;
}


// -----------------------------
// Função principal (loop)
// -----------------------------
void loop() {
  static long leitura_sensoratual = 0;
  static float profundidade = 0;

  // Só realiza leitura se o sensor estiver pronto
  if (!digitalRead(OUT_pin)) {
    leitura_sensoratual = lerSensor();
    profundidade = conversorSensor(leitura_sensoratual);
  }

  // Exibe a profundidade no monitor serial
  Serial.print("Profundidade (cm): ");
  Serial.println(profundidade);
}
