#define usToTicks(_us)    (( clockCyclesPerMicrosecond() * (_us)) / 8)     
#define ticksToUs(_ticks) (((unsigned)(_ticks) * 8) / clockCyclesPerMicrosecond())

#define REFRESH_INTERVAL 20000  // Tempo entre pulsos de servo em microssegundos
#define servo_pulsemax  2400  
#define servo_pulsemin 544

const byte servo_pin = 2;
long angulo_teste = 0;

volatile unsigned int servo_ticks;  // Duração do pulso em "ticks"

void handle_interrupt_simple() 
{
  static bool servoAtivo = 0;
  
  if (servoAtivo) 
  {
    // Final do pulso (LOW)
    PORTD &= ~(1 << servo_pin);  // digitalWrite(servo_pin, LOW)
    servoAtivo = 0;
    // Ajusta o tempo de espera até o próximo pulso (20ms total)
    OCR1A = usToTicks(REFRESH_INTERVAL - servo_ticks);  // Tempo de LOW = REFRESH_INTERVAL - HIGH
  }
  else 
  {
    // Início do pulso (HIGH)
    PORTD |= (1 << servo_pin);   // digitalWrite(servo_pin, HIGH)
    OCR1A = TCNT1 + servo_ticks; // Tempo de HIGH
    servoAtivo = 1;
  }
}

ISR(TIMER1_COMPA_vect)
{
  handle_interrupt_simple();
}

void servo_attach()
{
  pinMode(servo_pin, OUTPUT);

  // Configuração do Timer1
  TCCR1A = 0;              // Modo normal
  TCCR1B = _BV(CS11);      // Prescaler = 8
  TCNT1 = 0;

  TIFR1 |= _BV(OCF1A);     // Limpa flags de interrupção pendente
  TIMSK1 |= _BV(OCIE1A);   // Habilita interrupção por comparação
}

void servo_write(byte ang)
{

  ang = constrain(ang, 0, 180);  // Limita entre 0 e 180

  unsigned int pulse = map(ang, 0, 180, servo_pulsemin, servo_pulsemax);
  pulse = usToTicks(pulse);

  // Atualiza o tempo de pulso com interrupções desabilitadas
  uint8_t oldSREG = SREG;
  cli();              // Desativa interrupções
  servo_ticks = pulse;
  SREG = oldSREG;     // Restaura estado de interrupções
}

void setup() 
{
  Serial.begin(9600);
  servo_attach();     // Configura o timer e o pino do servo
  servo_write(angulo_teste);    // Servo na posição neutra
}

void loop() 
{
 
  if (Serial.available()) {
    angulo_teste = Serial.parseInt();  // Lê um número inteiro da serial

    if (angulo_teste >= 0 && angulo_teste <= 180) {
      servo_write(angulo_teste);           // Define o ângulo do servo
      Serial.print("Servo movido para: ");
      Serial.println(angulo_teste);
    } else {
      Serial.println("Valor inválido. Digite um número entre 0 e 180.");
    }
  }
}
