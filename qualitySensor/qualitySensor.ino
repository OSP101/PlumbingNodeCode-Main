float ntcValue; // Variable to store NTC value
int sensorPin = 32;

void setup() {
  Serial.begin(9600); // Baud rate: 9600
  pinMode(sensorPin, INPUT);
  pinMode(15, OUTPUT);
}

void loop() {
  digitalWrite(15, HIGH);
  int sensorValue = analogRead(sensorPin); // Read the analog sensor value
  // Map the analog sensor value to the NTC range (0 to 250)
  float voltage = sensorValue * (4.5 / 1024.0);
  ntcValue = map(voltage, 0, 20, 250, 0);

  Serial.print("Analog Sensor Value: ");
  Serial.print(voltage);
  Serial.print(", NTC Value: ");
  Serial.println(ntcValue);

  delay(500);
}

