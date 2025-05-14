
// ############################################# SERVO ###################

#define usToTicks(_us)    (( clockCyclesPerMicrosecond() * (_us)) / 8)     
#define ticksToUs(_ticks) (((unsigned)(_ticks) * 8) / clockCyclesPerMicrosecond())

#define REFRESH_INTERVAL 20000     // Tempo entre pulsos do servo (μs)
#define servo_pulsemax    2400
#define servo_pulsemin    544

const byte servo_pin = 2;
volatile unsigned int servo_ticks;  // Duração do pulso em "ticks"

void handle_interrupt_simple() {
  static bool servoAtivo = 0;
  
  if (servoAtivo) {
    // Final do pulso (LOW)
    PORTD &= ~(1 << servo_pin);
    servoAtivo = 0;
    OCR1A = usToTicks(REFRESH_INTERVAL - servo_ticks);
  } else {
    // Início do pulso (HIGH)
    PORTD |= (1 << servo_pin);
    OCR1A = TCNT1 + servo_ticks;
    servoAtivo = 1;
  }
}

ISR(TIMER1_COMPA_vect) {
  handle_interrupt_simple();
}

void servo_attach() {
  pinMode(servo_pin, OUTPUT);
  TCCR1A = 0;
  TCCR1B = _BV(CS11);  // Prescaler 8
  TCNT1 = 0;

  TIFR1 |= _BV(OCF1A);
  TIMSK1 |= _BV(OCIE1A);
}

void servo_write(int ang) {
  ang = constrain(ang, 0, 180);
  unsigned int pulse = map(ang, 0, 180, servo_pulsemin, servo_pulsemax);
  pulse = usToTicks(pulse);

  uint8_t oldSREG = SREG;
  cli();
  servo_ticks = pulse;
  SREG = oldSREG;
}

// ############################################# RADIO ###################

bool flutua = 0;
bool afunda = 0;
bool frente = 0;
bool tras   = 0;

void ler_radio() {
  if (analogRead(A2) < 50) afunda = 1;
  if (analogRead(A3) < 50) flutua = 1;
  if (analogRead(A1) < 50) frente = 1;
  if (analogRead(A4) < 50) tras   = 1;
}

// ########################################## SENSOR #########################

const byte OUT_pin = 12;  // Dados do sensor
const byte SCK_pin = 11;  // Clock do sensor
long leitura_sensorinicio_media = 0;

long lerSensor() {
  const byte GAIN = 1;
  byte data[3];

  for (byte j = 3; j--;) {
    data[j] = shiftIn(OUT_pin, SCK_pin, MSBFIRST);
  }

  for (byte l = 0; l < GAIN; l++) {
    digitalWrite(SCK_pin, HIGH);
    digitalWrite(SCK_pin, LOW);
  }

  data[2] ^= 0x80;
  long leitura = ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[0];
  leitura -= 8388607;
  return leitura;
}

float conversorSensor(long leitura_atual) {
  float leitura_relativa = (leitura_atual - leitura_sensorinicio_media) / 419430.35;
  float leitura_profundidade = leitura_relativa * 7.246;
  return leitura_profundidade;
}

// ########################################## SETUP / LOOP ###################

#define seringa_inicio  180
#define seringa_max     130
#define seringa_min     100

void setup() {
  Serial.begin(9600);
  servo_attach();
  servo_write(seringa_inicio);
  ler_radio();
  while(!frente)
  {
    ler_radio();
  }
  frente = 0;
  
}

void loop() {
  static long leitura_sensoratual = 0;
  static float profundidade = 0;
  static float setpoint_profundidade = 0;
  static float pressao_interna = 0;
  static int angtest = 0;
  static int pos = 180;
  static float profundidade_filtrada = 0;
  static float alpha = 0.5;
  angtest = 180 - pos;
  static unsigned long tempo2 = 0;
  static unsigned long tempo3 = 0;
  
  static unsigned long tempo = millis();
  servo_write(pos);
  

  ler_radio();

  if ((flutua) && ((millis()-tempo3)>500) ) {

    pos -= 5;
    tempo3 = millis();
  }

  if (afunda && ((millis()-tempo3)>500)) {
    pos += 5;
    tempo3 = millis();
  }

  if(tras)
  { 
    tras = 0;
    while (true) 
    {
      servo_write(seringa_inicio);

    }  // força travamento até o reset
  }
  pos = constrain(pos, 0, 180);
  delay(100);
  Serial.println(pos);

  
}
