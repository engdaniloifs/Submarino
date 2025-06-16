// ############################################# SERVO ###################

// Microseconds to ticks conversion
#define usToTicks(_us)    (( clockCyclesPerMicrosecond() * (_us)) / 8)     

// Servo control parameters
#define REFRESH_INTERVAL 20000     // Interval between servo pulses (μs)
#define servo_pulsemax    2400     // Maximum pulse width (μs)
#define servo_pulsemin    544      // Minimum pulse width (μs)

const byte servo_pin = 2;                  // Servo control pin
volatile unsigned int servo_ticks;        // Current pulse duration in "ticks"

// Timer1 interrupt — generates periodic pulses on the servo pin
ISR(TIMER1_COMPA_vect) {
  static bool servoAtivo = 0;

  if (servoAtivo) {
    // End of pulse (LOW)
    PORTD &= ~(1 << servo_pin);
    servoAtivo = 0;
    OCR1A = usToTicks(REFRESH_INTERVAL - servo_ticks);
  } else {
    // Start of pulse (HIGH)
    PORTD |= (1 << servo_pin);
    OCR1A = TCNT1 + servo_ticks;
    servoAtivo = 1;
  }
}

// Initializes Timer1 and configures the servo pin
void servo_attach() {
  pinMode(servo_pin, OUTPUT);
  TCCR1A = 0;
  TCCR1B = _BV(CS11);  // Prescaler 8
  TCNT1 = 0;

  TIFR1 |= _BV(OCF1A);
  TIMSK1 |= _BV(OCIE1A);
}

// Sets the servo angle (from 0 to 180 degrees)
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

// Radio control flags
bool flutua = 0;
bool afunda = 0;
bool frente = 0;
bool tras   = 0;

// Reads analog channels that control the submarine
void ler_radio() {
  if (analogRead(A3) < 50) {
    flutua = 1;
    return;
  }
  if (analogRead(A2) < 50) {
    afunda = 1;
    return;
  } 
  if (analogRead(A1) < 50) {
    frente = 1;
    return;
  }
  if (analogRead(A4) < 50) {
    tras = 1;
    return;
  }
}

// ########################################## SENSOR #########################

const byte OUT_pin = 12;  // Pressure sensor data pin
const byte SCK_pin = 11;  // Pressure sensor clock pin
long leitura_sensorinicio_media = 0;  // Initial average reading

// Reads raw data from the pressure sensor
long lerSensor() {
  const byte GAIN = 3 ;
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

// Converts the sensor reading to depth (in centimeters)
float conversorSensor(long leitura_atual) {
  float leitura_relativa = (leitura_atual - leitura_sensorinicio_media) / 419430.35;
  float leitura_profundidade = leitura_relativa * 7.1615;
  return leitura_profundidade;
}

// ########################################## SETUP / LOOP ###################

#define seringa_inicio  180
#define seringa_max     130
#define seringa_min     80

void setup() {
  Serial.begin(9600);

  // Initialize servo
  servo_attach();
  servo_write(seringa_inicio);

  // Wait for "frente" command
  ler_radio();
  while (!frente) {
    ler_radio();
  }
  frente = 0;

  // Initialize pressure sensor
  long leitura_sensorinicio = 0;
  const byte media = 10;

  // Setup for PLX-DAQ data logging
  Serial.println("CLEARDATA");  
  Serial.println("LABEL,TIME,Tempo_inicio,Profundidade,Setpoint,Angulo");

  // Set communication pins for the pressure sensor
  pinMode(SCK_pin, OUTPUT);
  pinMode(OUT_pin, INPUT);

  delay(500);

  // Measure the initial pressure average to define depth zero
  for (byte i = 0; i < media; i++) {
    while (digitalRead(OUT_pin));
    leitura_sensorinicio += lerSensor();
  }
  leitura_sensorinicio_media = leitura_sensorinicio / media;
}

void loop() {
  static float profundidade = 0;
  static float setpoint_profundidade = 11;
  static int angtest = 0;
  static int pos = 180;
  angtest = 180 - pos;

  static unsigned long tempo2 = 0;
  static unsigned long tempo3 = 0;
  static unsigned long tempo = millis();
  static float contatempo = 0;

  // Update servo to current position
  servo_write(pos);

  // If new sensor data is available, update depth
  if (!digitalRead(OUT_pin)) {
    long leitura_temp = lerSensor();
    float profundidade_temp = conversorSensor(leitura_temp);

    // Adds the compensation caused by internal pressure changes from the syringe piston
    float profundidade_com_pressao = profundidade_temp + angtest * 0.148;

    if (abs(profundidade_com_pressao - profundidade) < 2.0) {
      profundidade = profundidade_com_pressao;
    }
  }

  // Read radio control inputs
  ler_radio();

  // "tras" command forces system lock until manual reset
  if (tras) {
    tras = 0;
    while (true) {
      servo_write(seringa_inicio);
    }
  }

  // ON/OFF control logic
  if ((millis() - tempo) > 2000) {
    tempo = millis();

    if (profundidade > setpoint_profundidade) {
      pos = seringa_max;
    } else if (profundidade < setpoint_profundidade) {
      pos = seringa_min;
    }

    servo_write(pos);
  }

  // Sends data to serial/PLX-DAQ every 100 ms
  if ((millis() - tempo2) > 100) {
    Serial.print("DATA,TIME,");
    Serial.print(contatempo, 2); Serial.print(",");
    Serial.print(profundidade, 2); Serial.print(",");
    Serial.print(setpoint_profundidade, 2); Serial.print(",");
    Serial.println(angtest);
    tempo2 = millis();
  }

  contatempo = (float)millis() / 1000;
}
