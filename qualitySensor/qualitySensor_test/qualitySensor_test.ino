float ntcValue; // Variable to store NTC value


void setup() {
  Serial.begin(9600); // Baud rate: 9600
  pinMode(32, INPUT);
  pinMode(15, OUTPUT);
}

void loop() {
  digitalWrite(15, HIGH);
  int sensorValue = analogRead(32);
  float voltage = sensorValue * (4.5 / 1024.0);
  ntcValue = map(voltage, 0, 10, 12, 0);

  Serial.print("Analog Sensor Value: ");
  Serial.print(voltage);
  Serial.print(", NTC Value: ");
  Serial.println(ntcValue);

  delay(500);
}
