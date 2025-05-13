int flutua = A3;
int afunda = A2;
int frente = A1;
int tras = A4;
int flutualeitura = 0;
int afundaleitura = 0;
int frenteleitura = 0;
int trasleitura = 0;

void setup() 
{
  Serial.begin(9600);

}

void loop() 
{
  flutualeitura = analogRead(flutua);
  afundaleitura = analogRead(afunda);
  frenteleitura = analogRead(frente);
  trasleitura = analogRead(tras);
  
  Serial.print("Flutua:");
  Serial.println(flutualeitura);
  Serial.print("Afunda:");
  Serial.println(afundaleitura);
  Serial.print("Frente:");
  Serial.println(frenteleitura);
  Serial.print("Tras:");
  Serial.println(trasleitura);
  Serial.println("");
  Serial.println("");
  delay(1000);
}
